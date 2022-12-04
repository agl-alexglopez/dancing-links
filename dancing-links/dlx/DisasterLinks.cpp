/**
 * Author: Alexander G. Lopez
 * File: DisasterLinks.cpp
 * --------------------------
 * This file contains the implementation of Algorithm X via Dancing Links as applied to a Disaster
 * Planning problem. For more details on the thought process behind the implmenetation, please see
 * the .h file or the readme. Comments are detailed for my own understanding and because some of
 * the implementation is complicated in this context.
 */
#include "DisasterLinks.h"


/* * * * * * * * * * * * *  Algorithm X via Dancing Links Implementation  * * * * * * * * * * * * */


/**
 * @brief isDisasterReady  performs a recursive search to determine if a transportation grid
 *                         can be covered with the specified number of emergency supplies. It
 *                         will also place the found cities in the output parameter if there
 *                         exists a solution. A city is covered or safe it has supplies or
 *                         is adjacent to a city with supplies. The solution may not use all
 *                         of the provided supplies.
 * @param numSupplies      the limiting number of supplies we must distribute.
 * @param suppliedCities   the output parameter telling which cities received supplies.
 * @return                 true if we have found a viable supply scheme, false if not.
 */
bool DisasterLinks::isDisasterReady(int numSupplies, Set<std::string>& suppliedCities) {
    if (numSupplies < 0) {
        error("Negative supply quantity is impossible.");
    }
    if (dlx.numItemsAndOptions == 0) {
        return true;
    }
    return isCovered(numSupplies, suppliedCities);
}

/**
 * @brief isCovered       performs a modified exact cover search of a transportation Network.
 *                        Given a number of supplies for the network, determines if every city
 *                        is covered. A city is covered if it has received supplies or is
 *                        adjacent to a city with supplies. If a city has supplies it may not
 *                        be supplied again. However, if a city is safely covered by an adjacent
 *                        city with supplies, this safe city may still receive supplies to cover
 *                        other cities. With infinite supplies this algorithm will find a good
 *                        solution. For the optimal solution challenge it with decreasing supply
 *                        counts until it confirms a network cannot be covered with that amount.
 * @param numSupplies     the number of supplies that we have to distribute over the network.
 * @param suppliedCities  the output parameter showing the cities we have chosen to supply.
 * @return                true if all cities are safe, false if not.
 */
bool DisasterLinks::isCovered(int numSupplies, Set<std::string>& suppliedCities) {
    if (dlx.lookupTable[0].right == 0 && numSupplies >= 0) {
        return true;
    }
    if (numSupplies <= 0) {
        return false;
    }

    // Choose the city that appears the least across all sets because that will be hard to cover.
    int chosenIndex = chooseOption();

    /* Try to cover this city by first supplying the most connected city nearby. Try cities with
     * successively fewer connections then if all else fails supply the isolated city itself.
     */
    for (int cur = chosenIndex;
         dlx.grid[cur].down != chosenIndex;
         cur = dlx.grid[cur].down) {

        /* We will try the supply options as specified above to cover the city in question. This is
         * critical for the output parameter, the set of cities we covered in a successful solution.
         */
        std::string supplyLocation = coverCity(cur);
        suppliedCities.add(supplyLocation);

        if (isCovered(numSupplies - 1, suppliedCities)) {
            // This algorithm may run on successively decreasing supplies so we must cleanup.
            uncoverCity(cur);
            return true;
        }

        // This cleanup is in case of failed choices. Try another starting supply location.
        uncoverCity(cur);
        suppliedCities.remove(supplyLocation);
    }

    return false;
}

/**
 * @brief chooseOption  selects a city we are trying to cover either by giving it supplies or
 *                      covering an adjacent neighbor. The selection uses the following
 *                      heuristic:
 *                          - Select the most isolated city so far.
 *                          - We must cover this city so select an adjacent city with the
 *                            most connections and try that first.
 *                          - If that fails we try the next adjacent city with most connections.
 *                          - Finally, if all other neighbors fail, try to supply the actual city 
 *                            in question, not neighbors.
 * @return              the index of the city we are selecting to attempt to cover.
 */
int DisasterLinks::chooseOption() {
    int min = INT_MAX;
    int chosenIndex = 0;
    int head = 0;
    for (int cur = dlx.lookupTable[0].right; cur != head; cur = dlx.lookupTable[cur].right) {
        if (dlx.grid[cur].topOrLen < min) {
            chosenIndex = cur;
            min = dlx.grid[cur].topOrLen;
        }
    }
    return chosenIndex;
}

/**
 * @brief coverCity  covers a city with the option below the specified index. A city in question
 *                   may be covered by supplying a neighbor or supplying the city itself.
 * @param index      the index we start at for an item. We select the option beneath this index.
 * @return           the string name of the option we used to cover a city, neighbor or city.
 */
std::string DisasterLinks::coverCity(int index) {
    int startIndex = dlx.grid[index].down;

    // Hide the column of the city we want to cover first. It will be last one we uncover.
    hideItemCol(dlx.grid[startIndex]);

    DisasterLinks::cityItem start = dlx.grid[startIndex];
    DisasterLinks::cityItem cur = dlx.grid[start.right];


    /* Be sure to leave the row of the option we supply unchanged. Splice these cities out of all
     * other options in which they can be found above and below the current row.
     */

    std::string result = "";
    while (cur != start) {
        int top = cur.topOrLen;

        if (top <= 0) {
            /* We are always garunteed to pass the spacer tile so we will collect the name of the
             * city we have chosen to supply to prove our algorithm chose correctly.
            */
            result = dlx.lookupTable[std::abs(top)].name;
        } else {
            hideItemCol(cur);
            dlx.lookupTable[dlx.lookupTable[top].left].right = dlx.lookupTable[top].right;
            dlx.lookupTable[dlx.lookupTable[top].right].left = dlx.lookupTable[top].left;
        }
        cur = dlx.grid[cur.right];
    }

    dlx.lookupTable[dlx.lookupTable[cur.topOrLen].left].right = dlx.lookupTable[cur.topOrLen].right;
    dlx.lookupTable[dlx.lookupTable[cur.topOrLen].right].left = dlx.lookupTable[cur.topOrLen].left;

    return result;
}

/**
 * @brief uncoverCity  uncovers a city if that choice of option did not lead to a covered
 *                     network. Uncovers the same option that was selected for coverage if given
 *                     the same index.
 * @param index        the index of the item we covered with the option below the index.
 */
void DisasterLinks::uncoverCity(int index) {

    DisasterLinks::cityItem& start = dlx.grid[dlx.grid[index].down];
    DisasterLinks::cityItem cur = dlx.grid[start.left];
    dlx.lookupTable[dlx.lookupTable[start.topOrLen].left].right = start.topOrLen;
    dlx.lookupTable[dlx.lookupTable[start.topOrLen].right].left = start.topOrLen;

    /* To uncover a city we take the supplies away from the option in which we found this city. We
     * then must go up and down for every city covered by this supply location and put the cities
     * back in all the other sets. Original row was not altered so no other restoration necessary.
     */

    while (cur != start) {
        int top = cur.topOrLen;

        if (top > 0) {
            dlx.lookupTable[dlx.lookupTable[top].left].right = top;
            dlx.lookupTable[dlx.lookupTable[top].right].left = top;
            DisasterLinks::unhideItemCol(cur, index);
        }
        index = cur.left;
        cur = dlx.grid[index];
    }
    // Unhide the city's column last because it was the first one we hid.
    DisasterLinks::unhideItemCol(start, index);
}

/**
 * @brief hideItemCol  when we supply an option it covers itself and connected cities. We must
 *                     remove these cities from any other sets that contain them to make them
 *                     disappear from the world as uncovered cities. However, we keep them as
 *                     available cities to supply.
 * @param start        the city we start at in a row. We traverse downward to snip city out.
 */
void DisasterLinks::hideItemCol(DisasterLinks::cityItem& start) {
    DisasterLinks::cityItem cur = start;
    while ((cur = dlx.grid[cur.down]) != start) {
        dlx.grid[cur.right].left = cur.left;
        dlx.grid[cur.left].right = cur.right;
   }
}

/**
 * @brief unhideItemCol  when an option fails, we must put the cities it covers back into all
 *                       the sets to which they belong. This puts the cities back in network.
 * @param start          the city in the option we start at. We traverse upward to unhide.
 * @param index          the index we need cities to point to in order to restore network.
 */
void DisasterLinks::unhideItemCol(DisasterLinks::cityItem& start, int index) {
    DisasterLinks::cityItem cur = start;
    index = cur.up;
    while ((cur = dlx.grid[index]) != start) {
        dlx.grid[cur.right].left = index;
        dlx.grid[cur.left].right = index;
        index = cur.up;
    }
}


/* * * * * * * * * * *  Constructor and Building of Dancing Links Network   * * * * * * * * * * * */


/**
 * @brief DisasterLinks  a custom constructor for this class that can turn a Map representation
 *                       of a transportation grid into a vector grid prepared for exact cover
 *                       search via dancing links.
 * @param roadNetwork    the transportation grid passed in via map form.
 */
DisasterLinks::DisasterLinks(const Map<std::string, Set<std::string>>& roadNetwork) {
    dlx.numItemsAndOptions = 0;
    // We will set this up for a reverse build of column links for a given item.
    HashMap<std::string,int> columnBuilder = {};
    std::vector<std::pair<std::string,int>> connectionSizes = {};

    // We need to start preparing items in the grid immediately after the headers.
    initializeHeaders(roadNetwork, connectionSizes, columnBuilder);

    /* The second pass will fill in the columns and keep the headers and all elements appropriately
     * updated. We can hang on to helpful index info. Important to keep this O(m + n).
     */
    initializeItems(roadNetwork, connectionSizes, columnBuilder);
}

/**
 * @brief initializeItems  builds the structure needed to perform the dancing links algorithm.
 *                         This focusses on setting up the grid in the dancing Network struct
 *                         so that all item columns are tallied correctly and the option rows
 *                         represent all the cities that a supply city can cover, self included.
 * @param roadNetwork      we need to look back at the original map to grab sets to build.
 * @param connectionSizes  we organize rows in descending order top to bottom as a heuristic.
 * @param columnBuilder    the map we use to help build an accurate column for each city item.
 * @param index            we are passed in a starting index to begin building
 */
void DisasterLinks::initializeItems(const Map<std::string, Set<std::string>>& roadNetwork,
                                    const std::vector<std::pair<std::string,int>>& connectionSizes,
                                    HashMap<std::string,int>& columnBuilder) {
    int previousSetSize = dlx.grid.size();
    int index = dlx.grid.size();

    for (const auto& [city, connectionSize] : connectionSizes) {
        // This algorithm includes a city in its own set of connections.
        Set<std::string> connections = roadNetwork[city] + city;

        /* We will know which supplying city option an item is in by the spacerTitle.
         * lookupTable[abs(-hederIndexMap[city])] will give us the name of the option of that row.
         * That is the city we are supplying and the connections it covers, self included.
         */
        dlx.grid.add({-dlx.headerIndexMap[city],     // Negative index of city as option.
                      index - previousSetSize,       // First item in previous option
                      index + connections.size(),    // Last item in current option
                      index,
                      index + 1});

        // Manage column pointers for items connected across options. Update index.
        index = initializeColumns(connections, columnBuilder, index);

        previousSetSize = connections.size();
    }
    dlx.grid.add({NOT_PROCESSED, index - previousSetSize, 0, index - 1, NOT_PROCESSED});
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
void DisasterLinks::initializeHeaders(const Map<std::string, Set<std::string>>& roadNetwork,
                                      std::vector<std::pair<std::string,int>>& connectionSizes,
                                      HashMap<std::string,int>& columnBuilder) {
    dlx.lookupTable.add({"", 0, 1});
    dlx.grid.add({0,0,0,0,1});
    int index = 1;
    // The first pass will set up the name headers and the column headers in the two vectors.
    for (const std::string& city : roadNetwork) {

        // Add one to the connection size because we will add a city to its own connections later.
        connectionSizes.push_back({city, roadNetwork[city].size() + 1});

        // We need to set up multiple columns, so begin tracking the previous item for a column.
        columnBuilder[city] = index;

        dlx.lookupTable.add({city, index - 1, index + 1});
        dlx.lookupTable[0].left++;
        dlx.grid[0].left++;
        // Add the first headers for the item vector. They need count up and down.
        dlx.grid.add({0, index, index, index - 1, index + 1});
        dlx.headerIndexMap[city] = index;
        dlx.numItemsAndOptions++;
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

    dlx.lookupTable[dlx.lookupTable.size() - 1].right = 0;
    dlx.grid[dlx.grid.size() - 1].right = 0;
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
int DisasterLinks::initializeColumns(const Set<std::string>& connections,
                                     HashMap<std::string,int>& columnBuilder,
                                     int index) {
    int spacerIndex = index;
    for (const auto& c : connections) {
        dlx.grid[dlx.headerIndexMap[c]].topOrLen++;
        index++;

        // A single item in a circular doubly linked list points to itself.
        dlx.grid.add({dlx.headerIndexMap[c], index, index, index - 1, index + 1});

        /* Now we need to handle building the up and down pointers for a column of items.
         * We also must make sure to keep the most recent element pointing down to the
         * first of a column because this is circular. The first elem in the column must
         * also point up to the most recently added element because this is circular.
         */

        // This is the necessary adjustment to the column header's up field for a given item.
        dlx.grid[dlx.grid[columnBuilder[c]].down].up = index;

        // The current node is now the new tail in a vertical circular linked list for an item.
        dlx.grid[index].up = columnBuilder[c];
        dlx.grid[index].down = dlx.grid[columnBuilder[c]].down;

        // Update the old tail to reflect the new addition of an item in its option.
        dlx.grid[columnBuilder[c]].down = index;

        // Similar to a previous/current coding pattern but in an above/below column.
        columnBuilder[c] = index;
    }
    /* Every option "row" is a left-right circular linked list. This is how we recursively cover
     * Cities by removing them as items only. A city that is adjacent to a supplied city may still
     * receive supplies to cover other cities as an option. This is how we achieve that. This is
     * a significant variation from Knuth's DLX.
     */
    dlx.grid[index].right = spacerIndex;
    dlx.grid[spacerIndex].left = index;
    return ++index;
}


/* * * * * * * * * * * * * * * *   Overloaded Operators for Debugging   * * * * * * * * * * * * * */


bool operator==(const DisasterLinks::cityItem& lhs, const DisasterLinks::cityItem& rhs) {
    return lhs.topOrLen == rhs.topOrLen
            && lhs.up == rhs.up && lhs.down == rhs.down
               && lhs.left == rhs.left && lhs.right == rhs.right;
}

bool operator!=(const DisasterLinks::cityItem& lhs, const DisasterLinks::cityItem& rhs) {
    return !(lhs == rhs);
}

bool operator==(const DisasterLinks::cityHeader& lhs, const DisasterLinks::cityHeader& rhs) {
    return lhs.name == rhs.name && lhs.left == rhs.left && lhs.right == rhs.right;
}

bool operator!=(const DisasterLinks::cityHeader& lhs, const DisasterLinks::cityHeader& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, const DisasterLinks::cityItem& city) {
    os << "{ topOrLen: " << city.topOrLen << ", up: " << city.up << ", down: " << city.down
       << ", left: " << city.left << ", right: " << city.right << " }";
    return os;
}

std::ostream& operator<<(std::ostream& os, const DisasterLinks::cityHeader& city) {
    os << "{ name: " << city.name << ", left: " << city.left << ", right: " << city.right << " }";
    return os;
}

std::ostream& operator<<(std::ostream&os, const Vector<DisasterLinks::cityItem>& grid) {
    for (const auto& item : grid) {
        if (item.topOrLen <= 0) {
            os << std::endl;
        }
        os << "{" << item.topOrLen << "," << item.up << "," << item.down << ","
           << item.left << "," << item.right << "}, ";
    }
    os << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream&os, const DisasterLinks& network) {
    for (const auto& header : network.dlx.lookupTable) {
        os << "{" << header.name << "," << header.left << "," << header.right << "},";
    }
    os << std::endl;
    for (const auto& item : network.dlx.grid) {
        if (item.topOrLen <= 0) {
            os << std::endl;
        }
        os << "{" << item.topOrLen << "," << item.up << "," << item.down << ","
           << item.left << "," << item.right << "}, ";
    }
    os << std::endl;
    return os;
}


/* * * * * * * * * * * * * * * * *     Test Cases Below This Point      * * * * * * * * * * * * * */


#include "GUI/SimpleTest.h"


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
    Vector<DisasterLinks::cityHeader> networkHeaders = {
        {"",  3, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 0}
    };
    Vector<DisasterLinks::cityItem> dlxItems = {
        //        0               1A             2B            3C
              {0,0,0,3,1},    {2,9,5,0,2}, {2,12,6,1,3},  {3,13,7,2,0},
        //       4A               5A             6B            7C
        /*C*/ {-3,0,7,7,5},   {1,1,9,4,6}, {2,2,12,5,7},  {3,3,10,6,4},
        //       8B               9A                           10C
        /*A*/ {-1,5,10,10,9}, {1,5,1,8,10},               {3,7,13,9,8},
        //       11C                             12B           13C
        /*B*/ {-2,9,13,13,12},             {2,6,2,11,13}, {3,10,3,12,11},
              {NOT_PROCESSED,12,0,13,NOT_PROCESSED},
    };
    DisasterLinks network(cities);

    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], networkHeaders[i]);
    }

    for (int i = 0; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxItems[i]);
    }
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
    Vector<DisasterLinks::cityHeader> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    Vector<DisasterLinks::cityItem> dlxItems = {
        //        0                 1A             2B               3C               4D              5E             6F
              {0,0,0,6,1},     {2,24,12,0,2},   {3,20,8,1,3},   {3,25,13,2,4},   {3,27,9,3,5},   {3,22,10,4,6},   {2,28,18,5,0},
        /*B*/ {-2,0,10,10,8},                   {2,2,16,7,9},                    {4,4,17,8,10},  {5,5,14,9,7},
        /*C*/ {-3,8,14,14,12},  {1,1,24,11,13},                 {3,3,21,12,14},                  {5,10,22,13,11},
        /*D*/ {-4,12,18,18,16},                 {2,8,20,15,17},                  {4,9,27,16,18},                  {6,6,28,17,15},
        /*E*/ {-5,16,22,22,20},                 {2,16,2,19,21}, {3,13,25,20,22},                 {5,14,5,21,19},
        /*A*/ {-1,20,25,25,24}, {1,12,1,23,25},                 {3,21,3,24,23},
        /*F*/ {-6,24,28,28,27},                                                  {4,17,4,26,28},                  {6,18,6,27,26},
        {NOT_PROCESSED,27,0,28,NOT_PROCESSED},
    };
    DisasterLinks network(cities);

    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], networkHeaders[i]);
    }

    for (int i = 0; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxItems[i]);
    }
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
    Vector<DisasterLinks::cityHeader> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    Vector<DisasterLinks::cityItem> dlxItems = {
        /* Network
         *
         *      A  B  C  D  E  F
         *   B     1     1  1  1
         *   D  1  1  1  1
         *   A  1        1
         *   C        1  1
         *   E     1        1
         *   F     1           1
         */


        //        0                    1A             2B                3C                4D                5E               6F
              {0,0,0,6,1},       {2,18,13,0,2},   {4,27,8,1,3},     {2,21,15,2,4},   {4,22,9,3,5},     {2,25,10,4,6},   {2,28,11,5,0},
        //        7                                   8B                                  9D                10E              11F
        /*B*/ {-2,0,11,11,8},                     {2,2,14,7,9},                      {4,4,16,8,10},    {5,5,25,9,11},   {6,6,28,10,7},
        //        12                   13A            14B               15C               16D
        /*D*/ {-4,8,16,16,13},   {1,1,18,12,14},  {2,8,24,13,15},   {3,3,21,14,16},  {4,9,19,15,12},
        //        17                   18A                                                19D
        /*A*/ {-1,13,19,19,18},  {1,13,1,17,19},                                     {4,16,22,18,17},
        //        20                                                    21C               22D
        /*C*/ {-3,18,22,22,21},                                     {3,15,3,20,22},  {4,19,4,21,20},
        //        23                                  24B                                                   25E
        /*E*/ {-5,21,25,25,24},                   {2,14,27,23,25},                                     {5,10,5,24,23},
        //        26                                  27B                                                                    28F
        /*F*/ {-6,24,28,28,27},                   {2,24,2,26,28},                                                       {6,11,6,27,26},
        //        29



              {NOT_PROCESSED,27,0,28,NOT_PROCESSED},

    };

    DisasterLinks network (cities);

    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], networkHeaders[i]);
    }

    for (int i = 0; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxItems[i]);
    }
}


/* * * * * * * * * * * * * * * * *         Cover/Supplying Logic        * * * * * * * * * * * * * */


STUDENT_TEST("Supplying A will only cover A and C. C remains available supply location.") {
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
    Vector<DisasterLinks::cityHeader> networkHeaders = {
        {"",  3, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 0}
    };
    Vector<DisasterLinks::cityItem> dlxItems = {
        //        0                 1A           2B            3C
              {0,0,0,3,1},     {2,9,5,0,2},  {2,12,6,1,3},  {3,13,7,2,0},
        /*C*/ {-3,0,7,7,5},    {1,1,9,4,6},  {2,2,12,5,7},  {3,3,10,6,4},
        /*A*/ {-1,5,10,10,9},  {1,5,1,8,10},                {3,7,13,9,8},
        /*B*/ {-2,9,13,13,12},               {2,6,2,11,13}, {3,10,3,12,11},
              {NOT_PROCESSED,12,0,13,NOT_PROCESSED},
    };
    DisasterLinks network (cities);

    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], networkHeaders[i]);
    }
    for (int i = 0; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxItems[i]);
    }

    std::string supplyLocation = network.coverCity(5);

    EXPECT_EQUAL(supplyLocation, "A");

    Vector<DisasterLinks::cityHeader> headersCoverA = {
        {"",  2, 2},
        {"A", 0, 2},
        {"B", 0, 0},
        {"C", 2, 0}
    };
    Vector<DisasterLinks::cityItem> dlxCoverA = {
        /* Smaller network now.
         *       B
         *    B  1
         *    C  1
         */
        //        0                 1A           2B            3C
              {0,0,0,2,2},     {2,9,5,0,2},  {2,12,6,0,0},  {3,13,7,2,0},
        //        4                 5A           6B            7C
        /*C*/ {-3,0,7,6,6},    {1,1,9,4,6},  {2,2,12,4,4},  {3,3,10,6,4},
        //        8                 9A                         10C
        /*A*/ {-1,5,10,10,9},  {1,5,1,8,10},                {3,7,13,9,8},
        //        11                             12A           13C
        /*B*/ {-2,9,13,12,12},               {2,6,2,11,11}, {3,10,3,12,11},
              {NOT_PROCESSED,12,0,13,NOT_PROCESSED},
    };

    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], headersCoverA[i]);
    }
    for (int i = 0; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxCoverA[i]);
    }
}

STUDENT_TEST("Supplying B will only cover B and C. Make sure splicing from lookupTable works.") {
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
    Vector<DisasterLinks::cityHeader> networkHeaders = {
        {"",  3, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 0}
    };
    Vector<DisasterLinks::cityItem> dlxItems = {
        //        0                1A             2B            3C
              {0,0,0,3,1},     {2,9,5,0,2},  {2,12,6,1,3},  {3,13,7,2,0},
        /*C*/ {-3,0,7,7,5},    {1,1,9,4,6},  {2,2,12,5,7},  {3,3,10,6,4},
        /*A*/ {-1,5,10,10,9},  {1,5,1,8,10},                {3,7,13,9,8},
        /*B*/ {-2,9,13,13,12},               {2,6,2,11,13}, {3,10,3,12,11},
        {NOT_PROCESSED,12,0,13,NOT_PROCESSED},
    };
    DisasterLinks network (cities);

    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], networkHeaders[i]);
    }
    for (int i = 0; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxItems[i]);
    }

    // This covers down from index 6 AKA accessing item B with option B.
    std::string supplyLocation = network.coverCity(6);

    EXPECT_EQUAL(supplyLocation, "B");

    Vector<DisasterLinks::cityHeader> headersCoverB = {
        {"",  1, 1},
        {"A", 0, 0},
        {"B", 1, 0},
        {"C", 2, 0}
    };
    Vector<DisasterLinks::cityItem> dlxCoverB = {
        /* Smaller network now.
         *       A
         *    A  1
         *    C  1
         */

        //        0                 1A             2B            3C
              {0,0,0,1,1},     {2,9,5,0,0},  {2,12,6,1,3},  {3,13,7,1,0},
        //        4                 5A             6B            7C
        /*C*/ {-3,0,7,5,5},    {1,1,9,4,4},  {2,2,12,5,7},  {3,3,10,5,4},
        //        8                 9A                           10C
        /*A*/ {-1,5,10,9,9},   {1,5,1,8,8},                 {3,7,13,9,8},
        //        11                               12B           13C
        /*B*/ {-2,9,13,13,12},               {2,6,2,11,13}, {3,10,3,12,11},
              {NOT_PROCESSED,12,0,13,NOT_PROCESSED},
    };

    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], headersCoverB[i]);
    }

    for (int i = 0; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxCoverB[i]);
    }
}


STUDENT_TEST("Supplying C will cover all. Make sure splicing from lookupTable works.") {
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
    Vector<DisasterLinks::cityHeader> networkHeaders = {
        {"",  3, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 0}
    };
    Vector<DisasterLinks::cityItem> dlxItems = {
        //        0                1A             2B            3C
              {0,0,0,3,1},     {2,9,5,0,2},  {2,12,6,1,3},  {3,13,7,2,0},
        /*C*/ {-3,0,7,7,5},    {1,1,9,4,6},  {2,2,12,5,7},  {3,3,10,6,4},
        /*A*/ {-1,5,10,10,9},  {1,5,1,8,10},                {3,7,13,9,8},
        /*B*/ {-2,9,13,13,12},               {2,6,2,11,13}, {3,10,3,12,11},
              {NOT_PROCESSED,12,0,13,NOT_PROCESSED},
    };
    DisasterLinks network (cities);
    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], networkHeaders[i]);
    }
    for (int i = 0; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxItems[i]);
    }

    // Any item we choose from the header will select option C in the first row of options.
    std::string supplyLocation = network.coverCity(1);

    EXPECT_EQUAL(supplyLocation, "C");

    Vector<DisasterLinks::cityHeader> headersOptionC = {
        {"",  0, 0},
        {"A", 0, 0},
        {"B", 1, 3},
        {"C", 1, 0}
    };
    Vector<DisasterLinks::cityItem> dlxOptionC = {
        /* Successful empty network.
         *
         *       0
         *    A
         *    B
         */

        //        0                 1A             2B           3C
              {0,0,0,0,0},     {2,9,5,0,2},  {2,12,6,0,3},  {3,13,7,0,0},
        //        4                 5A             6B            7C
        /*C*/ {-3,0,7,7,5},    {1,1,9,4,6},  {2,2,12,5,7},  {3,3,10,6,4},
        //        8                 9A                           10C
        /*A*/ {-1,5,10,8,8},   {1,5,1,8,10},                {3,7,13,8,8},
        //        11                               12B           13C
        /*B*/ {-2,9,13,11,11},               {2,6,2,11,13}, {3,10,3,11,11},
        {NOT_PROCESSED,12,0,13,NOT_PROCESSED},
    };

    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], headersOptionC[i]);
    }

    for (int i = 0; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxOptionC[i]);
    }
}


/* * * * * * * * * * * * * * * * *       Cover then Uncover Logic       * * * * * * * * * * * * * */


STUDENT_TEST("Supplying A will only cover A and C with one supply. Uncover to try again.") {
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
    Vector<DisasterLinks::cityHeader> networkHeaders = {
        {"",  3, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 0}
    };
    Vector<DisasterLinks::cityItem> dlxItems = {
        //        0                1A             2B            3C
              {0,0,0,3,1},     {2,9,5,0,2},  {2,12,6,1,3},  {3,13,7,2,0},
        /*C*/ {-3,0,7,7,5},    {1,1,9,4,6},  {2,2,12,5,7},  {3,3,10,6,4},
        /*A*/ {-1,5,10,10,9},  {1,5,1,8,10},                {3,7,13,9,8},
        /*B*/ {-2,9,13,13,12},               {2,6,2,11,13}, {3,10,3,12,11},
              {NOT_PROCESSED,12,0,13,NOT_PROCESSED},
    };
    DisasterLinks network (cities);
    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], networkHeaders[i]);
    }
    for (int i = 0; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxItems[i]);
    }

    std::string supplyLocation = network.coverCity(5);

    EXPECT_EQUAL(supplyLocation, "A");


    Vector<DisasterLinks::cityHeader> headersCoverA = {
        {"",  2, 2},
        {"A", 0, 2},
        {"B", 0, 0},
        {"C", 2, 0}
    };
    Vector<DisasterLinks::cityItem> dlxCoverA = {
        /* Smaller network now.
         *       B
         *    B  1
         *    C  1
         */
        //        0                 1A           2B            3C
              {0,0,0,2,2},     {2,9,5,0,2},  {2,12,6,0,0},  {3,13,7,2,0},
        //        4                 5A           6B            7C
        /*C*/ {-3,0,7,6,6},    {1,1,9,4,6},  {2,2,12,4,4},  {3,3,10,6,4},
        //        8                 9A                         10C
        /*A*/ {-1,5,10,10,9},  {1,5,1,8,10},                {3,7,13,9,8},
        //        11                             12A           13C
        /*B*/ {-2,9,13,12,12},               {2,6,2,11,11}, {3,10,3,12,11},
              {NOT_PROCESSED,12,0,13,NOT_PROCESSED},
    };

    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], headersCoverA[i]);
    }
    for (int i = 0; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxCoverA[i]);
    }

    // We can just check it against our original array. All state should be returned to normal.
    network.uncoverCity(5);

    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], networkHeaders[i]);
    }
    for (int i = 0; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxItems[i]);
    }
}

STUDENT_TEST("Simple Ethene cover B, large item wipe out with D remaining an option.") {
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
    Vector<DisasterLinks::cityHeader> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    Vector<DisasterLinks::cityItem> dlxItems = {
        /* Network
         *
         *      A  B  C  D  E  F
         *   B     1     1  1  1
         *   D  1  1  1  1
         *   A  1        1
         *   C        1  1
         *   E     1        1
         *   F     1           1
         */

        //        0                  1A             2B               3C               4D            5E             6F
              {0,0,0,6,1},      {2,18,13,0,2},  {4,27,8,1,3},   {2,21,15,2,4},  {4,22,9,3,5},    {2,25,10,4,6},  {2,28,11,5,0},
        /*B*/ {-2,0,11,11,8},                   {2,2,14,7,9},                   {4,4,16,8,10},   {5,5,25,9,11},  {6,6,28,10,7},
        /*D*/ {-4,8,16,16,13},  {1,1,18,12,14}, {2,8,24,13,15}, {3,3,21,14,16}, {4,9,19,15,12},
        /*A*/ {-1,13,19,19,18}, {1,13,1,17,19},                                 {4,16,22,18,17},
        /*C*/ {-3,18,22,22,21},                                 {3,15,3,20,22}, {4,19,4,21,20},
        /*E*/ {-5,21,25,25,24},                 {2,14,27,23,25},                                 {5,10,5,24,23},
        /*F*/ {-6,24,28,28,27},                 {2,24,2,26,28},                                                  {6,11,6,27,26},
              {NOT_PROCESSED,27,0,28,NOT_PROCESSED},
    };

    DisasterLinks network (cities);

    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], networkHeaders[i]);
    }

    for (int i = 0; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxItems[i]);
    }

    std::string supplyLocation = network.coverCity(2);

    EXPECT_EQUAL(supplyLocation, "B");

    Vector<DisasterLinks::cityHeader> headersCoverB = {
        {"",  3, 1},
        {"A", 0, 3},
        {"B", 1, 3},
        {"C", 1, 0},
        {"D", 3, 5},
        {"E", 3, 6},
        {"F", 3, 0}
    };

    Vector<DisasterLinks::cityItem> dlxCoverB = {
        /* Network is now smaller but we can supply D option still even though D item is gone.
         *
         *      A  C
         *   A  1
         *   C     1
         *   D  1  1
         *   E
         *   F
         */

        //        0                  1A             2B               3C               4D                 5E             6F
              {0,0,0,3,1},      {2,18,13,0,3},  {4,27,8,1,3},    {2,21,15,1,0},  {4,22,9,3,5},    {2,25,10,3,6},  {2,28,11,3,0},
        /*B*/ {-2,0,11,11,8},                   {2,2,14,7,9},                    {4,4,16,8,10},   {5,5,25,9,11},  {6,6,28,10,7},
        /*D*/ {-4,8,16,15,13},  {1,1,18,12,15}, {2,8,24,13,15},  {3,3,21,13,12}, {4,9,19,15,12},
        /*A*/ {-1,13,19,18,18}, {1,13,1,17,17},                                  {4,16,22,18,17},
        /*C*/ {-3,18,22,21,21},                                  {3,15,3,20,20}, {4,19,4,21,20},
        /*E*/ {-5,21,25,23,23},                 {2,14,27,23,25},                                 {5,10,5,23,23},
        /*F*/ {-6,24,28,26,26},                 {2,24,2,26,28},                                                  {6,11,6,26,26},
              {NOT_PROCESSED,27,0,28,NOT_PROCESSED},
    };
    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], headersCoverB[i]);
    }
    for (int i = 0; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxCoverB[i]);
    }

    network.uncoverCity(2);

    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], networkHeaders[i]);
    }
    for (int i = 0; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxItems[i]);
    }

}


STUDENT_TEST("Simple Ethene cover A with option D.") {
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
    Vector<DisasterLinks::cityHeader> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    Vector<DisasterLinks::cityItem> dlxItems = {
        /* Network
         *
         *      A  B  C  D  E  F
         *   B     1     1  1  1
         *   D  1  1  1  1
         *   A  1        1
         *   C        1  1
         *   E     1        1
         *   F     1           1
         */

        //        0                  1A             2B               3C               4D            5E             6F
              {0,0,0,6,1},      {2,18,13,0,2},  {4,27,8,1,3},   {2,21,15,2,4},  {4,22,9,3,5},    {2,25,10,4,6},  {2,28,11,5,0},
        /*B*/ {-2,0,11,11,8},                   {2,2,14,7,9},                   {4,4,16,8,10},   {5,5,25,9,11},  {6,6,28,10,7},
        /*D*/ {-4,8,16,16,13},  {1,1,18,12,14}, {2,8,24,13,15}, {3,3,21,14,16}, {4,9,19,15,12},
        /*A*/ {-1,13,19,19,18}, {1,13,1,17,19},                                 {4,16,22,18,17},
        /*C*/ {-3,18,22,22,21},                                 {3,15,3,20,22}, {4,19,4,21,20},
        /*E*/ {-5,21,25,25,24},                 {2,14,27,23,25},                                 {5,10,5,24,23},
        /*F*/ {-6,24,28,28,27},                 {2,24,2,26,28},                                                  {6,11,6,27,26},
              {NOT_PROCESSED,27,0,28,NOT_PROCESSED},
    };

    DisasterLinks network (cities);

    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], networkHeaders[i]);
    }

    for (int i = 0; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxItems[i]);
    }

    std::string supplyLocation = network.coverCity(1);

    EXPECT_EQUAL(supplyLocation, "D");

    Vector<DisasterLinks::cityHeader> headersCoverD = {
        {"",  6, 5},
        {"A", 0, 5},
        {"B", 1, 3},
        {"C", 1, 4},
        {"D", 1, 5},
        {"E", 0, 6},
        {"F", 5, 0}
    };

    Vector<DisasterLinks::cityItem> dlxCoverD = {
        /* Network is now smaller but we can supply D option still even though D item is gone.
         *
         *      A  C
         *   D  1  1
         *   A  1
         *   C     1
         *   E
         *   F
         */

        //        0                  1A               2B             3C                4D             5E               6F
              {0,0,0,6,5},      {2,18,13,0,2},   {4,27,8,0,3},    {2,21,15,0,4},  {4,22,9,0,5},    {2,25,10,0,6},  {2,28,11,5,0},
        /*B*/ {-2,0,11,11,10},  {2,2,14,7,9},                                     {4,4,16,7,10},   {5,5,25,7,11},  {6,6,28,10,7},
        /*D*/ {-4,8,16,16,13},  {1,1,18,12,14},  {2,8,24,13,15},  {3,3,21,14,16}, {4,9,19,15,12},
        /*A*/ {-1,13,19,17,17}, {1,13,1,17,19},                                   {4,16,22,17,17},
        /*C*/ {-3,18,22,20,20},                                   {3,15,3,20,22}, {4,19,4,20,20},
        /*E*/ {-5,21,25,25,25},                  {2,14,27,23,25},                                  {5,10,5,23,23},
        /*F*/ {-6,24,28,28,28},                  {2,24,2,26,28},                                                   {6,11,6,26,26},
              {NOT_PROCESSED,27,0,28,NOT_PROCESSED},
    };
    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], headersCoverD[i]);
    }
    for (int i = 0; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxCoverD[i]);
    }

    network.uncoverCity(1);

    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], networkHeaders[i]);
    }
    for (int i = 1; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxItems[i]);
    }

}

STUDENT_TEST("Test for a depth 2 cover and uncover. Two covers then two uncovers.") {
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
    Vector<DisasterLinks::cityHeader> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    Vector<DisasterLinks::cityItem> dlxItems = {
        /* Network
         *
         *      A  B  C  D  E  F
         *   B     1     1  1  1
         *   D  1  1  1  1
         *   A  1        1
         *   C        1  1
         *   E     1        1
         *   F     1           1
         */

        //        0                  1A             2B               3C               4D            5E             6F
              {0,0,0,6,1},      {2,18,13,0,2},  {4,27,8,1,3},   {2,21,15,2,4},  {4,22,9,3,5},    {2,25,10,4,6},  {2,28,11,5,0},
        /*B*/ {-2,0,11,11,8},                   {2,2,14,7,9},                   {4,4,16,8,10},   {5,5,25,9,11},  {6,6,28,10,7},
        /*D*/ {-4,8,16,16,13},  {1,1,18,12,14}, {2,8,24,13,15}, {3,3,21,14,16}, {4,9,19,15,12},
        /*A*/ {-1,13,19,19,18}, {1,13,1,17,19},                                 {4,16,22,18,17},
        /*C*/ {-3,18,22,22,21},                                 {3,15,3,20,22}, {4,19,4,21,20},
        /*E*/ {-5,21,25,25,24},                 {2,14,27,23,25},                                 {5,10,5,24,23},
        /*F*/ {-6,24,28,28,27},                 {2,24,2,26,28},                                                  {6,11,6,27,26},
              {NOT_PROCESSED,27,0,28,NOT_PROCESSED},
    };
    DisasterLinks network (cities);
    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], networkHeaders[i]);
    }
    for (int i = 0; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxItems[i]);
    }

    std::string supplyLocation = network.coverCity(13);

    EXPECT_EQUAL(supplyLocation, "A");

    Vector<DisasterLinks::cityHeader> headersCoverA = {
        {"",  6, 2},
        {"A", 0, 2},
        {"B", 0, 3},
        {"C", 2, 5},
        {"D", 3, 5},
        {"E", 3, 6},
        {"F", 5, 0}
    };

    Vector<DisasterLinks::cityItem> dlxCoverA = {
        /* Network is now smaller.
         *
         *      B  C  E  F
         *   B  1     1  1
         *   D  1  1
         *   C     1
         *   E  1     1
         *   F  1        1
         */

        //         0                1A             2B               3C               4D                 5E             6F
              {0,0,0,6,2},      {2,18,13,0,2},  {4,27,8,0,3},   {2,21,15,2,5},  {4,22,9,3,5},    {2,25,10,3,6},  {2,28,11,5,0},
        /*B*/ {-2,0,11,11,8},                   {2,2,14,7,10},                  {4,4,16,8,10},   {5,5,25,8,11},  {6,6,28,10,7},
        /*D*/ {-4,8,16,15,14},  {1,1,18,12,14}, {2,8,24,12,15}, {3,3,21,14,12}, {4,9,19,15,12},
        /*A*/ {-1,13,19,19,18}, {1,13,1,17,19},                                 {4,16,22,18,17},
        /*C*/ {-3,18,22,21,21},                                 {3,15,3,20,20}, {4,19,4,21,20},
        /*E*/ {-5,21,25,25,24},                 {2,14,27,23,25},                                 {5,10,5,24,23},
        /*F*/ {-6,24,28,28,27},                 {2,24,2,26,28},                                                  {6,11,6,27,26},
        {NOT_PROCESSED,27,0,28,NOT_PROCESSED},
    };
    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], headersCoverA[i]);
    }
    for (int i = 0; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxCoverA[i]);
    }

    supplyLocation = network.coverCity(15);

    EXPECT_EQUAL(supplyLocation, "C");

    Vector<DisasterLinks::cityHeader> headersCoverC = {
        {"",  6, 2},
        {"A", 0, 2},
        {"B", 0, 5},
        {"C", 2, 5},
        {"D", 3, 5},
        {"E", 2, 6},
        {"F", 5, 0}
    };

    Vector<DisasterLinks::cityItem> dlxCoverC = {
        /* Second cover operation.
         *
         *      B  E  F
         *   B  1  1  1
         *   D  1
         *   E  1  1
         *   F  1     1
         */

        //        0                  1A             2B               3C               4D              5E          6F
              {0,0,0,6,2},      {2,18,13,0,2},  {4,27,8,0,5},    {2,21,15,2,5},  {4,22,9,3,5},   {2,25,10,2,6},  {2,28,11,5,0},
        /*B*/ {-2,0,11,11,8},                   {2,2,14,7,10},                   {4,4,16,8,10},  {5,5,25,8,11},  {6,6,28,10,7},
        /*D*/ {-4,8,16,14,14},  {1,1,18,12,14}, {2,8,24,12,12},  {3,3,21,14,12}, {4,9,19,15,12},
        /*A*/ {-1,13,19,19,18}, {1,13,1,17,19}, {4,16,22,18,17},
        /*C*/ {-3,18,22,21,21},                                  {3,15,3,20,20}, {4,19,4,21,20},
        /*E*/ {-5,21,25,25,24},                 {2,14,27,23,25},                                 {5,10,5,24,23},
        /*F*/ {-6,24,28,28,27},                 {2,24,2,26,28},                                                  {6,11,6,27,26},
        {NOT_PROCESSED,27,0,28,NOT_PROCESSED},
    };

    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], headersCoverC[i]);
    }
    for (int i = 1; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxCoverC[i]);
    }

    network.uncoverCity(15);

    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], headersCoverA[i]);
    }
    for (int i = 1; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxCoverA[i]);
    }

    network.uncoverCity(13);

    for (int i = 0; i < networkHeaders.size(); i++) {
        EXPECT_EQUAL(network.dlx.lookupTable[i], networkHeaders[i]);
    }
    for (int i = 1; i < dlxItems.size(); i++) {
        EXPECT_EQUAL(network.dlx.grid[i], dlxItems[i]);
    }

}


/* * * * * * * * * * * * * * * * *       Test Disaster Supplies         * * * * * * * * * * * * * */


/* This is a helper function that's useful for designing test cases. You give it a Map
 * of cities and what they're adjacent to, and it then produces a new Map where if city
 * A links to city B, then city B links back to city A. We recommend using this helper
 * function when writing tests, though you won't need it in your implementation of the main
 * canBeMadeDisasterReady function.
 */
Map<std::string, Set<std::string>> makeMap(const Map<std::string, Set<std::string>>& source) {
    Map<std::string, Set<std::string>> result = source;

    for (const std::string& from: source) {
        for (const std::string& to: source[from]) {
            result[from] += to;
            result[to] += from;
        }
    }

    return result;
}

/* This helper function tests whether a city has been covered by a set of supply locations
 * and is used by our testing code. You're welcome to use it in your tests as well!
 */
bool checkCovered(const std::string& city,
                  const Map<std::string, Set<std::string>>& roadNetwork,
                  const Set<std::string>& supplyLocations) {
    if (supplyLocations.contains(city)) return true;

    for (const std::string& neighbor: roadNetwork[city]) {
        if (supplyLocations.contains(neighbor)) return true;
    }

    return false;
}


STUDENT_TEST("Supplying C will cover all.") {
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

    DisasterLinks network(cities);
    Set<std::string> chosen = {};
    EXPECT(network.isDisasterReady(1, chosen));
}

STUDENT_TEST("Simple Ethene cover D and B to succeed.") {
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
    Vector<DisasterLinks::cityHeader> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    Vector<DisasterLinks::cityItem> dlxItems = {
        /* Network
         *
         *      A  B  C  D  E  F
         *   B     1     1  1  1
         *   D  1  1  1  1
         *   A  1        1
         *   C        1  1
         *   E     1        1
         *   F     1           1
         */

        //        0                  1A             2B               3C               4D            5E             6F
              {0,0,0,6,1},      {2,18,13,0,2},  {4,27,8,1,3},   {2,21,15,2,4},  {4,22,9,3,5},    {2,25,10,4,6},  {2,28,11,5,0},
        /*B*/ {-2,0,11,11,8},                   {2,2,14,7,9},                   {4,4,16,8,10},   {5,5,25,9,11},  {6,6,28,10,7},
        /*D*/ {-4,8,16,16,13},  {1,1,18,12,14}, {2,8,24,13,15}, {3,3,21,14,16}, {4,9,19,15,12},
        /*A*/ {-1,13,19,19,18}, {1,13,1,17,19},                                 {4,16,22,18,17},
        /*C*/ {-3,18,22,22,21},                                 {3,15,3,20,22}, {4,19,4,21,20},
        /*E*/ {-5,21,25,25,24},                 {2,14,27,23,25},                                 {5,10,5,24,23},
        /*F*/ {-6,24,28,28,27},                 {2,24,2,26,28},                                                  {6,11,6,27,26},
              {NOT_PROCESSED,27,0,28,NOT_PROCESSED},
    };
    DisasterLinks network(cities);
    Set<std::string> chosen = {};
    EXPECT(!network.isDisasterReady(1, chosen));
    // Make sure that we cleanup our data structure between calls in case it is tested before destr.
    EXPECT_EQUAL(network.dlx.lookupTable, networkHeaders);
    EXPECT_EQUAL(network.dlx.grid, dlxItems);
    EXPECT(network.isDisasterReady(2, chosen));
    EXPECT_EQUAL(network.dlx.lookupTable, networkHeaders);
    EXPECT_EQUAL(network.dlx.grid, dlxItems);
}

STUDENT_TEST("Make sure data structure returns to original state after many calls in a row.") {
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
    Vector<DisasterLinks::cityHeader> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    Vector<DisasterLinks::cityItem> dlxItems = {
        /* Network
         *
         *      A  B  C  D  E  F
         *   B     1     1  1  1
         *   D  1  1  1  1
         *   A  1        1
         *   C        1  1
         *   E     1        1
         *   F     1           1
         */

        //        0                  1A             2B               3C               4D            5E             6F
              {0,0,0,6,1},      {2,18,13,0,2},  {4,27,8,1,3},   {2,21,15,2,4},  {4,22,9,3,5},    {2,25,10,4,6},  {2,28,11,5,0},
        /*B*/ {-2,0,11,11,8},                   {2,2,14,7,9},                   {4,4,16,8,10},   {5,5,25,9,11},  {6,6,28,10,7},
        /*D*/ {-4,8,16,16,13},  {1,1,18,12,14}, {2,8,24,13,15}, {3,3,21,14,16}, {4,9,19,15,12},
        /*A*/ {-1,13,19,19,18}, {1,13,1,17,19},                                 {4,16,22,18,17},
        /*C*/ {-3,18,22,22,21},                                 {3,15,3,20,22}, {4,19,4,21,20},
        /*E*/ {-5,21,25,25,24},                 {2,14,27,23,25},                                 {5,10,5,24,23},
        /*F*/ {-6,24,28,28,27},                 {2,24,2,26,28},                                                  {6,11,6,27,26},
              {NOT_PROCESSED,27,0,28,NOT_PROCESSED},
    };
    DisasterLinks network(cities);
    Set<std::string> chosen = {};
    for (int i = 50; i >= 0; i--) {
        if (i < 2) {
            EXPECT(!network.isCovered(i, chosen));
        } else {
            EXPECT(network.isCovered(i, chosen));
        }
        chosen.clear();
        // No matter how many tests we do, the data structure should always restore itself.
        EXPECT_EQUAL(network.dlx.grid, dlxItems);
        EXPECT_EQUAL(network.dlx.lookupTable, networkHeaders);
    }
}

STUDENT_TEST("The straight line test. This will help us make sure we manage options correctly.") {
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
    Vector<DisasterLinks::cityHeader> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    Vector<DisasterLinks::cityItem> dlxItems = {
        //        0                 1A             2B               3C               4D              5E             6F
              {0,0,0,6,1},     {2,24,12,0,2},   {3,20,8,1,3},   {3,25,13,2,4},   {3,27,9,3,5},   {3,22,10,4,6},   {2,28,18,5,0},
        /*B*/ {-2,0,10,10,8},                   {2,2,16,7,9},                    {4,4,17,8,10},  {5,5,14,9,7},
        /*C*/ {-3,8,14,14,12},  {1,1,24,11,13},                 {3,3,21,12,14},                  {5,10,22,13,11},
        /*D*/ {-4,12,18,18,16},                 {2,8,20,15,17},                  {4,9,27,16,18},                  {6,6,28,17,15},
        /*E*/ {-5,16,22,22,20},                 {2,16,2,19,21}, {3,13,25,20,22},                 {5,14,5,21,19},
        /*A*/ {-1,20,25,25,24}, {1,12,1,23,25},                 {3,21,3,24,23},
        /*F*/ {-6,24,28,28,27},                                                  {4,17,4,26,28},                  {6,18,6,27,26},
        {NOT_PROCESSED,27,0,28,NOT_PROCESSED},
    };
    DisasterLinks network(cities);
    Set<std::string> chosen = {};
    EXPECT(!network.isDisasterReady(1, chosen));
    EXPECT_EQUAL(network.dlx.grid, dlxItems);
    EXPECT_EQUAL(network.dlx.lookupTable, networkHeaders);
    EXPECT(network.isDisasterReady(2, chosen));
    EXPECT_EQUAL(network.dlx.grid, dlxItems);
    EXPECT_EQUAL(network.dlx.lookupTable, networkHeaders);
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
    DisasterLinks network(cities);
    Set<std::string> chosen = {};
    EXPECT(!network.isDisasterReady(1, chosen));
    EXPECT(network.isDisasterReady(2, chosen));
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
    DisasterLinks network(cities);
    Set<std::string> chosen = {};
    EXPECT(!network.isDisasterReady(1, chosen));
    EXPECT(!network.isDisasterReady(2, chosen));
    EXPECT(network.isDisasterReady(3, chosen));
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
    DisasterLinks network(cities);
    Set<std::string> chosen = {};
    EXPECT(!network.isDisasterReady(1, chosen));
    EXPECT(!network.isDisasterReady(2, chosen));
    EXPECT(!network.isDisasterReady(3, chosen));
    EXPECT(!network.isDisasterReady(4, chosen));
    EXPECT(network.isDisasterReady(5, chosen));
}


PROVIDED_TEST("Can solve ethene example, regardless of ordering.") {
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

        DisasterLinks network(map);
        Set<std::string> chosen = {};
        EXPECT(network.isDisasterReady(2, chosen));

        chosen.clear();

        EXPECT(!network.isDisasterReady(1, chosen));
    } while (next_permutation(cities.begin(), cities.end()));
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

        DisasterLinks network(map);
        Set<std::string> chosen = {};
        EXPECT(network.isDisasterReady(2, chosen));

        EXPECT_EQUAL(chosen.size(), 2);
        EXPECT(chosen.contains(cities[2]));
        EXPECT(chosen.contains(cities[3]));

        chosen.clear();

        EXPECT(!network.isDisasterReady(1, chosen));
    } while (next_permutation(cities.begin(), cities.end()));
}

PROVIDED_TEST("Works for six cities in a line, regardless of order.") {
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

        DisasterLinks network(map);
        EXPECT(network.isDisasterReady(2, chosen));

        chosen.clear();

        EXPECT(!network.isDisasterReady(1, chosen));
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

        DisasterLinks network(map);
        EXPECT(network.isDisasterReady(2, chosen));

        EXPECT_EQUAL(chosen.size(), 2);
        EXPECT(chosen.contains(cities[1]));
        EXPECT(chosen.contains(cities[4]));

        chosen.clear();

        EXPECT(!network.isDisasterReady(1, chosen));
    } while (next_permutation(cities.begin(), cities.end()));
}

PROVIDED_TEST("Solves \"Don't be Greedy,\" regardless of ordering.") {
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

        DisasterLinks network(map);
        /* We should be able to cover everything with two cities. */
        EXPECT(network.isDisasterReady(2, chosen));

        chosen.clear();

        EXPECT(!network.isDisasterReady(1, chosen));
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

        DisasterLinks network(map);
        /* We should be able to cover everything with two cities. */
        EXPECT(network.isDisasterReady(2, chosen));

        /* Those cities should be 1 and 3. */
        EXPECT_EQUAL(chosen, { cities[1], cities[3] });

        chosen.clear();

        EXPECT(!network.isDisasterReady(1, chosen));
    } while (next_permutation(cities.begin(), cities.end()));
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

    DisasterLinks network(grid);
    Set<std::string> locations;
    EXPECT(network.isDisasterReady(10, locations));
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

    DisasterLinks network(grid);
    Set<std::string> locations;
    EXPECT(network.isDisasterReady(10, locations));

    for (char row = 'A'; row <= maxRow; row++) {
        for (int col = 1; col <= maxCol; col++) {
            EXPECT(checkCovered(row + std::to_string(col), grid, locations));
        }
    }
}
