/**
 * Author: Alexander G. Lopez
 * File: DisasterLinks.cpp
 * --------------------------
 * This file contains the implementation of Algorithm X via Dancing Links as applied to a Disaster
 * Planning problem. For more details on the thought process behind the implmenetation, please see
 * the DancingLinks.h file or the readme. Comments are detailed for my own understanding and because
 * some of the implementation is complicated, especially the building of the dancing links grid.
 */
#include <cmath>
#include <limits.h>
#include "DisasterLinks.h"

namespace DancingLinks {


/* * * * * * * * * * * * *    Free Functions for DancingLinks Namespace   * * * * * * * * * * * * */


bool hasOverlappingCover(DisasterLinks& links, int numSupplies, std::set<std::string>& selectedOptions) {
    return links.isDisasterReady(numSupplies, selectedOptions);
}

std::set<std::set<std::string>> getAllOverlappingCovers(DisasterLinks& links, int numSupplies) {
    return links.getAllDisasterConfigurations(numSupplies);
}


/* * * * * * * * * * * * *  Algorithm X via Dancing Links Implementation  * * * * * * * * * * * * */


bool DisasterLinks::isDisasterReady(int numSupplies, std::set<std::string>& suppliedCities) {
    if (numSupplies < 0) {
        error("Negative supply quantity is impossible.");
    }
    if (numItemsAndOptions_ == 0) {
        return true;
    }
    return isCovered(numSupplies, suppliedCities);
}

bool DisasterLinks::isCovered(int numSupplies, std::set<std::string>& suppliedCities) {
    if (table_[0].right == 0 && numSupplies >= 0) {
        return true;
    }
    if (numSupplies <= 0) {
        return false;
    }

    // Choose the city that appears the least across all sets because that will be hard to cover.
    int chosenIndex = chooseIsolatedCity();

    /* Try to cover this city by first supplying the most connected city nearby. Try cities with
     * successively fewer connections then if all else fails supply the isolated city itself.
     */
    for (int cur = grid_[chosenIndex].down; cur != chosenIndex; cur = grid_[cur].down) {

        std::string supplyLocation = coverCity(cur);

        if (isCovered(numSupplies - 1, suppliedCities)) {
            // Only add to the output if successful and be sure to cleanup in case it runs again.
            suppliedCities.insert(supplyLocation);
            uncoverCity(cur);
            return true;
        }

        // This cleanup is in case of failed choices. Try another starting supply location.
        uncoverCity(cur);
    }

    return false;
}

std::set<std::set<std::string>> DisasterLinks::getAllDisasterConfigurations(int numSupplies) {
    if (numSupplies < 0) {
        error("Negative supply count.");
    }

    std::set<std::string> suppliedCities {};
    std::set<std::set<std::string>> allConfigurations = {};
    fillConfigurations(numSupplies, suppliedCities, allConfigurations);
    return allConfigurations;
}

void DisasterLinks::fillConfigurations(int numSupplies,
                                       std::set<std::string>& suppliedCities,
                                       std::set<std::set<std::string>>& allConfigurations) {

    if (table_[0].right == 0 && numSupplies >= 0) {
        allConfigurations.insert(suppliedCities);
        return;
    }
    if (numSupplies <= 0) {
        return;
    }
    int chosenIndex = chooseIsolatedCity();

    for (int cur = grid_[chosenIndex].down; cur != chosenIndex; cur = grid_[cur].down) {

        std::string supplyLocation = coverCity(cur);
        suppliedCities.insert(supplyLocation);

        fillConfigurations(numSupplies - 1, suppliedCities, allConfigurations);

        suppliedCities.erase(supplyLocation);
        // This cleanup is in case of failed choices. Try another starting supply location.
        uncoverCity(cur);
    }
}

int DisasterLinks::chooseIsolatedCity() const {
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

std::string DisasterLinks::coverCity(int indexInOption) {
    /* Be sure to leave the row of the option we supply unchanged. Splice these cities out of all
     * other options in which they can be found above and below the current row.
     */
    int i = indexInOption;
    std::string result = "";
    do {
        int top = grid_[i].topOrLen;
        if (top <= 0) {
            /* We are always guaranteed to pass the spacer tile so we will collect the name of the
             * city we have chosen to supply to prove our algorithm chose correctly.
            */
            result = table_[std::abs(top)].name;
        } else {
            hideCityCol(i);
            table_[table_[top].left].right = table_[top].right;
            table_[table_[top].right].left = table_[top].left;
        }
        i = grid_[i].right;
    } while (i != indexInOption);

    return result;
}

void DisasterLinks::uncoverCity(int indexInOption) {
    /* To uncover a city we take the supplies away from the option in which we found this city. We
     * then must go up and down for every city covered by this supply location and put the cities
     * back in all the other sets. Original row was not altered so no other restoration necessary.
     */
    indexInOption = grid_[indexInOption].left;
    int i = indexInOption;
    do {
        int top = grid_[i].topOrLen;
        if (top > 0) {
            table_[table_[top].left].right = top;
            table_[table_[top].right].left = top;
            unhideCityCol(i);
        }
        i = grid_[i].left;
    } while (i != indexInOption);
}

void DisasterLinks::hideCityCol(int indexInCol) {
    for (int i = grid_[indexInCol].down; i != indexInCol; i = grid_[i].down) {
        cityItem cur = grid_[i];
        grid_[cur.right].left = cur.left;
        grid_[cur.left].right = cur.right;
    }
}

void DisasterLinks::unhideCityCol(int indexInCol) {
    for (int i = grid_[indexInCol].up; i != indexInCol; i = grid_[i].up) {
        cityItem cur = grid_[i];
        grid_[cur.right].left = i;
        grid_[cur.left].right = i;
    }
}


/* * * * * * * * * * *  Constructor and Building of Dancing Links Network   * * * * * * * * * * * */


DisasterLinks::DisasterLinks(const std::map<std::string, std::set<std::string>>& roadNetwork)
    : table_(),
      grid_(),
      numItemsAndOptions_(0) {

    // We will set this up for a reverse build of column links for a given item.
    std::unordered_map<std::string,int> columnBuilder = {};
    std::vector<std::pair<std::string,int>> connectionSizes = {};

    // We need to start preparing items in the grid immediately after the headers.
    initializeHeaders(roadNetwork, connectionSizes, columnBuilder);

    /* The second pass will fill in the columns and keep the headers and all elements appropriately
     * updated. We can hang on to helpful index info.
     */
    initializeItems(roadNetwork, connectionSizes, columnBuilder);
}

void DisasterLinks::initializeHeaders(const std::map<std::string, std::set<std::string>>& roadNetwork,
                                      std::vector<std::pair<std::string,int>>& connectionSizes,
                                      std::unordered_map<std::string,int>& columnBuilder) {
    table_.push_back({"", 0, 1});
    grid_.push_back({0,0,0,0,1});
    int index = 1;
    // The first pass will set up the name headers and the column headers in the two vectors.
    for (const auto& city : roadNetwork) {

        // Add one to the connection size because we will add a city to its own connections later.
        connectionSizes.push_back({city.first, roadNetwork.at(city.first).size() + 1});

        // We need to set up multiple columns, so begin tracking the previous item for a column.
        columnBuilder[city.first] = index;

        table_.push_back({city.first, index - 1, index + 1});
        table_[0].left++;
        grid_[0].left++;
        // Add the first headers for the item vector. They need count up and down.
        grid_.push_back({0, index, index, index - 1, index + 1});
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
    grid_[grid_.size() - 1].right = 0;
}

void DisasterLinks::initializeItems(const std::map<std::string, std::set<std::string>>& roadNetwork,
                                    const std::vector<std::pair<std::string,int>>& connectionSizes,
                                    std::unordered_map<std::string,int>& columnBuilder) {
    int previousSetSize = grid_.size();
    int index = grid_.size();

    for (const auto& [city, connectionSize] : connectionSizes) {
        // This algorithm includes a city in its own set of connections.
        std::set<std::string> connections = roadNetwork.at(city);
        connections.insert(city);
        int setSize = connections.size();

        /* We will know which supplying city option an item is in by the spacerTitle.
         * lookupTable[abs(-grid_[columnBuilder[city]].down)] will give us the name of the option
         * of that row. This accesses the last city in a column's down field to get the header.
         */
        grid_.push_back({-grid_[columnBuilder[city]].down,  // Negative index of city as option.
                         index - previousSetSize,           // First item in previous option
                         index + setSize,                   // Last item in current option
                         index,
                         index + 1});

        // Manage column pointers for items connected across options. Update index.
        index = initializeColumns(connections, columnBuilder, index);

        previousSetSize = setSize;
    }
    grid_.push_back({INT_MIN, index - previousSetSize, 0, index - 1, INT_MIN});
}

int DisasterLinks::initializeColumns(const std::set<std::string>& connections,
                                     std::unordered_map<std::string,int>& columnBuilder,
                                     int index) {
    int spacerIndex = index;
    for (const auto& c : connections) {
        // Circular lists give us access to header with down field of last city in a column.
        grid_[grid_[columnBuilder[c]].down].topOrLen++;
        index++;

        // A single item in a circular doubly linked list points to itself.
        grid_.push_back({grid_[columnBuilder[c]].down, index, index, index - 1, index + 1});

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
    /* Every option "row" is a left-right circular linked list. This is how we recursively cover
     * Cities by removing them as items only. A city that is adjacent to a supplied city may still
     * receive supplies to cover other cities as an option. This is how we achieve that. This is
     * a significant variation from Knuth's DLX.
     */
    grid_[index].right = spacerIndex;
    grid_[spacerIndex].left = index;
    return ++index;
}

} // namespace DancingLinks
