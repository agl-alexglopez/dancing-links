/**
 * Author: Alexander G. Lopez
 * File DisasterTags.h
 * --------------------------
 * This file defines the class for an implementation of Algorithm X via Dancing Links by Donald
 * Knuth. For a full discussion of this implementation see the readme.md. However, the basics are
 * that this implementation accomplishes a modified exact cover search of a transportation network
 * to determine if the network can be covered in case of emergency.
 */
#ifndef DISASTERTAGS_H
#define DISASTERTAGS_H
#include <string>
#include <vector>
#include "GUI/SimpleTest.h"
#include <set>
#include <map>
#include <unordered_map>

namespace DancingLinks {

class DisasterTags {

public:


    /* * * * * * * * * *    Constructor and Dancing Links Solver        * * * * * * * * * * * * * */


    /**
     * @brief DisasterTags   a custom constructor for this class that can turn a std::map representation
     *                       of a transportation grid into a vector grid prepared for exact cover
     *                       search via dancing links.
     * @param roadNetwork    the transportation grid passed in via map form.
     */
    explicit DisasterTags(const std::map<std::string, std::set<std::string>>& roadNetwork);

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
    bool hasDisasterCoverage(int numSupplies, std::set<std::string>& supplyLocations);

    /**
     * @brief getAllDisasterConfigurations  returns every possible disaster configuration possible
     *                                      with a given supply count. I advise finding the optimal
     *                                      number of supplies before running this function or it
     *                                      will work very hard. It is slow.
     * @param numSupplies                   the number of supplies we have to distribute.
     * @return                              all possible distributions of the supplies.
     */
    std::set<std::set<std::string>> getAllDisasterConfigurations(int numSupplies);




private:


    struct city {
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
    };

    struct cityName {
        std::string name;
        int left;
        int right;
    };


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
    bool isDLXCovered(int numSupplies, std::set<std::string>& supplyLocations);

    /**
     * @brief fillConfigurations  finds all possible distributions of the given number of supplies.
     *                            It generates duplicate configurations and uses a std::set to filter
     *                            them out. This is slow and I want to only generate unique
     *                            configurations but am having trouble finding a way.
     * @param numSupplies         the number of supplies we have to distribute.
     * @param suppliedCities      the set that will hold new configurations that work.
     * @param allConfigurations   the set that records all configurations found.
     */
    void fillConfigurations(int numSupplies,
                              std::set<std::string>& suppliedCities,
                              std::set<std::set<std::string>>& allConfigurations);

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
    int chooseIsolatedCity() const;

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
    void initializeHeaders(const std::map<std::string, std::set<std::string>>& roadNetwork,
                             std::vector<std::pair<std::string,int>>& connectionSizes,
                             std::unordered_map<std::string,int>& columnBuilder);

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
    void initializeItems(const std::map<std::string, std::set<std::string>>& roadNetwork,
                          const std::vector<std::pair<std::string,int>>& connectionSizes,
                          std::unordered_map<std::string,int>& columnBuilder);

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
    int initializeColumns(const std::set<std::string>& connections,
                            std::unordered_map<std::string,int>& columnBuilder,
                            int index);


    /* * * * * * * * * * * * *  Overloaded Debugging Operators  * * * * * * * * * * * * * * * * * */


    friend bool operator==(const city& lhs, const city& rhs);
    friend bool operator!=(const city& lhs, const city& rhs);
    friend bool operator==(const cityName& lhs, const cityName& rhs);
    friend bool operator!=(const cityName& lhs, const cityName& rhs);
    friend std::ostream& operator<<(std::ostream& os, const city& city);
    friend std::ostream& operator<<(std::ostream& os, const cityName& city);
    friend std::ostream& operator<<(std::ostream& os, const std::vector<city>& grid);
    friend std::ostream& operator<<(std::ostream& os, const std::vector<cityName>& links);
    friend std::ostream& operator<<(std::ostream& os, const DisasterTags& network);
    ALLOW_TEST_ACCESS();
};


} // namespace DancingLinks

#endif // DISASTERTAGS
