/**
 * Author: Alexander G. Lopez
 * File DisasterTags.cpp
 * --------------------------
 * This file contains the implementation for Algorithm X via Dancing Links with a new method I'm
 * calling Supply Tags. For more information, see the mini-writeup in the .h file or the detailed
 * write up in the README.md.
 */
#include <cmath>
#include <limits.h>
#include "DisasterTags.h"
#include "Utilities/DisasterUtilities.h"


/* * * * * * * * * * * * *  Algorithm X via Dancing Links with Depth Tags * * * * * * * * * * * * */


 /**
 * @brief hasDisasterCoverage  performs a recursive search to determine if a transportation grid
 *                             can be covered with the specified number of emergency supplies.
 *                             Places the found cities in the output parameter if there
 *                             exists a solution. A city is covered if it has supplies or
 *                             is adjacent to a city with supplies. The solution may not use all
 *                             of the provided supplies.
 * @param numSupplies          the limiting number of supplies we must distribute.
 * @param suppliedCities       the output parameter telling which cities received supplies.
 * @return                     true if we have found a viable supply scheme, false if not.
 */
bool DisasterTags::hasDisasterCoverage(int numSupplies, Set<std::string>& supplyLocations) {
    if (numSupplies < 0) {
        error("negative supplies");
    }
    if (numItemsAndOptions_ == 0) {
        return true;
    }
    return isDLXCovered(numSupplies, supplyLocations);
}

/**
 * @brief isDLXCovered     performs an in-place recursive search on a dancing links data
 *                         structure to determine if a transportation grid is safe with the
 *                         given number of supplies. A city is safe if it has supplies or is
 *                         adjacent to a city with supplies. All cities are tagged with the
 *                         supply number that has covered them either by being next to a city
 *                         with that supply number or holding the supply themselves.
 * @param numSupplies      the depth or our recursive search. How many supplies we can give out.
 * @param supplyLocations  the output parameter upon successfull coverage. Empty if we fail.
 * @return                 true if we can cover the grid with the given supplies, false if not.
 */
bool DisasterTags::isDLXCovered(int numSupplies, Set<std::string>& supplyLocations) {
    if (table_[0].right == 0 && numSupplies >= 0) {
        return true;
    }
    if (numSupplies <= 0) {
        return false;
    }

    int chosenIndex = chooseIsolatedCity();

    /* Try to cover this city by first supplying the most connected city nearby. Try cities with
     * successively fewer connections then if all else fails supply the isolated city itself.
     */
    for (int cur = grid_[chosenIndex].down; cur != chosenIndex; cur = grid_[cur].down) {

        // Tag every city with the supply number so we know which cities to uncover if this fails.
        std::string supplyLocation = coverCity(cur, numSupplies);

        if (isDLXCovered(numSupplies - 1, supplyLocations)) {
            // Only add to the output if successful and be sure to cleanup in case it runs again.
            supplyLocations.add(supplyLocation);
            uncoverCity(cur);
            return true;
        }

        // We will know which cities to uncover thanks to which we tagged on the way down.
        uncoverCity(cur);
    }
    return false;
}

/**
 * @brief getAllDisasterConfigurations  returns every possible disaster configuration possible
 *                                      with a given supply count. I advise finding the optimal
 *                                      number of supplies before running this function or it
 *                                      will work very hard. It is slow.
 * @param numSupplies                   the number of supplies we have to distribute.
 * @return                              all possible distributions of the supplies.
 */
Set<Set<std::string>> DisasterTags::getAllDisasterConfigurations(int numSupplies) {
    if (numSupplies < 0) {
        error("Negative supply count.");
    }

    Set<std::string> suppliedCities {};
    Set<Set<std::string>> allConfigurations = {};
    fillConfigurations(numSupplies, suppliedCities, allConfigurations);
    return allConfigurations;
}

/**
 * @brief fillConfigurations  finds all possible distributions of the given number of supplies.
 *                            It generates duplicate configurations and uses a Set to filter
 *                            them out. This is slow and I want to only generate unique
 *                            configurations but am having trouble finding a way.
 * @param numSupplies         the number of supplies we have to distribute.
 * @param suppliedCities      the set that will hold new configurations that work.
 * @param allConfigurations   the set that records all configurations found.
 */
void DisasterTags::fillConfigurations(int numSupplies,
                                      Set<std::string>& suppliedCities,
                                      Set<Set<std::string>>& allConfigurations) {

    if (table_[0].right == 0 && numSupplies >= 0) {
        allConfigurations.add(suppliedCities);
        return;
    }
    if (numSupplies <= 0) {
        return;
    }
    int chosenIndex = chooseIsolatedCity();

    for (int cur = grid_[chosenIndex].down; cur != chosenIndex; cur = grid_[cur].down) {

        std::string supplyLocation = coverCity(cur, numSupplies);
        suppliedCities.add(supplyLocation);

        fillConfigurations(numSupplies - 1, suppliedCities, allConfigurations);

        suppliedCities.remove(supplyLocation);
        uncoverCity(cur);
    }

}

/**
 * @brief chooseIsolatedCity  selects a city we are trying to cover either by giving it supplies or
 *                            covering an adjacent neighbor. The selection uses the following
 *                            heuristic:
 *                              - Select the most isolated city so far.
 *                              - We must cover this city so select an adjacent city with the
 *                                most connections and try that first.
 *                              - If that fails we try the next adjacent city with most connections.
 *                              - Finally, if all other neighbors fail, try to supply the city
 *                                in question, not neighbors.
 * @return                    the index of the city we are selecting to attempt to cover.
 */
int DisasterTags::chooseIsolatedCity() {
    int min = INT_MAX;
    int chosenIndex = 0;
    int head = 0;
    for (int cur = table_[0].right; cur != head; cur = table_[cur].right) {
        if (grid_[cur].topOrLen < min) {
            chosenIndex = cur;
            min = grid_[cur].topOrLen;
        }
    }
    return chosenIndex;
}

/**
 * @brief coverCity      covers a city wit supplies and all of its neighbors. All cities tagged
 *                       with a supply number equivalent to the current depth of the recursive
 *                       search. This tells us which cities are now unsafe if distributing that
 *                       exact supply did not work out and it must be removed later.
 * @param indexInOption  the index for the current city we are trying to cover.
 * @param supplyTag      uses number of supplies remaining as a unique tag for recursive depth.
 * @return               the name of the city holding the supplies.
 */
std::string DisasterTags::coverCity(int indexInOption, const int supplyTag) {
    int i = indexInOption;
    std::string result = "";
    do {
        int top = grid_[i].topOrLen;
        if (top <= 0) {
            /* We are always guaranteed to pass the spacer tile so we will collect the name of the
             * city we have chosen to supply to prove our algorithm chose correctly.
            */
            i = grid_[i].up;
            result = table_[std::abs(grid_[i - 1].topOrLen)].name;
        } else {
            /* Cities are "tagged" at the recursive depth at which they were given supplies, the
             * number of supplies remaining when distributed. Only give supplies to cities that
             * are still in need and have not been tagged.
             */
            if (!grid_[top].supplyTag) {
                grid_[top].supplyTag = supplyTag;
                table_[table_[top].left].right = table_[top].right;
                table_[table_[top].right].left = table_[top].left;
            }
            grid_[i++].supplyTag = supplyTag;
        }
    } while (i != indexInOption);

    return result;
}

/**
 * @brief uncoverCity    uncovers a city if that choice of option did not lead to a covered
 *                       network. Uncovers same option that was selected for coverage if given
 *                       the same index. Will only uncover those cities that were covered by
 *                       the previously given supply.
 * @param indexInOption  the index of the item we covered with the option below the index.
 */
void DisasterTags::uncoverCity(int indexInOption) {
    /* We must go in the reverse direction we started, fixing the first city we covered in the
     * lookup table last. This a requirement due to leaving the pointers of doubly linked left-right
     * list in place. Otherwise, table will not be fixed correctly.
     */
    int i = --indexInOption;
    do {
        int top = grid_[i].topOrLen;
        if (top < 0) {
            i = grid_[i].down;
        } else {
            /* This is the key optimization of this algorithm. Only reset a city to uncovered if
             * we know we are taking away the same supply we gave when covering this city. A simple
             * O(1) check beats an up,down,left,right pointer implementation that needs to splice
             * from a left right doubly linked list for an entire column.
             */
            if (grid_[top].supplyTag == grid_[i].supplyTag) {
                grid_[top].supplyTag = 0;
                table_[table_[top].left].right = top;
                table_[table_[top].right].left = top;
            }
            grid_[i--].supplyTag = 0;
        }
    } while (i != indexInOption);
}


/* * * * * * * * * * *  Constructor and Building of Dancing Links Network   * * * * * * * * * * * */


/**
 * @brief DisasterTags  a custom constructor for this class that can turn a Map representation
 *                      of a transportation grid into a vector grid prepared for exact cover
 *                      search via dancing links.
 * @param roadNetwork   the transportation grid passed in via map form.
 */
DisasterTags::DisasterTags(const Map<std::string, Set<std::string>>& roadNetwork) :
                           table_({}),
                           grid_({}),
                           numItemsAndOptions_(0) {

    // We will set this up for a reverse build of column links for a given item.
    HashMap<std::string,int> columnBuilder = {};
    std::vector<std::pair<std::string,int>> connectionSizes = {};

    // We need to start preparing items in the grid immediately after the headers.
    initializeHeaders(roadNetwork, connectionSizes, columnBuilder);

    /* The second pass will fill in the columns and keep the headers and all elements appropriately
     * updated. We can hang on to helpful index info.
     */
    initializeItems(roadNetwork, connectionSizes, columnBuilder);
}

/**
 * @brief initializeHeaders  creates the lookup table of city names and the first row of headers
 *                           that are in the dancing links array. This is the first pass on the
 *                           input map. We will perform a second, much longer pass to build the
 *                           columns later.
 * @param roadNetwork        the input of cities connected to other cities.
 * @param connectionSizes    a city and the size of the set of adjacent cities.
 * @param columnBuilder      the hash map we will use to connect newly added items to a column.
 */
void DisasterTags::initializeHeaders(const Map<std::string, Set<std::string>>& roadNetwork,
                                      std::vector<std::pair<std::string,int>>& connectionSizes,
                                      HashMap<std::string,int>& columnBuilder) {
    table_.push_back({"", 0, 1});
    grid_.push_back({0,0,0,0});
    int index = 1;
    // The first pass will set up the name headers and the column headers in the two vectors.
    for (const std::string& city : roadNetwork) {

        // Add one to the connection size because we will add a city to its own connections later.
        connectionSizes.push_back({city, roadNetwork[city].size() + 1});

        // We need to set up multiple columns, so begin tracking the previous item for a column.
        columnBuilder[city] = index;

        table_.push_back({city, index - 1, index + 1});
        table_[0].left++;
        // Add the first headers for the item vector. They need count up and down.
        grid_.push_back({0, index, index,0});
        numItemsAndOptions_++;
        index++;
    }

    /* We want the most isolated cities to try to cover themselves by supplying adjacent neighbor
     * with the most other connections. This is not garunteed to work every time, but most times
     * this is the best strategy to not waste time supplying very isolated cities first. So organize
     * the options by most connections to fewest.
     */
    std::sort(connectionSizes.begin(), connectionSizes.end(), [](auto &left, auto &right) {
        return left.second > right.second;
    });

    table_[table_.size() - 1].right = 0;
}

/**
 * @brief initializeItems  builds the structure needed to perform the dancing links algorithm.
 *                         This focusses on setting up the grid in the dancing Network struct
 *                         so that all item columns are tallied correctly and the option rows
 *                         represent all the cities that a supply city can cover, self included.
 * @param roadNetwork      we need to look back at the original map to grab sets to build.
 * @param connectionSizes  we organize rows in descending order top to bottom as a heuristic.
 * @param columnBuilder    the map we use to help build an accurate column for each city item.
 */
void DisasterTags::initializeItems(const Map<std::string, Set<std::string>>& roadNetwork,
                                    const std::vector<std::pair<std::string,int>>& connectionSizes,
                                    HashMap<std::string,int>& columnBuilder) {
    int previousSetSize = grid_.size();
    int index = grid_.size();

    for (const auto& [city, connectionSize] : connectionSizes) {
        // This algorithm includes a city in its own set of connections.
        Set<std::string> connections = roadNetwork[city] + city;

        /* We will know which supplying city option an item is in by the spacerTitle.
         * lookupTable[abs(-grid_[columnBuilder[city]].down)] will give us the name of the option
         * of that row. This accesses the last city in a column's down field to get the header.
         */
        grid_.push_back({-grid_[columnBuilder[city]].down,  // Negative index of city as option.
                         index - previousSetSize,           // First item in previous option
                         index + connections.size(),        // Last item in current option
                         0});                               // Supply number tag.

        // Manage column pointers for items connected across options. Update index.
        index = initializeColumns(connections, columnBuilder, index);

        previousSetSize = connections.size();
    }
    grid_.push_back({INT_MIN, index - previousSetSize, INT_MIN,0});
}

/**
 * @brief initializeColumns  this is the set builder for each row. When a city is given supplies
 *                           it is connected to itself and its adjacent cities. We represent
 *                           this in a row and connect each item to any appearance in a previous
 *                           row so that each column is built.
 * @param connections        the set containing the city and all adjacent connections.
 * @param columnBuilder      the map we use to track the last appearance of an item in a column.
 * @param index              the index of the array at which we start building.
 * @return                   the new index of the grid after adding all items in a row.
 */
int DisasterTags::initializeColumns(const Set<std::string>& connections,
                                     HashMap<std::string,int>& columnBuilder,
                                     int index) {
    for (const auto& c : connections) {
        // Circular lists give us access to header with down field of last city in a column.
        grid_[grid_[columnBuilder[c]].down].topOrLen++;
        index++;

        // A single item in a circular doubly linked list points to itself.
        grid_.push_back({grid_[columnBuilder[c]].down, index, index,0});

        /* Now we need to handle building the up and down pointers for a column of items.
         * We also must make sure to keep the most recent element pointing down to the
         * first of a column because this is circular. The first elem in the column must
         * also point up to the most recently added element because this is circular.
         */

        // This is the necessary adjustment to the column header's up field for a given item.
        grid_[grid_[columnBuilder[c]].down].up = index;

        // The current node is now the new tail in a vertical circular linked list for an item.
        grid_[index].up = columnBuilder[c];
        grid_[index].down = grid_[columnBuilder[c]].down;

        // Update the old tail to reflect the new addition of an item in its option.
        grid_[columnBuilder[c]].down = index;

        // Similar to a previous/current coding pattern but in an above/below column.
        columnBuilder[c] = index;
    }
    return ++index;
}


/* * * * * * * * * * * * * * * * *        Debugging Operators           * * * * * * * * * * * * * */


bool operator==(const DisasterTags::city& lhs, const DisasterTags::city& rhs) {
    return lhs.topOrLen == rhs.topOrLen && lhs.up == rhs.up && lhs.down == rhs.down;
}

bool operator!=(const DisasterTags::city& lhs, const DisasterTags::city& rhs) {
    return !(lhs == rhs);
}

bool operator==(const DisasterTags::cityName& lhs, const DisasterTags::cityName& rhs) {
    return lhs.name == rhs.name && lhs.left == rhs.left && lhs.right == rhs.right;
}

bool operator!=(const DisasterTags::cityName& lhs, const DisasterTags::cityName& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, const DisasterTags::city& person) {
    os << "{ topOrLen: " << person.topOrLen
       << ", up: " << person.up << ", down: " << person.down << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const DisasterTags::cityName& name) {
    os << "{ name: " << name.name << ", left: " << name.left << ", right: " << name.right << " }";
    return os;
}

std::ostream& operator<<(std::ostream&os, const std::vector<DisasterTags::cityName>& links) {
    os << "LOOKUP TABLE" << std::endl;
    for (const auto& item : links) {
        os << "{\"" << item.name << "\"," << item.left << "," << item.right << "}" << std::endl;
    }
    os << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream&os, const std::vector<DisasterTags::city>& links) {
    os << "DLX ARRAY" << std::endl;
    int index = 0;
    for (const auto& item : links) {
        if (item.topOrLen < 0) {
            os << std::endl;
        }
        os << "{"
           << item.topOrLen << ","
           << item.up << ","
           << item.down << ","
           << item.supplyTag << "}";
        index++;
    }
    os << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream&os, const DisasterTags& links) {
    os << "LOOKUP ARRAY" << std::endl;
    for (const auto& header : links.table_) {
        os << "{\""
           << header.name << "\","
           << header.left << ","
           << header.right << "},"
           << std::endl;
    }
    os << "DLX ARRAY" << std::endl;
    int index = 0;
    for (const auto& item : links.grid_) {
        if (index >= links.table_.size() && item.topOrLen < 0) {
            os << std::endl;
        }
        os << "{"
           << item.topOrLen << ","
           << item.up << ","
           << item.down << ","
           << item.supplyTag << "},";
        index++;
    }
    os << std::endl;
    os << "Number of Cities: " << links.numItemsAndOptions_ << std::endl;
    return os;
}


/* * * * * * * * * * * * * * * * *     Test Cases Below This Point      * * * * * * * * * * * * * */


/* * * * * * * * * * * * * * * * *          Initialization Tests        * * * * * * * * * * * * * */


STUDENT_TEST("Initialize a small dancing links.") {
    /*
     *
     *             B
     *             |
     *        A -- C
     *
     *
     *
     */
    const Map<std::string, Set<std::string>> cities = {
        {"A", {"C"}},
        {"B", {"C"}},
        {"C", {"A", "B"}},
    };
    std::vector<DisasterTags::cityName> networkHeaders = {
        {"",  3, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 0}
    };
    std::vector<DisasterTags::city> dlxItems = {
        // 0           1A        2B        3C
        {0,0,0,0},  {2,9,5,0},{2,12,6,0},{3,13,7,0},
        // 4           5A        6B        7C
        {-3,0,7,0}, {1,1,9,0},{2,2,12,0},{3,3,10,0},
        // 8        9A                     10C
        {-1,5,10,0},{1,5,1,0},           {3,7,13,0},
        // 11                    12A       13C
        {-2,9,13,0},          {2,6,2,0}, {3,10,3,0},
        {INT_MIN,12,INT_MIN,0},
    };
    DisasterTags network(cities);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}

STUDENT_TEST("Initialize larger dancing links.") {
    /*
     *
     *        F -- D -- B -- E -- C -- A
     *
     */
    const Map<std::string, Set<std::string>> cities = {
        {"A", {"C"}},
        {"B", {"D", "E"}},
        {"C", {"A", "E"}},
        {"D", {"B", "F"}},
        {"E", {"B", "C"}},
        {"F", {"D"}},
    };
    std::vector<DisasterTags::cityName> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    std::vector<DisasterTags::city> dlxItems = {
        //  0            1A          2B          3C         4D           5E          6F
        {0,0,0,0},   {2,24,12,0},{3,20,8,0}, {3,25,13,0},{3,27,9,0}, {3,22,10,0},{2,28,18,0},
        //  7                       8B                      9D          10E
        {-2,0,10,0},             {2,2,16,0},             {4,4,17,0}, {5,5,14,0},
        //  11          12A                     13C                     14E
        {-3,8,14,0}, {1,1,24,0},             {3,3,21,0},             {5,10,22,0},
        //  15                     16B                      17D                     18F
        {-4,12,18,0},            {2,8,20,0},             {4,9,27,0},             {6,6,28,0},
        //  19                     20B          21C                     22E
        {-5,16,22,0},            {2,16,2,0}, {3,13,25,0},            {5,14,5,0},
        //  23         24A                      25C
        {-1,20,25,0},{1,12,1,0},             {3,21,3,0},
        //  26                                              27D         28E
        {-6,24,28,0},                                    {4,17,4,0}, {6,18,6,0},
        {INT_MIN,27,INT_MIN,0},
    };
    DisasterTags network(cities);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}

STUDENT_TEST("Simple Ethene Network initialization.") {
    /*
     *
     *             C
     *             |
     *        A -- D -- B -- F
     *                  |
     *                  E
     *
     */
    const Map<std::string, Set<std::string>> cities = {
        {"A", {"D"}},
        {"B", {"D", "E", "F"}},
        {"C", {"D"}},
        {"D", {"A", "C", "B"}},
        {"E", {"B"}},
        {"F", {"B"}},
    };
    std::vector<DisasterTags::cityName> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    std::vector<DisasterTags::city> dlxItems = {
        //  0            1A          2B         3C          4D         5E          6F
        {0,0,0,0},   {2,18,13,0},{4,27,8,0},{2,21,15,0},{4,22,9,0},{2,25,10,0},{2,28,11,0},
        //  7                        8B                     9D         10E         11F
        {-2,0,11,0},             {2,2,14,0},            {4,4,16,0},{5,5,25,0}, {6,6,28,0},
        //  12          13A          14B        15C        16D
        {-4,8,16,0}, {1,1,18,0}, {2,8,24,0},{3,3,21,0}, {4,9,19,0},
        //  17          18A                                19D
        {-1,13,19,0},{1,13,1,0},                        {4,16,22,0},
        //  20                                  21C        22D
        {-3,18,22,0},                       {3,15,3,0}, {4,19,4,0},
        //  23                       24B                              25E
        {-5,21,25,0},            {2,14,27,0},                      {5,10,5,0},
        //  26                       27B                                           28F
        {-6,24,28,0},            {2,24,2,0},                                    {6,11,6,0},
        {INT_MIN,27,INT_MIN,0},
    };
    DisasterTags network (cities);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}


/* * * * * * * * * * * * * * * * *      Cover/Uncover Tests             * * * * * * * * * * * * * */


STUDENT_TEST("Cover uncover A should only cover A and C from header list.") {
    /*
     *
     *             B
     *             |
     *        A -- C
     *
     *
     *
     */
    const Map<std::string, Set<std::string>> cities = {
        {"A", {"C"}},
        {"B", {"C"}},
        {"C", {"A", "B"}},
    };
    std::vector<DisasterTags::cityName> networkHeaders = {
        {"",  3, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 0}
    };
    std::vector<DisasterTags::city> dlxItems = {
        // 0           1A        2B        3C
        {0,0,0,0},  {2,9,5,0},{2,12,6,0},{3,13,7,0},
        // 4           5A        6B        7C
        {-3,0,7,0}, {1,1,9,0},{2,2,12,0},{3,3,10,0},
        // 8        9A                     10C
        {-1,5,10,0},{1,5,1,0},           {3,7,13,0},
        // 11                    12A       13C
        {-2,9,13,0},          {2,6,2,0}, {3,10,3,0},
        {INT_MIN,12,INT_MIN,0},
    };
    DisasterTags network(cities);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);

    std::string location = network.coverCity(9,1);
    EXPECT_EQUAL(location, "A");
    std::vector<DisasterTags::cityName> headersCoverA {
        {"",  2, 2},
        {"A", 0, 2},
        {"B", 0, 0},
        {"C", 2, 0}
    };
    std::vector<DisasterTags::city> dlxCoverA = {
        // 0           1A        2B        3C
        {0,0,0,0},  {2,9,5,1},{2,12,6,0},{3,13,7,1},
        // 4           5A        6B        7C
        {-3,0,7,0}, {1,1,9,0},{2,2,12,0},{3,3,10,0},
        // 8           9A                  10C
        {-1,5,10,0},{1,5,1,1},           {3,7,13,1},
        // 11                    12A       13C
        {-2,9,13,0},          {2,6,2,0}, {3,10,3,0},
        {INT_MIN,12,INT_MIN,0},
    };
    EXPECT_EQUAL(network.table_, headersCoverA);
    EXPECT_EQUAL(network.grid_, dlxCoverA);

    network.uncoverCity(9);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}

STUDENT_TEST("Recursive depth field prevents uncovering cities that should remain covered.") {
    /*
     *
     *             C
     *             |
     *        A -- D -- B -- F
     *                  |
     *                  E
     *
     */
    const Map<std::string, Set<std::string>> cities = {
        {"A", {"D"}},
        {"B", {"D", "E", "F"}},
        {"C", {"D"}},
        {"D", {"A", "C", "B"}},
        {"E", {"B"}},
        {"F", {"B"}},
    };
    std::vector<DisasterTags::cityName> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    std::vector<DisasterTags::city> dlxItems = {
        //  0            1A          2B         3C          4D         5E          6F
        {0,0,0,0},   {2,18,13,0},{4,27,8,0},{2,21,15,0},{4,22,9,0},{2,25,10,0},{2,28,11,0},
        //  7                        8B                     9D         10E         11F
        {-2,0,11,0},             {2,2,14,0},            {4,4,16,0},{5,5,25,0}, {6,6,28,0},
        //  12          13A          14B        15C        16D
        {-4,8,16,0}, {1,1,18,0}, {2,8,24,0},{3,3,21,0}, {4,9,19,0},
        //  17          18A                                19D
        {-1,13,19,0},{1,13,1,0},                        {4,16,22,0},
        //  20                                  21C        22D
        {-3,18,22,0},                       {3,15,3,0}, {4,19,4,0},
        //  23                       24B                              25E
        {-5,21,25,0},            {2,14,27,0},                      {5,10,5,0},
        //  26                       27B                                           28F
        {-6,24,28,0},            {2,24,2,0},                                    {6,11,6,0},
        {INT_MIN,27,INT_MIN,0},
    };
    DisasterTags network (cities);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);

    std::string location = network.coverCity(13,1);
    EXPECT_EQUAL(location, "D");
    std::vector<DisasterTags::cityName> headersCoverD = {
        {"",  6, 5},
        {"A", 0, 2},
        {"B", 0, 3},
        {"C", 0, 4},
        {"D", 0, 5},
        {"E", 0, 6},
        {"F", 5, 0}
    };
    std::vector<DisasterTags::city> dlxCoverD = {
        //  0            1A          2B         3C          4D         5E          6F
        {0,0,0,0},   {2,18,13,1},{4,27,8,1},{2,21,15,1},{4,22,9,1},{2,25,10,0},{2,28,11,0},
        //  7                        8B                     9D         10E         11F
        {-2,0,11,0},             {2,2,14,0},            {4,4,16,0},{5,5,25,0}, {6,6,28,0},
        //  12          13A          14B        15C        16D
        {-4,8,16,0}, {1,1,18,1}, {2,8,24,1},{3,3,21,1}, {4,9,19,1},
        //  17          18A                                19D
        {-1,13,19,0},{1,13,1,0},                        {4,16,22,0},
        //  20                                  21C        22D
        {-3,18,22,0},                       {3,15,3,0}, {4,19,4,0},
        //  23                       24B                              25E
        {-5,21,25,0},            {2,14,27,0},                      {5,10,5,0},
        //  26                       27B                                           28F
        {-6,24,28,0},            {2,24,2,0},                                    {6,11,6,0},
        {INT_MIN,27,INT_MIN,0},
    };
    EXPECT_EQUAL(network.table_, headersCoverD);
    EXPECT_EQUAL(network.grid_, dlxCoverD);

    location = network.coverCity(10,2);
    EXPECT_EQUAL(location, "B");
    std::vector<DisasterTags::cityName> headersCoverB = {
        {"",  0, 0},
        {"A", 0, 2},
        {"B", 0, 3},
        {"C", 0, 4},
        {"D", 0, 5},
        {"E", 0, 6},
        {"F", 0, 0}
    };
    std::vector<DisasterTags::city> dlxCoverB = {
        //  0            1A          2B         3C          4D         5E          6F
        {0,0,0,0},   {2,18,13,1},{4,27,8,1},{2,21,15,1},{4,22,9,1},{2,25,10,2},{2,28,11,2},
        //  7                        8B                     9D         10E         11F
        {-2,0,11,0},             {2,2,14,2},            {4,4,16,2},{5,5,25,2}, {6,6,28,2},
        //  12          13A          14B        15C        16D
        {-4,8,16,0}, {1,1,18,1}, {2,8,24,1},{3,3,21,1}, {4,9,19,1},
        //  17          18A                                19D
        {-1,13,19,0},{1,13,1,0},                        {4,16,22,0},
        //  20                                  21C        22D
        {-3,18,22,0},                       {3,15,3,0}, {4,19,4,0},
        //  23                       24B                              25E
        {-5,21,25,0},            {2,14,27,0},                      {5,10,5,0},
        //  26                       27B                                           28F
        {-6,24,28,0},            {2,24,2,0},                                    {6,11,6,0},
        {INT_MIN,27,INT_MIN,0},
    };
    EXPECT_EQUAL(network.table_, headersCoverB);
    EXPECT_EQUAL(network.grid_, dlxCoverB);

    network.uncoverCity(10);
    EXPECT_EQUAL(network.table_, headersCoverD);
    EXPECT_EQUAL(network.grid_, dlxCoverD);
    network.uncoverCity(13);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}


/* * * * * * * * * * * * * * * * *      Full Coverage Tests             * * * * * * * * * * * * * */


STUDENT_TEST("Can cover this map with one supply.") {
    /*
     *
     *             B
     *             |
     *        A -- C
     *
     *
     *
     */
    const Map<std::string, Set<std::string>> cities = {
        {"A", {"C"}},
        {"B", {"C"}},
        {"C", {"A", "B"}},
    };
    std::vector<DisasterTags::cityName> networkHeaders = {
        {"",  3, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 0}
    };
    std::vector<DisasterTags::city> dlxItems = {
        // 0           1A        2B        3C
        {0,0,0,0},  {2,9,5,0},{2,12,6,0},{3,13,7,0},
        // 4           5A        6B        7C
        {-3,0,7,0}, {1,1,9,0},{2,2,12,0},{3,3,10,0},
        // 8           9A                 10C
        {-1,5,10,0},{1,5,1,0},           {3,7,13,0},
        // 11                    12A       13C
        {-2,9,13,0},          {2,6,2,0}, {3,10,3,0},
        {INT_MIN,12,INT_MIN,0},
    };
    Set<std::string> locations = {};

    DisasterTags network(cities);

    EXPECT(network.hasDisasterCoverage(1,locations));
    EXPECT_EQUAL(locations, {"C"});
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}

STUDENT_TEST("Can cover Ethene with two supplies.") {
    /*
     *
     *             C
     *             |
     *        A -- D -- B -- F
     *                  |
     *                  E
     *
     */
    const Map<std::string, Set<std::string>> cities = {
        {"A", {"D"}},
        {"B", {"D", "E", "F"}},
        {"C", {"D"}},
        {"D", {"A", "C", "B"}},
        {"E", {"B"}},
        {"F", {"B"}},
    };
    std::vector<DisasterTags::cityName> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    std::vector<DisasterTags::city> dlxItems = {
        //  0            1A          2B         3C          4D         5E          6F
        {0,0,0,0},   {2,18,13,0},{4,27,8,0},{2,21,15,0},{4,22,9,0},{2,25,10,0},{2,28,11,0},
        //  7                        8B                     9D         10E         11F
        {-2,0,11,0},             {2,2,14,0},            {4,4,16,0},{5,5,25,0}, {6,6,28,0},
        //  12          13A          14B        15C        16D
        {-4,8,16,0}, {1,1,18,0}, {2,8,24,0},{3,3,21,0}, {4,9,19,0},
        //  17          18A                                19D
        {-1,13,19,0},{1,13,1,0},                        {4,16,22,0},
        //  20                                  21C        22D
        {-3,18,22,0},                       {3,15,3,0}, {4,19,4,0},
        //  23                       24B                              25E
        {-5,21,25,0},            {2,14,27,0},                      {5,10,5,0},
        //  26                       27B                                           28F
        {-6,24,28,0},            {2,24,2,0},                                    {6,11,6,0},
        {INT_MIN,27,INT_MIN,0},
    };
    Set<std::string> locations = {};
    DisasterTags network (cities);
    EXPECT(network.hasDisasterCoverage(2,locations));
    EXPECT_EQUAL(locations, {"D","B"});
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}

STUDENT_TEST("Passes Straight Line test with two supplies.") {
    /*
     *
     *        F -- D -- B -- E -- C -- A
     *
     */
    const Map<std::string, Set<std::string>> cities = {
        {"A", {"C"}},
        {"B", {"D", "E"}},
        {"C", {"A", "E"}},
        {"D", {"B", "F"}},
        {"E", {"B", "C"}},
        {"F", {"D"}},
    };
    std::vector<DisasterTags::cityName> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    std::vector<DisasterTags::city> dlxItems = {
        //  0            1A          2B          3C         4D           5E          6F
        {0,0,0,0},   {2,24,12,0},{3,20,8,0}, {3,25,13,0},{3,27,9,0}, {3,22,10,0},{2,28,18,0},
        //  7                       8B                      9D          10E
        {-2,0,10,0},             {2,2,16,0},             {4,4,17,0}, {5,5,14,0},
        //  11          12A                     13C                     14E
        {-3,8,14,0}, {1,1,24,0},             {3,3,21,0},             {5,10,22,0},
        //  15                     16B                      17D                     18F
        {-4,12,18,0},            {2,8,20,0},             {4,9,27,0},             {6,6,28,0},
        //  19                     20B          21C                     22E
        {-5,16,22,0},            {2,16,2,0}, {3,13,25,0},            {5,14,5,0},
        //  23         24A                      25C
        {-1,20,25,0},{1,12,1,0},             {3,21,3,0},
        //  26                                              27D         28E
        {-6,24,28,0},                                    {4,17,4,0}, {6,18,6,0},
        {INT_MIN,27,INT_MIN,0},
    };
    Set<std::string> locations = {};
    DisasterTags network (cities);
    EXPECT(network.hasDisasterCoverage(2,locations));
    EXPECT_EQUAL(locations, {"D","C"});
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}

STUDENT_TEST("Do not be greedy with our algorithm in terms of most connected cities.") {
    /*
     *
     *     A       E
     *     |       |
     *     B - D - F
     *      \ / \ /
     *       C   G
     *
     */

    const Map<std::string, Set<std::string>> cities = {
        {"A", {"B"}},
        {"B", {"A", "C", "D"}},
        {"C", {"B", "D"}},
        {"D", {"B", "C", "F", "G"}},
        {"E", {"F"}},
        {"F", {"D", "E", "G"}},
        {"G", {"D", "F"}},
    };
    DisasterTags network(cities);
    Set<std::string> chosen = {};
    EXPECT(!network.hasDisasterCoverage(1, chosen));
    chosen.clear();
    EXPECT(network.hasDisasterCoverage(2, chosen));
    EXPECT_EQUAL(chosen, {"B","F"});
}

STUDENT_TEST("Supply an island city when we must.") {
    /*
     *
     *     A       E
     *     |
     *     B
     *      \
     *       C-D
     *
     */

    const Map<std::string, Set<std::string>> cities = {
        {"A", {"B"}},
        {"B", {"A", "C"}},
        {"C", {"B", "D"}},
        {"D", {"C"}},
        {"E", {}}
    };
    DisasterTags network(cities);
    Set<std::string> chosen = {};
    EXPECT(!network.hasDisasterCoverage(1, chosen));
    EXPECT(!network.hasDisasterCoverage(2, chosen));
    EXPECT(network.hasDisasterCoverage(3, chosen));
}

STUDENT_TEST("Supply 5 island cities when we must.") {
    /*
     *
     *     A            E
     *
     *            B
     *
     *      C              D
     *
     */

    const Map<std::string, Set<std::string>> cities = {
        {"A", {}},
        {"B", {}},
        {"C", {}},
        {"D", {}},
        {"E", {}}
    };
    DisasterTags network(cities);
    Set<std::string> chosen = {};
    EXPECT(!network.hasDisasterCoverage(1, chosen));
    EXPECT(!network.hasDisasterCoverage(2, chosen));
    EXPECT(!network.hasDisasterCoverage(3, chosen));
    EXPECT(!network.hasDisasterCoverage(4, chosen));
    EXPECT(network.hasDisasterCoverage(5, chosen));
}

PROVIDED_TEST("Can solve ethene example, regardless of ordering, with output.") {
    /* Because Map and Set internally store items in sorted order, the order
     * in which you iterate over the cities when making decisions is sensitive
     * to the order of those cities' names. This test looks at a map like
     * this one, trying out all possible orderings of the city names:
     *
     *             *
     *             |
     *        * -- * -- * -- *
     *                  |
     *                  *
     *
     * There are 6! = 720 possible permutations of the ordering of these six
     * cities. If your code is able to solve the problem correctly for all of
     * those orderings, there's a good chance that you're correctly
     * covering and uncovering cities at each step. On the other hand, if
     * your code runs into issues here, it may indicate that the way in which
     * you're covering and uncovering cities accidentally uncovers a city that
     * you have previously covered.
     */
    Vector<std::string> cities = { "A", "B", "C", "D", "E", "F" };
    do {
        /* Ethene arrangement:
         *
         *        0
         *       1234
         *         5
         */
        Map<std::string, Set<std::string>> map = makeMap({
            { cities[2], { cities[0], cities[1], cities[3] } },
            { cities[3], { cities[4], cities[5] } }
        });

        DisasterTags network(map);
        Set<std::string> chosen = {};
        EXPECT(network.hasDisasterCoverage(2, chosen));

        EXPECT_EQUAL(chosen.size(), 2);
        EXPECT(chosen.contains(cities[2]));
        EXPECT(chosen.contains(cities[3]));

        chosen.clear();

        EXPECT(!network.hasDisasterCoverage(1, chosen));
    } while (next_permutation(cities.begin(), cities.end()));
}

PROVIDED_TEST("Works for six cities in a line, regardless of order with output.") {
    /* Because Map and Set internally store items in sorted order, the order
     * in which you iterate over the cities when making decisions is sensitive
     * to the order of those cities' names. This test looks at a map like
     * this one, trying out all possible orderings of the city names:
     *
     *        * -- * -- * -- * -- * -- *
     *
     * There are 6! = 720 possible permutations of the ordering of these six
     * cities in a line. If your code is able to solve the problem correctly
     * for all of those orderings, there's a good chance that you're correctly
     * covering and uncovering cities at each step. On the other hand, if
     * your code runs into issues here, it may indicate that the way in which
     * you're covering and uncovering cities accidentally uncovers a city that
     * you have previously covered.
     */
    Vector<std::string> cities = { "A", "B", "C", "D", "E", "F" };
    do {
        /* Linear arrangement. */
        Map<std::string, Set<std::string>> map;
        for (int i = 1; i + 1 < cities.size(); i++) {
            map[cities[i]] = { cities[i - 1], cities[i + 1] };
        }

        Set<std::string> chosen = {};
        map = makeMap(map);

        DisasterTags network(map);
        EXPECT(network.hasDisasterCoverage(2, chosen));

        EXPECT_EQUAL(chosen.size(), 2);
        EXPECT(chosen.contains(cities[1]));
        EXPECT(chosen.contains(cities[4]));

        chosen.clear();

        EXPECT(!network.hasDisasterCoverage(1, chosen));
    } while (next_permutation(cities.begin(), cities.end()));
}

PROVIDED_TEST("Solves \"Don't be Greedy,\" regardless of ordering with output.") {
    /* Because Map and Set internally store items in sorted order, the order
     * in which you iterate over the cities when making decisions is sensitive
     * to the order of those cities' names. This test looks at a map like
     * this one, trying out all possible orderings of the city names:
     *
     *     0       4
     *     |       |
     *     1 - 2 - 3
     *      \ / \ /
     *       5   6
     *
     * There are 7! = 5,040 possible permutations of the ordering of these seven
     * cities. If your code is able to solve the problem correctly for all of
     * those orderings, there's a good chance that you're correctly
     * covering and uncovering cities at each step. On the other hand, if
     * your code runs into issues here, it may indicate that the way in which
     * you're covering and uncovering cities accidentally uncovers a city that
     * you have previously covered.
     */
    Vector<std::string> cities = { "A", "B", "C", "D", "E", "F", "G" };
    do {
        Map<std::string, Set<std::string>> map = makeMap({
            { cities[1], { cities[0], cities[2], cities[5] } },
            { cities[2], { cities[3], cities[5], cities[6] } },
            { cities[3], { cities[4], cities[6] } },
        });

        Set<std::string> chosen = {};

        DisasterTags network(map);
        /* We should be able to cover everything with two cities. */
        EXPECT(network.hasDisasterCoverage(2, chosen));

        /* Those cities should be 1 and 3. */
        EXPECT_EQUAL(chosen, { cities[1], cities[3] });

        chosen.clear();

        EXPECT(!network.hasDisasterCoverage(1, chosen));
    } while (next_permutation(cities.begin(), cities.end()));
}

STUDENT_TEST("We should still be quick if we start by focussing on most isolated so far.") {
    /*
     *
     * 0 is extremely well connected but will play no role in final solution to supply all cities.
     * Every surrounding city in the inner ring must receive supplies.
     *
     *             32   17   18
     *          31   \  |  /   19
     *            \  16 1 2   /
     *             15 | | | 3
     *     30 --14--| | | | |--4--- 20
     *     29 --13--|-|-0-|-|--5--- 21
     *     28 --12--| | | | |--6--- 22
     *             11 | | |-7
     *            /  10 9 8  \
     *          27   /  |  \  23
     *              26  25  24
     *
     */
    Vector<std::string> cities = {
        "A", "B", "C", "D", "E", "F", "G", "H",
        "I", "J", "K", "L", "M", "N", "O", "P",
        "Q", "R", "S", "T", "U", "V", "W", "X",
        "Y", "Z", "?", "@", "#", "$", "%", ")","*",
    };
    Map<std::string, Set<std::string>> map = makeMap({
        { cities[0], { cities[1], cities[2], cities[3], cities[4], cities[5], cities[6],
                       cities[7],cities[8],cities[9],cities[10],cities[11],cities[12],
                       cities[13],cities[14],cities[15],cities[16],} },
        { cities[17], { cities[1]} },
        { cities[18], { cities[2]} },
        { cities[19], { cities[3]} },
        { cities[20], { cities[4]} },
        { cities[21], { cities[5]} },
        { cities[22], { cities[6]} },
        { cities[23], { cities[7]} },
        { cities[24], { cities[8]} },
        { cities[25], { cities[9]} },
        { cities[26], { cities[10]} },
        { cities[27], { cities[11]} },
        { cities[28], { cities[12]} },
        { cities[29], { cities[13]} },
        { cities[30], { cities[14]} },
        { cities[31], { cities[15]} },
        { cities[32], { cities[16]} },
    });

    Set<std::string> chosen = {};

    DisasterTags network(map);
    /* We should be able to cover everything with two cities. */
    EXPECT(network.hasDisasterCoverage(16, chosen));

    /* Those cities should be 1 and 3. */
    EXPECT_EQUAL(chosen, { cities[1], cities[2], cities[3], cities[4], cities[5], cities[6],
                           cities[7],cities[8],cities[9],cities[10],cities[11],cities[12],
                           cities[13],cities[14],cities[15],cities[16], });

    chosen.clear();

    EXPECT(!network.hasDisasterCoverage(15, chosen));
}

PROVIDED_TEST("Stress test: 6 x 6 grid. (This should take at most a few seconds.)") {
    Map<std::string, Set<std::string>> grid;

    /* Build the grid. */
    char maxRow = 'F';
    int  maxCol = 6;
    for (char row = 'A'; row <= maxRow; row++) {
        for (int col = 1; col <= maxCol; col++) {
            if (row != maxRow) {
                grid[row + std::to_string(col)] += (char(row + 1) + std::to_string(col));
            }
            if (col != maxCol) {
                grid[row + std::to_string(col)] += (char(row) + std::to_string(col + 1));
            }
        }
    }
    grid = makeMap(grid);

    DisasterTags network(grid);
    Set<std::string> locations;
    EXPECT(network.hasDisasterCoverage(10, locations));
}

PROVIDED_TEST("Stress test: 6 x 6 grid, with output. (This should take at most a few seconds.)") {
    Map<std::string, Set<std::string>> grid;

    /* Build the grid. */
    char maxRow = 'F';
    int  maxCol = 6;
    for (char row = 'A'; row <= maxRow; row++) {
        for (int col = 1; col <= maxCol; col++) {
            if (row != maxRow) {
                grid[row + std::to_string(col)] += (char(row + 1) + std::to_string(col));
            }
            if (col != maxCol) {
                grid[row + std::to_string(col)] += (char(row) + std::to_string(col + 1));
            }
        }
    }
    grid = makeMap(grid);

    DisasterTags network(grid);
    Set<std::string> locations;
    EXPECT(network.hasDisasterCoverage(10, locations));

    for (char row = 'A'; row <= maxRow; row++) {
        for (int col = 1; col <= maxCol; col++) {
            EXPECT(checkCovered(row + std::to_string(col), grid, locations));
        }
    }
}

PROVIDED_TEST("Stress test: 8 x 8 grid, with output. (This should take at most a few seconds.)") {
    Map<std::string, Set<std::string>> grid;

    /* Build the grid. */
    char maxRow = 'G';
    int  maxCol = 8;
    for (char row = 'A'; row <= maxRow; row++) {
        for (int col = 1; col <= maxCol; col++) {
            if (row != maxRow) {
                grid[row + std::to_string(col)] += (char(row + 1) + std::to_string(col));
            }
            if (col != maxCol) {
                grid[row + std::to_string(col)] += (char(row) + std::to_string(col + 1));
            }
        }
    }
    grid = makeMap(grid);

    DisasterTags network(grid);
    Set<std::string> locations;
    EXPECT(network.hasDisasterCoverage(14, locations));

    for (char row = 'A'; row <= maxRow; row++) {
        for (int col = 1; col <= maxCol; col++) {
            EXPECT(checkCovered(row + std::to_string(col), grid, locations));
        }
    }
}

STUDENT_TEST("All possible configurations of a square.") {
    /*
     *
     *        A----B
     *        |    |
     *        C----D
     *
     *
     *
     */
    const Map<std::string, Set<std::string>> cities = {
        {"A", {"B","C"}},
        {"B", {"A","D"}},
        {"C", {"A","D"}},
        {"D", {"B","C"}},
    };
    Set<Set<std::string>> allConfigs = {
        {"A","C"},
        {"A","B"},
        {"A","D"},
        {"B","C"},
        {"B","D"},
        {"C","D"},
    };
    DisasterTags grid(cities);
    Set<Set<std::string>> allFound = grid.getAllDisasterConfigurations(2);
    EXPECT_EQUAL(allFound,allConfigs);
}

STUDENT_TEST("All possible configurations with 4 supplies are many.") {
    /*
     *
     *   H--A     E--I
     *      |     |
     *      B--D--F
     *      |     |
     *      C     G
     *
     */
    const Map<std::string, Set<std::string>> cities = {
        {"A", {"H","B"}},
        {"B", {"A","C","D"}},
        {"C", {"B"}},
        {"D", {"B","F"}},
        {"E", {"F","I"}},
        {"F", {"D","E","G"}},
        {"G", {"F"}},
        {"H", {"A"}},
        {"I", {"E"}},
    };
    DisasterTags grid(cities);
    Set<Set<std::string>> allFound = grid.getAllDisasterConfigurations(4);
    Set<Set<std::string>> allConfigs = {
        {"A", "B", "E", "F"},
        {"A", "B", "E", "G"},
        {"A", "B", "F", "I"},
        {"A", "B", "G", "I"},
        {"A", "C", "E", "F"},
        {"A", "C", "F", "I"},
        {"B", "E", "F", "H"},
        {"B", "E", "G", "H"},
        {"B", "F", "H", "I"},
        {"B", "G", "H", "I"},
        {"C", "E", "F", "H"},
        {"C", "F", "H", "I"}
    };
    EXPECT_EQUAL(allFound,allConfigs);
}

STUDENT_TEST("Larger maps are more difficult to filter out duplicates.") {
    /*
     * This is a good test for when I am trying to avoid generating duplicates. Put a number
     * counter in the getAllDisasterConfigurations function and have it count every time a
     * set is generated. You will see that we generate extra sets and the containing set filters
     * them for us. I cannot yet figure out how to only generate unique sets.
     *
     *                                    /--S-----------
     *                         T --------   /            \
     *                          \     ----R               \
     *                           \   /    |                \
     *                            -U      Q       L-- K--- J
     *                            /      /      /  \   |   |
     *                 A         /      P      O   N   M---I
     *                  \       /       |     |    |   |  /
     *                   -----B----C -- E-----F----G---H--
     *                             \
     *                              D
     */
    const Map<std::string, Set<std::string>> cities = {
        {"A", {"B"}},
        {"B", {"A","C","U"}},
        {"C", {"B","D","E"}},
        {"D", {"C"}},
        {"E", {"C","P","F"}},
        {"F", {"E","O","G"}},
        {"G", {"F","H","N"}},
        {"H", {"G","I","M"}},
        {"I", {"H","J","M"}},
        {"J", {"I","K","M"}},
        {"K", {"J","L","M"}},
        {"L", {"K","N","O"}},
        {"M", {"H","I","K"}},
        {"N", {"G","L"}},
        {"O", {"F","L"}},
        {"P", {"E","Q"}},
        {"Q", {"P","R"}},
        {"R", {"S","Q","U"}},
        {"S", {"J","R","T"}},
        {"T", {"S","U"}},
        {"U", {"B","R","T"}},
    };
    DisasterTags grid(cities);
    Set<Set<std::string>> allFound = grid.getAllDisasterConfigurations(7);
    Set<Set<std::string>> allConfigs = {
        {"A", "C", "E", "H", "L", "R", "S"}, {"A", "C", "F", "I", "L", "Q", "T"},
        {"A", "C", "G", "I", "L", "Q", "T"}, {"A", "C", "G", "J", "L", "Q", "T"},
        {"A", "C", "G", "J", "O", "Q", "T"}, {"A", "D", "E", "H", "L", "R", "S"},
        {"A", "D", "F", "I", "L", "Q", "T"}, {"B", "C", "E", "H", "L", "P", "S"},
        {"B", "C", "E", "H", "L", "Q", "S"}, {"B", "C", "E", "H", "L", "R", "S"},
        {"B", "C", "F", "H", "L", "P", "S"}, {"B", "C", "F", "H", "L", "Q", "S"},
        {"B", "C", "F", "I", "L", "P", "S"}, {"B", "C", "F", "I", "L", "Q", "S"},
        {"B", "C", "F", "I", "L", "Q", "T"}, {"B", "C", "F", "L", "M", "P", "S"},
        {"B", "C", "F", "L", "M", "Q", "S"}, {"B", "C", "F", "M", "N", "P", "S"},
        {"B", "C", "F", "M", "N", "Q", "S"}, {"B", "C", "G", "H", "L", "P", "S"},
        {"B", "C", "G", "H", "L", "Q", "S"}, {"B", "C", "G", "I", "L", "P", "S"},
        {"B", "C", "G", "I", "L", "Q", "S"}, {"B", "C", "G", "I", "L", "Q", "T"},
        {"B", "C", "G", "J", "L", "P", "S"}, {"B", "C", "G", "J", "L", "Q", "S"},
        {"B", "C", "G", "J", "L", "Q", "T"}, {"B", "C", "G", "J", "O", "P", "S"},
        {"B", "C", "G", "J", "O", "Q", "S"}, {"B", "C", "G", "J", "O", "Q", "T"},
        {"B", "C", "G", "L", "M", "P", "S"}, {"B", "C", "G", "L", "M", "Q", "S"},
        {"B", "C", "G", "M", "O", "P", "S"}, {"B", "C", "G", "M", "O", "Q", "S"},
        {"B", "C", "H", "L", "O", "P", "S"}, {"B", "C", "H", "L", "O", "Q", "S"},
        {"B", "C", "M", "N", "O", "P", "S"}, {"B", "C", "M", "N", "O", "Q", "S"},
        {"B", "D", "E", "H", "L", "P", "S"}, {"B", "D", "E", "H", "L", "Q", "S"},
        {"B", "D", "E", "H", "L", "R", "S"}, {"B", "D", "F", "H", "L", "P", "S"},
        {"B", "D", "F", "H", "L", "Q", "S"}, {"B", "D", "F", "I", "L", "P", "S"},
        {"B", "D", "F", "I", "L", "Q", "S"}, {"B", "D", "F", "I", "L", "Q", "T"},
        {"B", "D", "F", "L", "M", "P", "S"}, {"B", "D", "F", "L", "M", "Q", "S"},
        {"B", "D", "F", "M", "N", "P", "S"}, {"B", "D", "F", "M", "N", "Q", "S"},
        {"B", "D", "G", "H", "L", "P", "S"}, {"B", "D", "G", "I", "L", "P", "S"},
        {"B", "D", "G", "J", "L", "P", "S"}, {"B", "D", "G", "J", "O", "P", "S"},
        {"B", "D", "G", "L", "M", "P", "S"}, {"B", "D", "G", "M", "O", "P", "S"},
        {"B", "D", "H", "L", "O", "P", "S"}, {"B", "D", "M", "N", "O", "P", "S"}
    };
    EXPECT_EQUAL(allFound,allConfigs);
}
