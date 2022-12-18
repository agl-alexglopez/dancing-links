/**
 * Author: Alexander G. Lopez
 * File DisasterTags.h
 * --------------------------
 * This file defines the class for an implementation of Algorithm X via Dancing Links by Donald
 * Knuth. For a full discussion of this implementation see the readme.md. However, the basics are
 * that this implementation accomplishes a modified exact cover search of a transportation network
 * to determine if the network can be covered in case of emergency.
 *
 * A city is covered if it has supplies or is adjacent to a city with supplies. A city cannot be
 * supplied twice but if a city is safe due to being adjacent to a city with supplies it can still
 * receive supplies.
 *
 * I was required to heavily modify Donald Knuth's implemntation of Algorithm X via Dancing Links
 * to fit this problem. Here is the basic logic behind my choices:
 *
 * Organize the network by all the cities that exist along the top row and all the options for
 * covering them in rows beneath this. Take this world for example.
 *
 *             C
 *             |
 *        A -- D -- B -- F
 *                  |
 *                  E
 *
 * We can represent these connections with the following matrix.
 *
 *            A  B  C  D  E  F    <-Cities as items that exist in the world
 *         B     1     1  1  1    <-The cities that a city will cover if supplied
 *         D  1  1  1  1
 *         A  1        1
 *         C        1  1
 *         E     1        1
 *         F     1           1
 *
 * Notice that a city covers itself. This is important in the case of island cities that are
 * connected to no cities. I have also organized the rows so that the cities with the
 * most connections are at the top and will be explored first. We will now discuss the selection
 * process.
 *
 * We must distribute two supplies. Select the city to cover that is the most isolated in the
 * network. It will have the lowest number of appearances across all sets. The (*) marks our
 * selection. When we supply a city all of the associated cities in whichever supply option we
 * choose get a supply number tag. It is the same as the number of supplies remaining before we
 * give out that supply.
 *
 *            *
 *    Tag --> 2  2  2  2
 *            A  B  C  D  E  F
 *         B     1     1  1  1
 *    Tag --> 2  2  2  2
 *         D  1  1  1  1
 *         A  1        1
 *         C        1  1
 *         E     1        1
 *         F     1           1
 *
 * It is very likely that supplying the most connected city adjacent to the isolated city to supply
 * will be the best choice, so we will try to cover A this way first. Supply city D and make all
 * the cities attached to it safe. This is how our network shrinks.
 *
 *            E  F
 *         B  1  1
 *         A
 *         C
 *         E  1
 *         F    1
 *
 * Notice that A and C no longer can help this network as supply locations. The only helpful options
 * are B E and F. Luckily our selection heuristic will select to cover E with option B.
 *
 *    Tag --> 1  1
 *            E  F
 *    Tag --> 1  1
 *         B  1  1
 *         A
 *         C
 *         E  1
 *         F    1
 *
 *           0
 *         A
 *         C
 *         E
 *         F
 *
 * Thus our world is empty and the problem is solved by supplying cities B and D when given two
 * supplies. If we were given one supply we would not have been able cover this network. Had we
 * needed to take back the supplies from a supply location we would use the supply tag to only
 * uncover all associated cities with that supply tag. This saves a meaningful amount of work
 * when compared to an implementation that uses quadruply linked lists with up,down,left, and right
 * fields.
 *
 * If you wish to see how significantly this varies from Knuth's original implementation please read
 *
 *      The Art of Computer Programming,
 *      Volume 4B,
 *      Combinatorial Algorithms, Part 2,
 *      Sec. 7.2.2.1,
 *      Pg.65-70,
 *      Knuth
 *
 */
#ifndef DISASTERTAGS_H
#define DISASTERTAGS_H
#include <string>
#include <vector>
#include "GUI/SimpleTest.h"
#include "set.h"
#include "map.h"
#include "hashmap.h"


class DisasterTags {

public:

    typedef struct city {
        int topOrLen;
        int up;
        int down;
        /* New addition! If we tag all cities associated with a supply location with the same
         * supply number tag, we no longer need a second doubly linked list. No right left pointers
         * are needed. Instead we will always check the supply tags of headers for a colum and items
         * in that column. If the header has not been tagged, the city needs to be tagged and then
         * can be removed from the world. If a city has been tagged we know it has an associated
         * supply number and is safe. We will use this number if we need to uncover that city
         * because giving that exact supply did not work for all of the cities associated with that
         * supply number.
         */
        int supplyTag;
    }city;

    typedef struct cityName {
        std::string name;
        int left;
        int right;
    }cityName;


    /* * * * * * * * * *    Constructor and Dancing Links Solver        * * * * * * * * * * * * * */


    /**
     * @brief DisasterTags   a custom constructor for this class that can turn a Map representation
     *                       of a transportation grid into a vector grid prepared for exact cover
     *                       search via dancing links.
     * @param roadNetwork    the transportation grid passed in via map form.
     */
    explicit DisasterTags(const Map<std::string, Set<std::string>>& roadNetwork);

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
    bool hasDisasterCoverage(int numSupplies, Set<std::string>& supplyLocations);

    /**
     * @brief getAllDisasterConfigurations  returns every possible disaster configuration possible
     *                                      with a given supply count. I advise finding the optimal
     *                                      number of supplies before running this function or it
     *                                      will work very hard. It is slow.
     * @param numSupplies                   the number of supplies we have to distribute.
     * @return                              all possible distributions of the supplies.
     */
    Set<Set<std::string>> getAllDisasterConfigurations(int numSupplies);


    /* * * * * * * * * * * * *  Overloaded Debugging Operators  * * * * * * * * * * * * * * * * * */


    friend bool operator==(const city& lhs, const city& rhs);

    friend bool operator!=(const city& lhs, const city& rhs);

    friend bool operator==(const cityName& lhs, const cityName& rhs);

    friend bool operator!=(const cityName& lhs, const cityName& rhs);

    friend std::ostream& operator<<(std::ostream& os, const city& city);

    friend std::ostream& operator<<(std::ostream& os, const cityName& city);

    friend std::ostream& operator<<(std::ostream&os, const std::vector<city>& grid);

    friend std::ostream& operator<<(std::ostream&os, const std::vector<cityName>& links);

    friend std::ostream& operator<<(std::ostream&os, const DisasterTags& network);


private:


    /* * * * * * * * * *       Core Dancing Links Implementation        * * * * * * * * * * * * * */


    /* Use table to control recursion and know when all cities are safe. Table will be empty.
     * Grid contains cities as nodes. Cities apear as columns with appearances across rows.
     * Rows are the same cities but they indicate which other cities they cover if supplied.
     * All cities apear as rows and columns so the grid is square.
     */
    std::vector<cityName> table_;
    std::vector<city> grid_;
    int numItemsAndOptions_;


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
    bool isDLXCovered(int numSupplies, Set<std::string>& supplyLocations);

    /**
     * @brief fillConfigurations  finds all possible distributions of the given number of supplies.
     *                            It generates duplicate configurations and uses a Set to filter
     *                            them out. This is slow and I want to only generate unique
     *                            configurations but am having trouble finding a way.
     * @param numSupplies         the number of supplies we have to distribute.
     * @param suppliedCities      the set that will hold new configurations that work.
     * @param allConfigurations   the set that records all configurations found.
     */
    void fillConfigurations(int numSupplies,
                            Set<std::string>& suppliedCities,
                            Set<Set<std::string>>& allConfigurations);

    /**
     * @brief chooseIsolatedCity  selects a city we are trying to cover either by giving it supplies
     *                            or covering an adjacent neighbor. The selection uses the following
     *                            heuristic:
     *                              - Select the most isolated city so far.
     *                              - We must cover this city so select an adjacent city with the
     *                                most connections and try that first.
     *                              - If that fails try the next city with most connections.
     *                              - Finally, try actual city in question, not neighbors.
     * @return                    the index of the city we are selecting to attempt to cover.
     */
    int chooseIsolatedCity();

    /**
     * @brief coverCity      covers a city wit supplies and all of its neighbors. All cities tagged
     *                       with a supply number equivalent to the current depth of the recursive
     *                       search. This tells us which cities are now unsafe if distributing that
     *                       exact supply did not work out and it must be removed later.
     * @param indexInOption  the index for the current city we are trying to cover.
     * @param supplyTag      uses number of supplies remaining as a unique tag for recursive depth.
     * @return               the name of the city holding the supplies.
     */
    std::string coverCity(int indexInOption, const int supplyTag);

    /**
     * @brief uncoverCity    uncovers a city if that choice of option did not lead to a covered
     *                       network. Uncovers same option that was selected for coverage if given
     *                       the same index. Will only uncover those cities that were covered by
     *                       the previously given supply.
     * @param indexInOption  the index of the item we covered with the option below the index.
     */
    void uncoverCity(int indexInOption);


    /* * * * * * * * * *    Constructors for Dancing Links Building     * * * * * * * * * * * * * */


    /**
     * @brief initializeHeaders  creates the lookup table of city names and the first row of headers
     *                           that are in the dancing links array. This is the first pass on the
     *                           input map. We will perform a second, much longer pass to build the
     *                           columns later.
     * @param roadNetwork        the input of cities connected to other cities.
     * @param connectionSizes    a city and the size of the set of adjacent cities.
     * @param columnBuilder      the hash map we will use to connect newly added items to a column.
     */
    void initializeHeaders(const Map<std::string, Set<std::string>>& roadNetwork,
                           std::vector<std::pair<std::string,int>>& connectionSizes,
                           HashMap<std::string,int>& columnBuilder);

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
    void initializeItems(const Map<std::string, Set<std::string>>& roadNetwork,
                         const std::vector<std::pair<std::string,int>>& connectionSizes,
                         HashMap<std::string,int>& columnBuilder);

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
    int initializeColumns(const Set<std::string>& connections,
                          HashMap<std::string,int>& columnBuilder,
                          int index);

    ALLOW_TEST_ACCESS();
};

#endif // DISASTERTAGS
