/**
 * Author: Alexander G. Lopez
 * File: DisasterLinks.h
 * -------------------------
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
 * most connections are at the top and will be explored first. However, do not mistake this for
 * a greedy approach. We will now discuss the selection process.
 *
 * We must distribute two supplies. Select the city to cover that is the most isolated in the
 * network. It will have the lowest number of appearances across all sets. The (*) marks our
 * selection.
 *
 *            *
 *            2  4  1  4  2  2
 *            A  B  C  D  E  F
 *         B     1     1  1  1
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
 *           0
 *         A
 *         C
 *         E
 *         F
 *
 * Thus our world is empty and the problem is solved by supplying cities B and D when given two
 * supplies. If we were given one supply we would not have been able cover this network.
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
#pragma once
#ifndef DISASTERLINKS_H
#define DISASTERLINKS_H
#include <limits.h>
#include "GUI/SimpleTest.h"
#include "map.h"
#include "hashmap.h"
#include "set.h"

#define NOT_PROCESSED INT_MIN

class DisasterLinks {

public:


    /* The cityItem will carry most of the logic of our problem. These nodes are in the grid we
     * set up in order for the links to "dance" as we recurse and leave the data structure in place.
     */
    typedef struct cityItem {
        /* If this item is a column header, this is the number of items in a colum.
         * If this item is in the grid, this is the index of the header for an item.
         */
        int topOrLen;
        // We traverse through options to cover one item with the up down field.
        int up;
        int down;
        // We cut an item within an option out of the world with the left right field to recurse.
        int left;
        int right;
    }cityItem;

    /* The cityHeader helps us track what items still need to be covered. We also use this as a
     * lookup table for the names of the options in which we find an item. This option is the city
     * we have given supplies to.
     */
    typedef struct cityHeader {
        std::string name;
        int left;
        int right;
    }cityHeader;


    /* This Network is our world. These data structures will remain in place during the recursive
     * algorithm. We do not need to make any copies, only modify the indices of the cityItems in
     * the grid in order to find the solution. All backtracking can be accomplished by only
     * modifying the fields of surrounding nodes and leaving the node in question unchanged.
     * This is the core concept behind Knuth's Algorithm X via Dancing Links which I have adapted
     * to this problem.
     */
    typedef struct Network {
        Vector<cityHeader> lookupTable;
        Vector<cityItem> grid;
        HashMap<std::string,int> headerIndexMap;
        /* In this application, the number of colums equals the number of rows. Cities are both
         * items that need to be covered and cities that can receive supplies.
         */
        int numItemsAndOptions;
    }Network;

    /**
     * @brief DisasterLinks  a custom constructor for this class that can turn a Map representation
     *                       of a transportation grid into a vector grid prepared for exact cover
     *                       search via dancing links.
     * @param roadNetwork    the transportation grid passed in via map form.
     */
    DisasterLinks(const Map<std::string, Set<std::string>>& roadNetwork);

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
    bool isDisasterReady(int numSupplies, Set<std::string>& suppliedCities);


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

    friend std::ostream& operator<<(std::ostream&os, const Vector<cityItem>& grid);

    friend std::ostream& operator<<(std::ostream&os, const DisasterLinks& network);


private:

    /* An instance of this class will produce a Network item that helps us solve the problem. It
     * will stay in place until the instance is destructed. So you could, for example, run a loop
     * with decreasing supply amounts to test against this network and it will always restore
     * itself between tests.
     */
    Network dlx;


    /* * * * * * * * * * * * * *       Modified Algorithm X via Dancing Links     * * * * * * * * */


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
    bool isCovered(int numSupplies, Set<std::string>& suppliedCities);

    /**
     * @brief chooseOption  selects a city we are trying to cover either by giving it supplies or
     *                      covering an adjacent neighbor. The selection uses the following
     *                      heuristic:
     *                          - Select the most isolated city so far.
     *                          - We must cover this city so select an adjacent city with the
     *                            most connections and try that first.
     *                          - If that fails we try the next adjacent city with most connections.
     *                          - Finally, try to supply the actual city in question, not neighbors.
     * @return              the index of the city we are selecting to attempt to cover.
     */
    int chooseOption();

    /**
     * @brief coverCity  covers a city with the option below the specified index. A city in question
     *                   may be covered by supplying a neighbor or supplying the city itself.
     * @param index      the index we start at for an item. We select the option beneath this index.
     * @return           the string name of the option we used to cover a city, neighbor or city.
     */
    std::string coverCity(int index);

    /**
     * @brief uncoverCity  uncovers a city if that choice of option did not lead to a covered
     *                     network. Uncovers the same option that was selected for coverage if given
     *                     the same index.
     * @param index        the index of the item we covered with the option below the index.
     */
    void uncoverCity(int index);

    /**
     * @brief hideItemCol  when we supply an option it covers itself and connected cities. We must
     *                     remove these cities from any other sets that contain them to make them
     *                     disappear from the world as uncovered cities. However, we keep them as
     *                     available cities to supply.
     * @param start        the city we start at in a row. We traverse downward to snip city out.
     */
    void hideItemCol(DisasterLinks::cityItem& start);

    /**
     * @brief unhideItemCol  when an option fails, we must put the cities it covers back into all
     *                       the sets to which they belong. This puts the cities back in network.
     * @param start          the city in the option we start at. We traverse upward to unhide.
     * @param index          the index we need cities to point to in order to restore network.
     */
    void unhideItemCol(DisasterLinks::cityItem& start, int index);


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


    // I like to test the private internals of a class rather than just unit tests so add this here.
    ALLOW_TEST_ACCESS();
};

#endif // DISASTERLINKS_H
