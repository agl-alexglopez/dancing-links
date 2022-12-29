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

    /* Colors to use when drawing cities. */
    const vector<CityColors> kColorOptions = {
        { "#101010", "#202020", Font(FontFamily::MONOSPACE, FontStyle::BOLD, 12, "#A0A0A0") },   // Uncovered
        { "#303060", "#404058", Font(FontFamily::MONOSPACE, FontStyle::BOLD, 12, "#C0C0C0") },   // Indirectly covered
        { "#806030", "#FFB000", Font(FontFamily::MONOSPACE, FontStyle::BOLD, 12, "#000000") },   // Directly covered
    };

    const set<string> ALL_ATTACK_TYPES = {"Normal","Fire","Water","Electric","Grass","Ice",
                                          "Fighting","Poison","Ground","Flying","Psychic","Bug",
                                          "Rock","Ghost","Dragon","Dark","Steel","Fairy"};

    const string GYM_1_STR = "G1";
    const string GYM_2_STR = "G2";
    const string GYM_3_STR = "G3";
    const string GYM_4_STR = "G4";
    const string GYM_5_STR = "G5";
    const string GYM_6_STR = "G6";
    const string GYM_7_STR = "G7";
    const string GYM_8_STR = "G8";
    const string ELT_4_STR = "E4";
    const string NOT_SELECTED = "#000000";
    const string SELECTED = "#FF0000";

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

        typedef enum CoverageRequested {
            EXACT,
            OVERLAPPING
        }CoverageRequested;


        /* Dropdown of all the problems to choose from. */
        Temporary<GComboBox> mProblems;
        Temporary<GColorConsole> mSolutionsDisplay;
        const double DISPLAY_WIDTH = 800.0;
        //const double DISPLAY_HEIGHT = 200;

        /* Button to trigger the solver. */
        Temporary<GContainer> controls;
        GButton* exactDefenseButton;
        GButton* exactAttackButton;
        GButton* overlappingDefenseButton;
        GButton* overlappingAttackButton;

        Temporary<GContainer> gymControls;
        GButton* gym1;
        GButton* gym2;
        GButton* gym3;
        GButton* gym4;
        GButton* gym5;
        GButton* gym6;
        GButton* gym7;
        GButton* gym8;
        GButton* elite4;
        GButton* clearChoices;

        /* Current network and solution. */
        PokemonTest mGeneration;
        Set<string> mSelected;
        unique_ptr<set<RankedSet<std::string>>> mAllCoverages;

        /* Loads the world with the given name. */
        void loadWorld(const string& filename);

        void toggleSelectedGym(GButton*& button);
        void clearSelections();
        void solveDefense(const CoverageRequested& exactOrOverlapping);
        void solveAttack(const CoverageRequested& exactOrOverlapping);
    };

    PokemonGUI::PokemonGUI(GWindow& window) : ProblemHandler(window) {

        exactDefenseButton  = new GButton("Exact Defense Coverage");
        exactAttackButton  = new GButton("Exact Attack Coverage");
        overlappingDefenseButton  = new GButton("Loose Defense Coverage");
        overlappingAttackButton  = new GButton("Loose Attack Coverage");

        gym1 = new GButton(GYM_1_STR);
        gym2 = new GButton(GYM_2_STR);
        gym3 = new GButton(GYM_3_STR);
        gym4 = new GButton(GYM_4_STR);
        gym5 = new GButton(GYM_5_STR);
        gym6 = new GButton(GYM_6_STR);
        gym7 = new GButton(GYM_7_STR);
        gym8 = new GButton(GYM_8_STR);
        elite4 = new GButton(ELT_4_STR);
        clearChoices = new GButton("clear");
        gymControls = make_temporary<GContainer>(window, "WEST", GContainer::LAYOUT_GRID);
        gymControls->addToGrid(gym1, 6, 0);
        gymControls->addToGrid(gym2, 6, 1);
        gymControls->addToGrid(gym3, 6, 2);
        gymControls->addToGrid(gym4, 7, 0);
        gymControls->addToGrid(gym5, 7, 1);
        gymControls->addToGrid(gym6, 7, 2);
        gymControls->addToGrid(gym7, 8, 0);
        gymControls->addToGrid(gym8, 8, 1);
        gymControls->addToGrid(elite4, 8, 2);
        gymControls->addToGrid(clearChoices, 9, 1);

        gymControls->setEnabled(false);

        controls = make_temporary<GContainer>(window, "WEST", GContainer::LAYOUT_GRID);
        controls->addToGrid(exactDefenseButton, 0, 0);
        controls->addToGrid(exactAttackButton, 1, 0);
        controls->addToGrid(overlappingDefenseButton, 2, 0);
        controls->addToGrid(overlappingAttackButton, 3, 0);

        controls->setEnabled(false);

        GComboBox* choices = new GComboBox();
        for (const string& file: sampleProblems(kBasePath)) {
            choices->addItem(file);
        }
        choices->setEditable(false);
        mProblems = Temporary<GComboBox>(choices, window, "SOUTH");
        mProblems->setWidth(190);
        mSolutionsDisplay = Temporary<GColorConsole>(new GColorConsole(), window, "SOUTH");
        mSolutionsDisplay->setWidth(DISPLAY_WIDTH);

        loadWorld(choices->getSelectedItem());
    }

    void PokemonGUI::changeOccurredIn(GObservable* source) {
        if (source == mProblems) {
            loadWorld(mProblems->getSelectedItem());
        }
    }

    void PokemonGUI::toggleSelectedGym(GButton*& button) {
        string gymName = button->getText();
        if (mSelected.contains(gymName)) {
            mSelected.remove(gymName);
            button->setForeground(NOT_SELECTED);
        } else {
            mSelected.add(gymName);
            button->setForeground(SELECTED);
        }
        requestRepaint();
    }

    void PokemonGUI::clearSelections() {
        mSelected.clear();
        mAllCoverages.reset();
        mSolutionsDisplay->clearDisplay();
        mSolutionsDisplay->flush();
        gym1->setForeground(NOT_SELECTED);
        gym2->setForeground(NOT_SELECTED);
        gym3->setForeground(NOT_SELECTED);
        gym4->setForeground(NOT_SELECTED);
        gym5->setForeground(NOT_SELECTED);
        gym6->setForeground(NOT_SELECTED);
        gym7->setForeground(NOT_SELECTED);
        gym8->setForeground(NOT_SELECTED);
        elite4->setForeground(NOT_SELECTED);
        requestRepaint();
    }


    void PokemonGUI::actionPerformed(GObservable* source) {
        if (source == exactDefenseButton) {
            solveDefense(EXACT);
        } else if (source == exactAttackButton) {
            solveAttack(EXACT);
        } else if (source == overlappingDefenseButton) {
            solveDefense(OVERLAPPING);
        } else if (source == overlappingAttackButton) {
            solveAttack(OVERLAPPING);
        } else if (source == gym1) {
            toggleSelectedGym(gym1);
        } else if (source == gym2) {
            toggleSelectedGym(gym2);
        } else if (source == gym3) {
            toggleSelectedGym(gym3);
        } else if (source == gym4) {
            toggleSelectedGym(gym4);
        } else if (source == gym5) {
            toggleSelectedGym(gym5);
        } else if (source == gym6) {
            toggleSelectedGym(gym6);
        } else if (source == gym7) {
            toggleSelectedGym(gym7);
        } else if (source == gym8) {
            toggleSelectedGym(gym8);
        } else if (source == elite4) {
            toggleSelectedGym(elite4);
        } else if (source == clearChoices) {
            clearSelections();
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
        mAllCoverages.reset();
        mSelected.clear();
        mSolutionsDisplay->clearDisplay();
        mSolutionsDisplay->flush();
        controls->setEnabled(true);
        gymControls->setEnabled(true);
        gym1->setForeground(NOT_SELECTED);
        gym2->setForeground(NOT_SELECTED);
        gym3->setForeground(NOT_SELECTED);
        gym4->setForeground(NOT_SELECTED);
        gym5->setForeground(NOT_SELECTED);
        gym6->setForeground(NOT_SELECTED);
        gym7->setForeground(NOT_SELECTED);
        gym8->setForeground(NOT_SELECTED);
        elite4->setForeground(NOT_SELECTED);
        requestRepaint();
    }

    void PokemonGUI::solveDefense(const CoverageRequested& exactOrOverlapping) {
        mAllCoverages.reset();
        mSolutionsDisplay->clearDisplay();
        mSolutionsDisplay->flush();
        controls->setEnabled(false);
        gymControls->setEnabled(false);
        mProblems->setEnabled(false);

        bool resetOnFailure = false;

        std::set<std::string> gymAttackTypes = {};


        if (!mSelected.isEmpty()) {
            gymAttackTypes = loadSelectedGymsAttacks(mProblems->getSelectedItem(), mSelected);
            (*mSolutionsDisplay) << "Attack types to defend against:" << endl;
            (*mSolutionsDisplay) << "|";
            for (const auto& g : gymAttackTypes) {
                (*mSolutionsDisplay) << g << " | ";
            }
            (*mSolutionsDisplay) << endl;
        } else {
            resetOnFailure = true;
            gym1->setForeground(SELECTED);
            gym2->setForeground(SELECTED);
            gym3->setForeground(SELECTED);
            gym4->setForeground(SELECTED);
            gym5->setForeground(SELECTED);
            gym6->setForeground(SELECTED);
            gym7->setForeground(SELECTED);
            gym8->setForeground(SELECTED);
            elite4->setForeground(SELECTED);
            for (const auto& s : mGeneration.pokemonGenerationMap.network) {
                mSelected.add(s);
            }
            (*mSolutionsDisplay) << "Attack types to defend against:" << endl;
            (*mSolutionsDisplay) << "|";
            for (const auto& a : ALL_ATTACK_TYPES) {
                (*mSolutionsDisplay) << a << " | ";
            }
            (*mSolutionsDisplay) << " |" << endl;
        }

        // If gymAttackTypes is empty the constructor just builds the full generation of pokemon.
        PokemonLinks dlx(mGeneration.typeInteractions, gymAttackTypes);

        set<RankedSet<std::string>> solution = {};

        if (exactOrOverlapping == EXACT) {
            solution = dlx.getExactTypeCoverage();
        } else {
            solution = dlx.getOverlappingTypeCoverage();
        }

        mAllCoverages.reset(new set<RankedSet<std::string>>(solution));

        *mSolutionsDisplay << "Found " << (*mAllCoverages).size()
                           << " Pokemon teams [SCORE,TEAM]. Lower score is better." << endl;

        string maximumOutputExceeded = {};
        if ((*mAllCoverages).size() == MAX_OUTPUT_SIZE) {
            maximumOutputExceeded = "...exceeded maximum output, stopping at " + to_string(MAX_OUTPUT_SIZE);
        }
        *mSolutionsDisplay << maximumOutputExceeded << endl;
        for (const RankedSet<std::string>& cov : (*mAllCoverages)) {
            *mSolutionsDisplay << cov.rank() << " | ";
            for (const std::string& type : cov) {
                *mSolutionsDisplay << type << " | ";
            }
            *mSolutionsDisplay << endl;
        }
        *mSolutionsDisplay << maximumOutputExceeded << endl;


        /* Enable controls. */
        controls->setEnabled(true);
        mProblems->setEnabled(true);
        gymControls->setEnabled(true);

        if ((*mAllCoverages).size()) {
            requestRepaint();
        } else if (resetOnFailure) {
            mSelected.clear();
            gym1->setForeground(NOT_SELECTED);
            gym2->setForeground(NOT_SELECTED);
            gym3->setForeground(NOT_SELECTED);
            gym4->setForeground(NOT_SELECTED);
            gym5->setForeground(NOT_SELECTED);
            gym6->setForeground(NOT_SELECTED);
            gym7->setForeground(NOT_SELECTED);
            gym8->setForeground(NOT_SELECTED);
            elite4->setForeground(NOT_SELECTED);
        }
    }

    void PokemonGUI::solveAttack(const CoverageRequested& exactOrOverlapping) {
        mAllCoverages.reset();
        mSolutionsDisplay->clearDisplay();
        mSolutionsDisplay->flush();
        controls->setEnabled(false);
        mProblems->setEnabled(false);
        gymControls->setEnabled(false);

        bool resetOnFailure = false;
        map<string,set<Resistance>> modifiedGeneration = {};
        if (!mSelected.isEmpty()) {
            modifiedGeneration = loadSelectedGymsDefense(mGeneration.typeInteractions,
                                                         mProblems->getSelectedItem(),
                                                         mSelected);
            (*mSolutionsDisplay) << "Attacking the following defensive types:" << endl;
            (*mSolutionsDisplay) << "|";
            for (const auto& type : modifiedGeneration) {
                (*mSolutionsDisplay) << type.first << " | ";
            }
            (*mSolutionsDisplay) << endl;
        } else {
            resetOnFailure = true;
            gym1->setForeground(SELECTED);
            gym2->setForeground(SELECTED);
            gym3->setForeground(SELECTED);
            gym4->setForeground(SELECTED);
            gym5->setForeground(SELECTED);
            gym6->setForeground(SELECTED);
            gym7->setForeground(SELECTED);
            gym8->setForeground(SELECTED);
            elite4->setForeground(SELECTED);
            for (const auto& s : mGeneration.pokemonGenerationMap.network) {
                mSelected.add(s);
            }
            (*mSolutionsDisplay) << "Attacking the following defensive types:" << endl;
            (*mSolutionsDisplay) << "|";
            for (const auto& type : mGeneration.typeInteractions) {
                (*mSolutionsDisplay) << type.first << " | ";
            }
            (*mSolutionsDisplay) << endl;
        }

        set<RankedSet<std::string>> solution = {};
        if (modifiedGeneration.empty()) {
            PokemonLinks dlx(mGeneration.typeInteractions, PokemonLinks::ATTACK);
            if (exactOrOverlapping == EXACT) {
                solution = dlx.getExactTypeCoverage();
            } else {
                solution = dlx.getOverlappingTypeCoverage();
            }
        } else {
            PokemonLinks dlx(modifiedGeneration, PokemonLinks::ATTACK);
            if (exactOrOverlapping == EXACT) {
                solution = dlx.getExactTypeCoverage();
            } else {
                solution = dlx.getOverlappingTypeCoverage();
            }

        }


        mAllCoverages.reset(new set<RankedSet<std::string>>(solution));

        *mSolutionsDisplay << "Found " << (*mAllCoverages).size()
                           << " attack configurations [SCORE,TYPES]. Higher score is better." << endl;
        string maximumOutputExceeded = {};
        if ((*mAllCoverages).size() == MAX_OUTPUT_SIZE) {
            maximumOutputExceeded = "...exceeded maximum output, stopping at " + to_string(MAX_OUTPUT_SIZE);
        }
        *mSolutionsDisplay << maximumOutputExceeded << endl;
        for (auto it = (*mAllCoverages).rbegin(); it != (*mAllCoverages).rend(); it++) {
            *mSolutionsDisplay << it->rank() << " | ";
            for (const std::string& type : *it) {
                *mSolutionsDisplay << type << " | ";
            }
            *mSolutionsDisplay << endl;
        }
        *mSolutionsDisplay << maximumOutputExceeded << endl;


        /* Enable controls. */
        controls->setEnabled(true);
        mProblems->setEnabled(true);
        gymControls->setEnabled(true);

        if ((*mAllCoverages).size()) {
            requestRepaint();
        } else if (resetOnFailure) {
            mSelected.clear();
            gym1->setForeground(NOT_SELECTED);
            gym2->setForeground(NOT_SELECTED);
            gym3->setForeground(NOT_SELECTED);
            gym4->setForeground(NOT_SELECTED);
            gym5->setForeground(NOT_SELECTED);
            gym6->setForeground(NOT_SELECTED);
            gym7->setForeground(NOT_SELECTED);
            gym8->setForeground(NOT_SELECTED);
            elite4->setForeground(NOT_SELECTED);
        }
    }

}

GRAPHICS_HANDLER("Pokemon Planning", GWindow& window) {
    return std::make_shared<PokemonGUI>(window);
}
