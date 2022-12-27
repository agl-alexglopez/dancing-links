#include "GUI/MiniGUI.h"
#include "PokemonParser.h"
#include "PokemonLinks.h"
#include "Utilities/PokemonUtilities.h"
#include "Utilities/RankedSet.h"
#include <fstream>
#include <memory>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <queue>
#include <vector>
#include <regex>
#include <string>
#include <set>
#include "filelib.h"
#include "strlib.h"
using namespace std;
using namespace MiniGUI;

namespace {
    /* File constants. */
    const string kProblemSuffix = ".dst";
    const string kBasePath = "res/pokemon/";
    /* Background color. */
    const string kBackgroundColor  = "#000000";

    /* Colors to use when drawing the network. */
    struct CityColors {
        string borderColor;
        string fillColor;
        Font   font;
    };

    enum CityState { // "The state the city is in," not "Singapore." :-)
        UNCOVERED,
        COVERED_INDIRECTLY,
        COVERED_DIRECTLY
    };

    /* This communicates solver used and index in kSolverColorOptions to use as well. */
    enum CitySolver {
        SET_BASED=0,
        QUAD_DLX=1,
        TAGGED_DLX=2,
    };

    /* Colors to use when drawing cities. */
    const vector<CityColors> kColorOptions = {
        { "#101010", "#202020", Font(FontFamily::MONOSPACE, FontStyle::BOLD, 12, "#A0A0A0") },   // Uncovered
        { "#303060", "#404058", Font(FontFamily::MONOSPACE, FontStyle::BOLD, 12, "#C0C0C0") },   // Indirectly covered
        { "#806030", "#FFB000", Font(FontFamily::MONOSPACE, FontStyle::BOLD, 12, "#000000") },   // Directly covered
    };

    /* Colors to use to draw the roads. */
    const string kDarkRoadColor = "#505050";
    const string kLightRoadColor = "#FFFFFF";

    /* Line thicknesses. */
    const double kRoadWidth = 3;
    const double kCityWidth = 1.5;

    /* Font to use for city labels. */
    const string kLabelFont = "Monospace-BOLD-12";

    /* Radius of a city */
    const double kCityRadius = 25;

    /* Buffer space around the window. */
    const double kBufferSpace = 60;

    /* Lower bound on the width or height of the data range, used for
     * collinear points.
     */
    const double kLogicalPadding = 1e-5;

    /* Max length of a string in a label. */
    const string::size_type kMaxLength = 3;

    /* Geometry information for drawing the network. */
    struct Geometry {
        /* Range of X and Y values in the data set, used for
         * scaling everything.
         */
        double minDataX, minDataY, maxDataX, maxDataY;

        /* Range of X and Y values to use when drawing everything. */
        double minDrawX, minDrawY, maxDrawX, maxDrawY;
    };

    /* Given a data set, fills in the min and max X and Y values
     * encountered in that set.
     */
    void computeDataBounds(const DisasterTest& network, Geometry& geo) {
        geo.minDataX = geo.minDataY = numeric_limits<double>::infinity();
        geo.maxDataX = geo.maxDataY = -numeric_limits<double>::infinity();

        for (const string& cityName: network.cityLocations) {
            geo.minDataX = min(geo.minDataX, network.cityLocations[cityName].x);
            geo.minDataY = min(geo.minDataY, network.cityLocations[cityName].y);

            geo.maxDataX = max(geo.maxDataX, network.cityLocations[cityName].x);
            geo.maxDataY = max(geo.maxDataY, network.cityLocations[cityName].y);
        }

        /* Pad the boundaries. This accounts for the edge case where one set of bounds is
         * degenerate.
         */
        geo.minDataX -= kLogicalPadding;
        geo.minDataY -= kLogicalPadding;
        geo.maxDataX += kLogicalPadding;
        geo.maxDataY += kLogicalPadding;
    }

    /* Once we have the data bounds, we can compute the graphics bounds,
     * which will try to take maximum advantage of the width and height
     * that we have available to us.
     */
    void computeGraphicsBounds(GWindow& window, Geometry& geo) {
        /* Get the aspect ratio of the window. */
        double winWidth  = window.getCanvasWidth()  - 2 * kBufferSpace;
        double winHeight = window.getCanvasHeight() - 2 * kBufferSpace;
        double winAspect = winWidth / winHeight;

        /* Get the aspect ratio of the data set. */
        double dataAspect = (geo.maxDataX - geo.minDataX) / (geo.maxDataY - geo.minDataY);

        double dataWidth, dataHeight;

        /* If the data aspect ratio exceeds the window aspect ratio,
         * the limiting factor in the display is going to be the
         * width. Therefore, we'll use that to determine our effective
         * width and height.
         */
        if (dataAspect >= winAspect) {
            dataWidth = winWidth;
            dataHeight = dataWidth / dataAspect;
        } else {
            dataHeight = winHeight;
            dataWidth = dataAspect * dataHeight;
        }

        /* Now, go center that in the window. */
        geo.minDrawX = (winWidth  -  dataWidth) / 2.0 + kBufferSpace;
        geo.minDrawY = (winHeight - dataHeight) / 2.0 + kBufferSpace;

        geo.maxDrawX = geo.minDrawX + dataWidth;
        geo.maxDrawY = geo.minDrawY + dataHeight;
    }

    /* Given the road network, determines its geometry. */
    Geometry geometryFor(GWindow& window, const DisasterTest& network) {
        Geometry result;
        computeDataBounds(network, result);
        computeGraphicsBounds(window, result);
        return result;
    }

    /* Converts a coordinate in logical space into a coordinate in
     * physical space.
     */
    GPoint logicalToPhysical(const GPoint& pt, const Geometry& geo) {
        double x = ((pt.x - geo.minDataX) / (geo.maxDataX - geo.minDataX)) * (geo.maxDrawX - geo.minDrawX) + geo.minDrawX;
        double y = ((pt.y - geo.minDataY) / (geo.maxDataY - geo.minDataY)) * (geo.maxDrawY - geo.minDrawY) + geo.minDrawY;

        return { x, y };
    }

    /* Draws all the roads in the network, highlighting ones that
     * are adjacent to lit cities.
     */
    void drawRoads(GWindow& window,
                   const Geometry& geo,
                   const DisasterTest& network,
                   const Set<string>& selected) {
        /* For efficiency's sake, just create one line. */
        GLine toDraw;
        toDraw.setLineWidth(kRoadWidth);

        for (const string& source: network.network) {
            for (const string& dest: network.network[source]) {
                /* Selected roads draw in the bright color; deselected
                 * roads draw in a the dark color.
                 */
                toDraw.setColor((selected.contains(source) || selected.contains(dest))? kLightRoadColor : kDarkRoadColor);

                /* Draw the line, remembering that the coordinates are in
                 * logical rather than physical space.
                 */
                auto src = logicalToPhysical(network.cityLocations[source], geo);
                auto dst = logicalToPhysical(network.cityLocations[dest], geo);
                toDraw.setStartPoint(src.x, src.y);
                toDraw.setEndPoint(dst.x, dst.y);

                window.draw(toDraw);
            }
        }
    }

    /* Returns a shortened name for the given city name. We use the first
     * three letters of the name if it's a single word and otherwise use
     * its initials.
     */
    string shorthandFor(const string& name) {
        auto components = stringSplit(name, " ");
        if (components.size() == 0) {
            error("It shouldn't be possible for there to be no components of the city name.");
            return "";
        } else if (components.size() == 1) {
            if (components[0].length() < kMaxLength) return components[0];
            else return components[0].substr(0, 3);
        } else {
            /* Use initials. */
            string result;
            for (size_t i = 0; result.length() < kMaxLength && i < components.size(); i++) {
                /* Skip empty components, which might exist if there are consecutive spaces in
                 * the name
                 */
                if (!components[i].empty()) {
                    result += components[i][0];
                }
            }
            return result;
        }
    }

    /* Draws all the cities, highlighting the ones that are in the
     * selected set.
     */
    void drawCities(GWindow& window,
                    const Geometry& geo,
                    const DisasterTest& network,
                    const Set<string>& selected) {

        /* For simplicity, just make a single oval. */
        GOval oval(0, 0, 2 * kCityRadius, 2 * kCityRadius);
        oval.setLineWidth(kCityWidth);
        oval.setFilled(true);

        for (const string& city: network.network) {
            /* Figure out the center of the city on the screen. */
            auto center = logicalToPhysical(network.cityLocations[city], geo);

            /* See what state the city is in with regards to coverage. */
            CityState state = UNCOVERED;
            if (selected.contains(city)) state = COVERED_DIRECTLY;
            else if (!(selected * network.network[city]).isEmpty()) state = COVERED_INDIRECTLY;

            /* There's no way to draw a filled circle with a boundary as one call. */
            oval.setColor(kColorOptions[state].borderColor);
            oval.setFillColor(kColorOptions[state].fillColor);
            window.draw(oval,
                        center.x - kCityRadius,
                        center.y - kCityRadius);

            /* Set the label text and color. */
            auto render = TextRender::construct(shorthandFor(city), {
                                                    center.x - kCityRadius,
                                                    center.y - kCityRadius,
                                                    2 * kCityRadius,
                                                    2 * kCityRadius
                                                }, kColorOptions[state].font);
            render->alignCenterHorizontally();
            render->alignCenterVertically();
            render->draw(window);
        }
    }

    void visualizeNetwork(GWindow& window,
                          const PokemonTest& network,
                          const Set<string>& selected) {
        clearDisplay(window, kBackgroundColor);

        /* Edge case: Don't draw if the window is too small. */
        if (window.getCanvasWidth()  <= 2 * kBufferSpace ||
            window.getCanvasHeight() <= 2 * kBufferSpace) {
            return;
        }

        /* There's a weird edge case where if there are no cities,
         * the window geometry can't be calculated properly. Therefore,
         * we're going skip all this logic if there's nothing to draw.
         */
        if (!network.pokemonGenerationMap.network.isEmpty()) {
            Geometry geo = geometryFor(window, network.pokemonGenerationMap);

            /* Draw the roads under the cities to avoid weird graphics
             * artifacts.
             */
            drawRoads(window, geo, network.pokemonGenerationMap, selected);
            drawCities(window, geo, network.pokemonGenerationMap, selected);
        }
    }

    vector<string> sampleProblems(const string& basePath) {
        vector<string> result;
        for (const auto& file: listDirectory(basePath)) {
            if (endsWith(file, kProblemSuffix)) {
                result.push_back(file);
            }
        }
        return result;
    }

    class PokemonGUI: public ProblemHandler {
    public:
        PokemonGUI(GWindow& window);

        void actionPerformed(GObservable* source) override;
        void changeOccurredIn(GObservable* source) override;

    protected:
        void repaint() override;

    private:
        /* Dropdown of all the problems to choose from. */
        Temporary<GComboBox> mProblems;
        Temporary<GColorConsole> mSolutionsDisplay;
        const double DISPLAY_WIDTH = 800.0;
        //const double DISPLAY_HEIGHT = 200;

        /* Button to trigger the solver. */
        Temporary<GButton> mSolveDefense;
        Temporary<GButton> mSolveAttack;

        /* Current network and solution. */
        PokemonTest mGeneration;
        Set<string> mSelected;
        unique_ptr<multiset<RankedSet<std::string>>> mAllDefenseCoverages;
        unique_ptr<multiset<RankedSet<std::string>>> mAllAttackCoverages;

        /* Loads the world with the given name. */
        void loadWorld(const string& filename);

        /* Computes an optimal solution. */
        void solveDefense();
        //void solveAttack();
    };

    PokemonGUI::PokemonGUI(GWindow& window) : ProblemHandler(window) {
        GComboBox* choices = new GComboBox();
        for (const string& file: sampleProblems(kBasePath)) {
            choices->addItem(file);
        }
        choices->setEditable(false);

        mProblems = Temporary<GComboBox>(choices, window, "SOUTH");
        mSolutionsDisplay = Temporary<GColorConsole>(new GColorConsole(), window, "SOUTH");
        mSolutionsDisplay->setWidth(DISPLAY_WIDTH);
        mSolveDefense = Temporary<GButton>(new GButton("Solve Defense"), window, "SOUTH");
        mSolveAttack = Temporary<GButton>(new GButton("Solve Attack"), window, "SOUTH");
        loadWorld(choices->getSelectedItem());
    }

    void PokemonGUI::changeOccurredIn(GObservable* source) {
        if (source == mProblems) {
            loadWorld(mProblems->getSelectedItem());
        }
    }

    void PokemonGUI::actionPerformed(GObservable* source) {
        if (source == mSolveDefense) {
            solveDefense();
        } else if (source == mSolveAttack) {
            //solveAttack();
        }
    }
    void PokemonGUI::repaint() {
        /* Added functionality to draw the network different colors based on solver used. */
        visualizeNetwork(window(), mGeneration, mSelected);
    }

    void PokemonGUI::loadWorld(const string& filename) {
        ifstream input(kBasePath + filename);
        if (!input) error("Cannot open file.");

        mGeneration = loadPokemonGeneration(input);
        mAllDefenseCoverages.reset();
        mAllAttackCoverages.reset();
        mSelected.clear();
        mSolutionsDisplay->clearDisplay();
        requestRepaint();
    }

    void PokemonGUI::solveDefense() {
        /* Clear out any old solution. We're going to get a new one. */
        mSelected.clear();
        mAllDefenseCoverages.reset();
        mAllAttackCoverages.reset();
        mSolutionsDisplay->clearDisplay();

        /* Disable all controls until the operation finishes. */
        mSolveDefense->setEnabled(false);
        mSolveAttack->setEnabled(false);
        mProblems->setEnabled(false);

        if (mSelected.isEmpty()) {
            for (const auto& s : mGeneration.pokemonGenerationMap.network) {
                mSelected.add(s);
            }
        }

        mAllDefenseCoverages.reset(
            new multiset<RankedSet<std::string>>(
                PokemonLinks(
                    mGeneration.typeInteractions, PokemonLinks::DEFENSE
                ).getAllCoveredTeams()
            )
        );
        *mSolutionsDisplay << "Found " << (*mAllDefenseCoverages).size()
                           << " Pokemon teams [SCORE,TEAM]. Lower score is better." << endl;
        for (const RankedSet<std::string>& cov : (*mAllDefenseCoverages)) {
            *mSolutionsDisplay << cov.rank() << " | ";
            for (const std::string& type : cov) {
                *mSolutionsDisplay << type << " | ";
            }
            *mSolutionsDisplay << endl;
        }

        /* Enable controls. */
        mSolveDefense->setEnabled(true);
        mSolveAttack->setEnabled(true);
        mProblems->setEnabled(true);

        requestRepaint();
    }
}

GRAPHICS_HANDLER("Pokemon Planning", GWindow& window) {
    return std::make_shared<PokemonGUI>(window);
}
