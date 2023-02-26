/**
 * Author: Alexander G. Lopez
 * File: DisasterLinks.h
 * -------------------------
 * This file defines the class for an implementation of Algorithm X via Dancing Links by Donald
 * Knuth. For a full discussion of this implementation see the readme.md. However, the basics are
 * that this implementation accomplishes a modified exact cover search of a transportation network
 * to determine if the network can be covered in case of emergency.
 */
#ifndef DISASTERLINKS_H
#define DISASTERLINKS_H
#include <vector>
#include "GUI/SimpleTest.h"
#include <map>
#include <unordered_map>
#include <set>

namespace DancingLinks {

class DisasterLinks {

public:


    /**
     * @brief DisasterLinks  a custom constructor for this class that can turn a std::map representation
     *                       of a transportation grid into a vector grid prepared for exact cover
     *                       search via dancing links.
     * @param roadNetwork    the transportation grid passed in via map form.
     */
    explicit DisasterLinks(const std::map<std::string, std::set<std::string>>& roadNetwork);

    /**
     * @brief isDisasterReady  performs a recursive search to determine if a transportation grid
     *                         can be covered with the specified number of emergency supplies. It
     *                         will also place the found cities in the output parameter if there
     *                         exists a solution. A city is covered or safe if it has supplies or
     *                         is adjacent to a city with supplies. The solution may not use all
     *                         of the provided supplies.
     * @param numSupplies      the limiting number of supplies we must distribute.
     * @param suppliedCities   the output parameter telling which cities received supplies.
     * @return                 true if we have found a viable supply scheme, false if not.
     */
    bool isDisasterReady(int numSupplies, std::set<std::string>& suppliedCities);

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


    /* The cityItem will carry most of the logic of our problem. These nodes are in the grid we
     * set up in order for the links to "dance" as we recurse and leave the data structure in place.
     */
    struct cityItem {
        /* If this item is a column header, this is the number of items in a column.
         * If this item is in the grid, this is the index of the header for an item.
         */
        int topOrLen;
        // We traverse through options to cover one item with the up down field.
        int up;
        int down;
        // We cut an item within an option out of the world with the left right field to recurse.
        int left;
        int right;
    };

    /* The cityHeader helps us track what items still need to be covered. We also use this as a
     * lookup table for the names of the options in which we find an item. This option is the city
     * we have given supplies to.
     */
    struct cityHeader {
        std::string name;
        int left;
        int right;
    };

    /* This is our world. These data structures will remain in place during the recursive
     * algorithm. We do not need to make any copies, only modify the indices of the cityItems in
     * the grid in order to find the solution. All backtracking can be accomplished by only
     * modifying the fields of surrounding nodes and leaving the node in question unchanged.
     * This is the core concept behind Knuth's Algorithm X via Dancing Links which I have adapted
     * to this problem.
     */
    std::vector<cityHeader> table_;
    std::vector<cityItem> grid_;
    /* In this application, the number of colums equals the number of rows. Cities are both
     * items that need to be covered and cities that can receive supplies.
     */
    int numItemsAndOptions_;


    /* * * * * * * * * * * * * *       Modified Algorithm X via Dancing Links     * * * * * * * * */


    /**
     * @brief isCovered       performs a modified exact cover search of a transportation Network.
     *                        Given a number of supplies for the network, determines if every city
     *                        is covered. A city is covered if it has received supplies or is
     *                        adjacent to a city with supplies. If a city has supplies it may not
     *                        be supplied again. However, if a city is safely covered by an adjacent
     *                        city with supplies, this safe city may still receive supplies to cover
     *                        other cities. With infinite supplies this algorithm will find a good
     *                        solution. For the optimal solution, challenge decrease supply
     *                        counts until it confirms a network cannot be covered with that amount.
     * @param numSupplies     the number of supplies that we have to distribute over the network.
     * @param suppliedCities  the output parameter showing the cities we have chosen to supply.
     * @return                true if all cities are safe with given supplies, false if not.
     */
    bool isCovered(int numSupplies, std::set<std::string>& suppliedCities);

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
     * @brief coverCity      covers a city in the option indicated by the index. A city in question
     *                       may be covered by supplying a neighbor or supplying the city itself.
     * @param indexInOption  the index we start at for item in the supply option(row) we found it.
     * @return               the string of the option we used to cover a city, neighbor or city.
     */
    std::string coverCity(int indexInOption);

    /**
     * @brief uncoverCity    uncovers a city if that choice of option did not lead to a covered
     *                       network. Uncovers same option that was selected for coverage if given
     *                       the same index.
     * @param indexInOption  the index we start at for item in the supply option(row) we found it.
     */
    void uncoverCity(int indexInOption);

    /**
     * @brief hideCityCol  when we supply an option it covers itself and connected cities. We must
     *                     remove these cities from any other sets that contain them to make them
     *                     disappear from the world as uncovered cities. However, we keep them as
     *                     available cities to supply.
     * @param start        the city we start at in a row. We traverse downward to snip city out.
     */
    void hideCityCol(int indexInCol);

    /**
     * @brief unhideCityCol  when an option fails, we must put the cities it covers back into all
     *                       the sets to which they belong. This puts the cities back in network.
     * @param start          the city in the option we start at. We traverse upward to unhide.
     * @param index          the index we need cities to point to in order to restore network.
     */
    void unhideCityCol(int indexInCol);


    /* * * * * * * * * * * * * *    Logic to Build the Dancing Links Structure    * * * * * * * * */


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


    /* The following operators are nothing special. The fields of these types are simple integers.
     * We just need to define how to compare the fields in the structs. I also need helpful
     * printing information for the structs while debugging.
     */

    friend bool operator==(const cityItem& lhs, const cityItem& rhs);
    friend bool operator!=(const cityItem& lhs, const cityItem& rhs);
    friend bool operator==(const cityHeader& lhs, const cityHeader& rhs);
    friend bool operator!=(const cityHeader& lhs, const cityHeader& rhs);
    friend std::ostream& operator<<(std::ostream& os, const cityItem& city);
    friend std::ostream& operator<<(std::ostream& os, const cityHeader& city);
    friend std::ostream& operator<<(std::ostream&os, const std::vector<cityItem>& grid);
    friend std::ostream& operator<<(std::ostream&os, const std::vector<cityHeader>& grid);
    friend std::ostream& operator<<(std::ostream&os, const DisasterLinks& network);
    // I like to test the private internals of a class rather than just unit tests so add this here.
    ALLOW_TEST_ACCESS();
};


} // namespace DancingLinks


#endif // DISASTERLINKS_H
