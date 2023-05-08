/**
 * Author: Alexander G. Lopez
 * File DisasterTags.cpp
 * --------------------------
 * This file contains the implementation for Algorithm X via Dancing Links with a new method I'm
 * calling Supply Tags. For more information, see the mini-writeup in the DancingLinks.h file or the
 * detailed write up in the README.md.
 */
#include <cmath>
#include <climits>
#include "DisasterTags.h"

namespace DancingLinks {


/* * * * * * * * * * * * *    Free Functions for DancingLinks Namespace   * * * * * * * * * * * * */


bool hasOverlappingCover(DisasterTags& links, int numSupplies,
                           std::set<std::string>& selectedOptions) {
    return links.hasDisasterCoverage(numSupplies, selectedOptions);
}

std::set<std::set<std::string>> getAllOverlappingCovers(DisasterTags& links, int numSupplies) {
    return links.getAllDisasterConfigurations(numSupplies);
}


/* * * * * * * * * * * * *  Algorithm X via Dancing Links with Depth Tags * * * * * * * * * * * * */


bool DisasterTags::hasDisasterCoverage(int numSupplies, std::set<std::string>& supplyLocations) {
    if (numSupplies < 0) {
        error("negative supplies");
    }
    if (numItemsAndOptions_ == 0) {
        return true;
    }
    return isDLXCovered(numSupplies, supplyLocations);
}

bool DisasterTags::isDLXCovered(int numSupplies, std::set<std::string>& supplyLocations) {
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
            supplyLocations.insert(supplyLocation);
            uncoverCity(cur);
            return true;
        }

        // We will know which cities to uncover thanks to which we tagged on the way down.
        uncoverCity(cur);
    }
    return false;
}

std::set<std::set<std::string>> DisasterTags::getAllDisasterConfigurations(int numSupplies) {
    if (numSupplies < 0) {
        error("Negative supply count.");
    }

    std::set<std::string> suppliedCities {};
    std::set<std::set<std::string>> allConfigurations = {};
    fillConfigurations(numSupplies, suppliedCities, allConfigurations);
    return allConfigurations;
}

void DisasterTags::fillConfigurations(int numSupplies,
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

        std::string supplyLocation = coverCity(cur, numSupplies);
        suppliedCities.insert(supplyLocation);

        fillConfigurations(numSupplies - 1, suppliedCities, allConfigurations);

        suppliedCities.erase(supplyLocation);
        uncoverCity(cur);
    }

}

int DisasterTags::chooseIsolatedCity() const {
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


DisasterTags::DisasterTags(const std::map<std::string, std::set<std::string>>& roadNetwork)
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

void DisasterTags::initializeHeaders(const std::map<std::string, std::set<std::string>>& roadNetwork,
                                     std::vector<std::pair<std::string,int>>& connectionSizes,
                                     std::unordered_map<std::string,int>& columnBuilder) {
    table_.push_back({"", 0, 1});
    grid_.push_back({0,0,0,0});
    int index = 1;
    // The first pass will set up the name headers and the column headers in the two vectors.
    for (const auto& city : roadNetwork) {

        // Add one to the connection size because we will add a city to its own connections later.
        connectionSizes.push_back({city.first, roadNetwork.at(city.first).size() + 1});

        // We need to set up multiple columns, so begin tracking the previous item for a column.
        columnBuilder[city.first] = index;

        table_.push_back({city.first, index - 1, index + 1});
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

void DisasterTags::initializeItems(const std::map<std::string, std::set<std::string>>& roadNetwork,
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
                         0});                               // Supply number tag.

        // Manage column pointers for items connected across options. Update index.
        index = initializeColumns(connections, columnBuilder, index);

        previousSetSize = connections.size();
    }
    grid_.push_back({INT_MIN, index - previousSetSize, INT_MIN,0});
}

int DisasterTags::initializeColumns(const std::set<std::string>& connections,
                                     std::unordered_map<std::string,int>& columnBuilder,
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

} // namespace DancingLinks
