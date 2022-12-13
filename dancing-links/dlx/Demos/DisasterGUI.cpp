#include "GUI/MiniGUI.h"
#include "DisasterParser.h"
#include <fstream>
#include <memory>
#include <string>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <vector>
#include "filelib.h"
#include "strlib.h"
#include "gthread.h"
#include "simpio.h"
#include "DisasterLinks.h"
#include "DisasterTags.h"
#include <regex>
using namespace std;
using namespace MiniGUI;

namespace {
    /* File constants. */
    const string kProblemSuffix = ".dst";
    const string kBasePath = "res/disaster-planning/";

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
    const vector<vector<CityColors>> kSolverColorOptions = {
        /* SET_BASED */
        {
            { "#101010", "#202020", Font(FontFamily::MONOSPACE, FontStyle::BOLD, 12, "#A0A0A0") },   // Uncovered
            { "#303060", "#404058", Font(FontFamily::MONOSPACE, FontStyle::BOLD, 12, "#C0C0C0") },   // Indirectly covered
            { "#806030", "#FFDF80", Font(FontFamily::MONOSPACE, FontStyle::BOLD, 12, "#000000") },   // Directly covered
        },
        /* QUAD_DLX */
        {
            { "#101010", "#202020", Font(FontFamily::MONOSPACE, FontStyle::BOLD, 12, "#A0A0A0") },   // Uncovered
            { "#303060", "#404058", Font(FontFamily::MONOSPACE, FontStyle::BOLD, 12, "#C0C0C0") },   // Indirectly covered
            { "#FF00FF", "#F5CBF5", Font(FontFamily::MONOSPACE, FontStyle::BOLD, 12, "#000000") },   // Directly covered
        },
        /* TAGGED_DLX */
        {
            { "#101010", "#202020", Font(FontFamily::MONOSPACE, FontStyle::BOLD, 12, "#A0A0A0") },   // Uncovered
            { "#303060", "#404058", Font(FontFamily::MONOSPACE, FontStyle::BOLD, 12, "#C0C0C0") },   // Indirectly covered
            { "#00FFFF", "#C5EBEB", Font(FontFamily::MONOSPACE, FontStyle::BOLD, 12, "#000000") },   // Directly covered
        },
    };

    /* Colors to use to draw the roads. */
    const string kDarkRoadColor    = "#505050";
    const string kLightRoadColor   = "#FFFFFF";

    /* Line thicknesses. */
    const double kRoadWidth = 3;
    const double kCityWidth = 1.5;

    /* Font to use for city labels. */
    const string kLabelFont        = "Monospace-BOLD-12";

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
                    const Set<string>& selected,
                    enum CitySolver solverUsed) {

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
            oval.setColor(kSolverColorOptions[solverUsed][state].borderColor);
            oval.setFillColor(kSolverColorOptions[solverUsed][state].fillColor);
            window.draw(oval,
                        center.x - kCityRadius,
                        center.y - kCityRadius);

            /* Set the label text and color. */
            auto render = TextRender::construct(shorthandFor(city), {
                                                    center.x - kCityRadius,
                                                    center.y - kCityRadius,
                                                    2 * kCityRadius,
                                                    2 * kCityRadius
                                                }, kSolverColorOptions[solverUsed][state].font);
            render->alignCenterHorizontally();
            render->alignCenterVertically();
            render->draw(window);
        }
    }

    void visualizeNetwork(GWindow& window,
                          const DisasterTest& network,
                          const Set<string>& selected,
                          enum CitySolver solverUsed) {
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
        if (!network.network.isEmpty()) {
            Geometry geo = geometryFor(window, network);

            /* Draw the roads under the cities to avoid weird graphics
             * artifacts.
             */
            drawRoads(window, geo, network, selected);
            drawCities(window, geo, network, selected, solverUsed);
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

    /* I wish I could use a function pointer or something but my DLX solvers need instantiation
     * of their respective classes and I am not sure how that would be united into one function
     * with a function pointer. Instead just do the same functions many times.
     */

    /* Uses binary search to find the optimal number of cities to use for disaster
     * preparedness, populating the result field with the minimum group of cities
     * that ended up being needed.
     */
    void solveOptimallyWithSets(const DisasterTest& test, Set<string>& result) {
        /* The variable 'low' is the lowest number that might be feasible.
         * The variable 'high' is the highest number that we know is feasible.
         */
        int low = 0, high = test.network.size();

        /* Begin with a feasible solution that uses as many cities as we'd like. */
        (void) canBeMadeDisasterReady(test.network, high, result);

        while (low < high) {
            /* This line looks weird, but it's designed to avoid integer overflows
             * on large inputs. The idea that (high + low) can overflow, but
             * (high - low) / 2 never will.
             */
            int mid = low + (high - low) / 2;
            Set<string> thisResult;

            /* If this option works, decrease high to it, since we know all is good. */
            if (canBeMadeDisasterReady(test.network, mid, thisResult)) {
                high = mid;
                result = thisResult; // Remember this result for later.
            }
            /* Otherwise, rule out anything less than or equal to it. */
            else {
                low = mid + 1;
            }
        }
    }


    void solveOptimallyWithQuadDLX(const DisasterTest& test, Set<string>& result) {
        /* The variable 'low' is the lowest number that might be feasible.
         * The variable 'high' is the highest number that we know is feasible.
         */
        int low = 0, high = test.network.size();

        /* Begin with a feasible solution that uses as many cities as we'd like. */
        DisasterLinks network(test.network);
        (void) network.isDisasterReady(high, result);

        while (low < high) {
            /* This line looks weird, but it's designed to avoid integer overflows
             * on large inputs. The idea that (high + low) can overflow, but
             * (high - low) / 2 never will.
             */
            int mid = low + (high - low) / 2;
            Set<string> thisResult;

            /* If this option works, decrease high to it, since we know all is good. */
            if (network.isDisasterReady(mid, thisResult)) {
                high = mid;
                result = thisResult; // Remember this result for later.
            }
            /* Otherwise, rule out anything less than or equal to it. */
            else {
                low = mid + 1;
            }
        }
    }

    void solveOptimallyWithSupplyTagDLX(const DisasterTest& test, Set<string>& result) {
        int low = 0, high = test.network.size();
        DisasterTags network(test.network);
        (void) network.hasDisasterCoverage(high, result);
        while (low < high) {
            int mid = low + (high - low) / 2;
            Set<string> thisResult;
            if (network.hasDisasterCoverage(mid, thisResult)) {
                high = mid;
                result = thisResult;
            }
            else {
                low = mid + 1;
            }
        }
    }

    /* These are all bad and slow. Right now I can only generate all viable configurations by
     * filtering out duplicate configurations with a Set. I then transfer all solutions from the
     * set to the vector to make it work better with the GUI. I would like to only generate unique
     * coverage schemes so I can use a vector from the beggining. I haven't figured it out.
     */

    void solveAllWithSets(const DisasterTest& test, unique_ptr<vector<Set<string>>>& allSolutions) {
        int low = 0, high = test.network.size();
        Set<string> result = {};
        (void) canBeMadeDisasterReady(test.network, high, result);
        while (low < high) {
            int mid = low + (high - low) / 2;
            Set<string> thisResult;
            if (canBeMadeDisasterReady(test.network, mid, thisResult)) {
                high = mid;
                result = thisResult;
            }
            else {
                low = mid + 1;
            }
        }
        int optimalSupplies = result.size();
        Set<Set<string>> allFoundConfigs = findAllSupplySchemes(test.network, optimalSupplies);
        for (const auto& found : allFoundConfigs) {
            (*allSolutions).push_back(found);
        }
    }

    void solveAllWithQuadDLX(const DisasterTest& test,
                             unique_ptr<vector<Set<string>>>& allSolutions) {
        int low = 0, high = test.network.size();
        Set<string> result = {};
        DisasterLinks network(test.network);
        (void) network.isDisasterReady(high, result);
        while (low < high) {
            int mid = low + (high - low) / 2;
            Set<string> thisResult;

            if (network.isDisasterReady(mid, thisResult)) {
                high = mid;
                result = thisResult;
            }
            else {
                low = mid + 1;
            }
        }
        int optimalSupplies = result.size();
        Set<Set<string>> allFoundConfigs = findAllSupplySchemes(test.network, optimalSupplies);
        for (const auto& found : allFoundConfigs) {
            (*allSolutions).push_back(found);
        }
    }

    void solveAllWithSupplyTagDLX(const DisasterTest& test,
                                  unique_ptr<vector<Set<string>>>& allSolutions) {
        int low = 0, high = test.network.size();
        Set<string> result = {};
        DisasterTags network(test.network);
        (void) network.hasDisasterCoverage(high, result);
        while (low < high) {
            int mid = low + (high - low) / 2;
            Set<string> thisResult;
            if (network.hasDisasterCoverage(mid, thisResult)) {
                high = mid;
                result = thisResult;
            }
            else {
                low = mid + 1;
            }
        }
        int optimalSupplies = result.size();
        Set<Set<string>> allFoundConfigs = findAllSupplySchemes(test.network, optimalSupplies);
        for (const auto& found : allFoundConfigs) {
            (*allSolutions).push_back(found);
        }
    }

    class DisasterGUI: public ProblemHandler {
    public:
        DisasterGUI(GWindow& window);

        void actionPerformed(GObservable* source) override;
        void changeOccurredIn(GObservable* source) override;

    protected:
        void repaint() override;

    private:
        /* Dropdown of all the problems to choose from. */
        Temporary<GComboBox> mProblems;

        /* Dropdown of the solver you want to use for the problem */
        Temporary<GComboBox> mSolver;
        enum CitySolver mSolverUsed;
        const string mSetSolver = "Solver: Set Based";
        const string mQuadDLXSolver = "Solver: Quadruple Linked DLX";
        const string mSupplyTagDLXSolver = "Solver: Supply Tag DLX";
        const vector<string> mSolverNames = {mSetSolver, mQuadDLXSolver, mSupplyTagDLXSolver};

        /* Button to trigger the solver. */
        Temporary<GButton> mSolve;

        /* Button to trigger finding all distributions of optimal supplies. */
        Temporary<GButton> mPrevSolution;
        Temporary<GButton> mAllSolutions;
        Temporary<GButton> mNextSolution;
        unique_ptr<vector<Set<string>>> mStoredSolutions;
        int mCurrentSolutionIndex;
        const string mAllSolutionsMessage = "Solutions Found:";

        /* Current network and solution. */
        DisasterTest    mNetwork;
        Set<string> mSelected;

        /* Loads the world with the given name. */
        void loadWorld(const string& filename);

        /* Computes an optimal solution. */
        void solve();
        /* Computes all optimal solutions. */
        void showPreviousSolution();
        void solveAll();
        void showNextSolution();
    };

    DisasterGUI::DisasterGUI(GWindow& window) : ProblemHandler(window) {
        GComboBox* choices = new GComboBox();
        for (const string& file: sampleProblems(kBasePath)) {
            choices->addItem(file);
        }
        choices->setEditable(false);

        GComboBox* solvers = new GComboBox();
        for (const string& solver : mSolverNames) {
            solvers->addItem(solver);
        }
        solvers->setEditable(false);

        mProblems = Temporary<GComboBox>(choices, window, "SOUTH");
        /* Select the implementation you want to solve the problems */
        mSolver = Temporary<GComboBox>(solvers, window, "SOUTH");
        mSolve    = Temporary<GButton>(new GButton("Solve"), window, "SOUTH");
        mPrevSolution = Temporary<GButton>(new GButton("<<"), window, "SOUTH");
        /* These implementations are fast enough to find all optimal solutions in good time. */
        mAllSolutions = Temporary<GButton>(new GButton("All Optimal Solutions"), window, "SOUTH");
        mNextSolution = Temporary<GButton>(new GButton(">>"), window, "SOUTH");
        mPrevSolution->setEnabled(false);
        mNextSolution->setEnabled(false);
        mCurrentSolutionIndex = 0;

        loadWorld(choices->getSelectedItem());
    }

    void DisasterGUI::changeOccurredIn(GObservable* source) {
        if (source == mProblems) {
            loadWorld(mProblems->getSelectedItem());
        }
    }

    void DisasterGUI::actionPerformed(GObservable* source) {
        if (source == mSolve) {
            solve();
        } else if (source == mAllSolutions) {
            solveAll();
        } else if (source == mPrevSolution) {
            showPreviousSolution();
        } else if (source == mNextSolution) {
            showNextSolution();
        }
    }

    void DisasterGUI::repaint() {
        /* Added functionality to draw the network different colors based on solver used. */
        visualizeNetwork(window(), mNetwork, mSelected, mSolverUsed);
    }

    void DisasterGUI::loadWorld(const string& filename) {
        ifstream input(kBasePath + filename);
        if (!input) error("Cannot open file.");

        mNetwork = loadDisaster(input);
        mSelected.clear();
        mStoredSolutions.reset();
        mPrevSolution->setEnabled(false);
        mNextSolution->setEnabled(false);
        requestRepaint();
    }

    void DisasterGUI::solve() {
        /* Clear out any old solution. We're going to get a new one. */
        mSelected.clear();
        mStoredSolutions.reset();

        /* Disable all controls until the operation finishes. */
        mSolve->setEnabled(false);
        mProblems->setEnabled(false);
        mSolver->setEnabled(false);
        mAllSolutions->setEnabled(false);
        mPrevSolution->setEnabled(false);
        mNextSolution->setEnabled(false);

        if (mSolver->getSelectedItem() == mSetSolver) {
            mSolverUsed = SET_BASED;
            solveOptimallyWithSets(mNetwork, mSelected);
        } else if (mSolver->getSelectedItem() == mQuadDLXSolver) {
            mSolverUsed = QUAD_DLX;
            solveOptimallyWithQuadDLX(mNetwork, mSelected);
        } else if (mSolver->getSelectedItem() == mSupplyTagDLXSolver) {
            mSolverUsed = TAGGED_DLX;
            solveOptimallyWithSupplyTagDLX(mNetwork, mSelected);
        }

        /* Enable controls. */
        mSolve->setEnabled(true);
        mProblems->setEnabled(true);
        mSolver->setEnabled(true);
        mAllSolutions->setEnabled(true);

        requestRepaint();
    }

    void DisasterGUI::showPreviousSolution() {
        if (mStoredSolutions && (*mStoredSolutions).size() > 1) {
            if (--mCurrentSolutionIndex < 0) {
                mCurrentSolutionIndex = (*mStoredSolutions).size() - 1;
            }
            mSelected = (*mStoredSolutions)[mCurrentSolutionIndex];
            requestRepaint();
        }
    }

    void DisasterGUI::showNextSolution() {
        if (mStoredSolutions && (*mStoredSolutions).size() > 1) {
            ++mCurrentSolutionIndex %= (*mStoredSolutions).size();
            mSelected = (*mStoredSolutions)[mCurrentSolutionIndex];
            requestRepaint();
        }
    }

    void DisasterGUI::solveAll() {
        /* Clear out any old solution. We're going to get a new one. */
        mSelected.clear();
        mStoredSolutions.reset(new vector<Set<string>>{});

        /* Disable all controls until the operation finishes. */
        mSolve->setEnabled(false);
        mProblems->setEnabled(false);
        mSolver->setEnabled(false);
        mAllSolutions->setEnabled(false);
        mCurrentSolutionIndex = 0;

        if (mSolver->getSelectedItem() == mSetSolver) {
            mSolverUsed = SET_BASED;
            solveAllWithSets(mNetwork, mStoredSolutions);
        } else if (mSolver->getSelectedItem() == mQuadDLXSolver) {
            mSolverUsed = QUAD_DLX;
            solveAllWithQuadDLX(mNetwork, mStoredSolutions);
        } else if (mSolver->getSelectedItem() == mSupplyTagDLXSolver) {
            mSolverUsed = TAGGED_DLX;
            solveAllWithSupplyTagDLX(mNetwork, mStoredSolutions);
        }

        mSelected = (*mStoredSolutions)[mCurrentSolutionIndex];

        /* Are there other ways to dynamically display messages besides a pop up? */
        GOptionPane::showMessageDialog(&window(), to_string((*mStoredSolutions).size()), mAllSolutionsMessage);
        /* Enable controls. */
        mSolve->setEnabled(true);
        mProblems->setEnabled(true);
        mSolver->setEnabled(true);
        mAllSolutions->setEnabled(true);
        mPrevSolution->setEnabled(true);
        mNextSolution->setEnabled(true);

        requestRepaint();
    }
}

GRAPHICS_HANDLER("Disaster Planning", GWindow& window) {
    return make_shared<DisasterGUI>(window);
}

namespace {
    /* Displays the given transportation grid. */
    void displayMap(const Map<string, Set<string>>& network) {
        cout << "This transportation grid has " << pluralize(network.size(), "city", "cities") << "." << endl;
        for (string city: network) {
            cout << "  The city " << city << " is adjacent to " << pluralize(network[city].size(), "city", "cities") << "." << endl;
            for (string neighbor: network[city]) {
                cout << "    " << neighbor << endl;
            }
        }
    }

    /* Displays the cities used in an optimal solution. */
    void displayBestCities(const Set<string>& cities) {
        cout << "You need to stockpile in " << pluralize(cities.size(), "city", "cities") << " to provide coverage." << endl;
        for (string city: cities) {
            cout << "  " << city << endl;
        }
    }

    void demoDisasterPlanning() {
        cout << "Disaster Planning" << endl;
        do {
            ifstream input(makeFileSelection(".dst"));
            if (!input) error("Internal error - not your fault: Can't open the chosen file.");

            auto scenario = loadDisaster(input);

            displayMap(scenario.network);

            cout << "Running your code to find the fewest number of cities needed... " << flush;
            Set<string> cities;
            solveOptimallyWithSets(scenario, cities);
            cout << "done!" << endl;

            displayBestCities(cities);
        } while (getYesOrNo("Try another demo file? "));
    }
}

CONSOLE_HANDLER("Disaster Planning") {
    demoDisasterPlanning();
}
