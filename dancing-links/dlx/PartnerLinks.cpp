/**
 * Author: Alexander G. Lopez
 * File: PartnerLinks.cpp
 * -------------------------
 * This file contains the implementation of the PartnerLinks class. Specifically it implements
 * Algorithm X via Dancing Links as outlined by Donald Knuth. There is modification to this
 * algorithm as necessary in order to solve two graph problems: Perfect Matching and Max Weight
 * Matching. For more details on the concepts behind the algorithm see the mini write up in the
 * PartnerLinks.h file or the full write up in the README.md for the repository.
 */
#include "PartnerLinks.h"


/* * * * * * * * * * * * *  Perfect Matching Algorithm X via Dancing Links  * * * * * * * * * * * */


/**
 * @brief hasPerfectLinks  determines if an instance of a PartnerLinks matrix can solve the
 *                         Perfect Match problem. A perfect matching is when every person is
 *                         partnered with someone.
 * @param pairs            the output parameter that shows the first perfect matching we found.
 * @return                 true if there is a perfect matching false if not.
 */
bool PartnerLinks::hasPerfectLinks(Set<Pair>& pairs) {
    // Mathematically impossible perfect links no work necessary.
    if (dlx.hasSingleton || dlx.numPeople % 2 != 0) {
        return false;
    }
    return isPerfectMatching(pairs);
}

/**
 * @brief isPerfectMatching  finds the first available Perfect Matching for a network. The set
 *                           is empty if no matching is found.
 * @param pairs              the output parameter of the found matching.
 * @return                   true if the matching is found, false if not.
 */
bool PartnerLinks::isPerfectMatching(Set<Pair>& pairs) {
    if (dlx.lookupTable[0].right == 0) {
        return true;
    }
    // If our previous pairings led to someone that can no longer be reached stop recursion.
    int chosenPerson = choosePerson();
    if (chosenPerson == -1) {
        return false;
    }

    for (int cur = chosenPerson; dlx.links[cur].down != chosenPerson; cur = dlx.links[cur].down) {

        Pair match = coverPairing(cur);

        if (isPerfectMatching(pairs)) {
            // Cleanup the data structure in case we are asked again. Maybe uneccessary.
            pairs.add(match);
            uncoverPairing(cur);
            return true;
        }
        // Selecting to partner chosenPerson with this option did not work out. Retry other option.
        uncoverPairing(cur);
    }
    return false;
}

/**
 * @brief choosePerson  chooses a person for the Perfect Matching algorithm. It will simply
 *                      select the next person avaialable with no advanced heuristics. However,
 *                      it first checks if anyone has been isolated due to previous parings.
 *                      We willl then know we should stop recursion because someone is alone and
 *                      has no pairing possibilities.
 * @return              the index of the next person to pair or -1 if someone is alone.
 */
int PartnerLinks::choosePerson() {
    int head = 0;
    for (int cur = dlx.lookupTable[0].right; cur != head; cur = dlx.lookupTable[cur].right) {
        // Someone has become inaccessible due to other matches.
        if (dlx.links[cur].topOrLen == 0) {
            return -1;
        }
    }
    return dlx.lookupTable[0].right;
}

/**
 * @brief coverPairing  when we cover a pairing in a Perfect Matching we report back the
 *                      partnership as a Pair. This is helpful for proving the recursive
 *                      selections are correct. This selects the option beneath the index given.
 *                      Covering a pair means that both people will dissapear from all other
 *                      partnerships they could have matched with, eliminating those options.
 * @param index         the index of the pair we want to cover. Chooses option below this.
 * @return              the pair we have created by selecting this option.
 */
Pair PartnerLinks::coverPairing(int index) {

    // We could start on a header so always step down.
    index = dlx.links[index].down;


    /* We now must cover the two people in this option in the lookup table. Then go through all
     * other options and eliminate the other pairings in which each appears because they are paired
     * off and therefore no longer accessible to other people that want to pair with them.
     */

    personName p1 = dlx.lookupTable[dlx.links[index].topOrLen];
    dlx.lookupTable[p1.right].left = p1.left;
    dlx.lookupTable[p1.left].right = p1.right;

    // p1 needs to dissapear from all other pairings.
    hidePersonPairings(dlx.links[index], index);

    // In case I ever apply a selection heuristic, partner might not be to the right.
    toPairIndex(index);

    personName p2 = dlx.lookupTable[dlx.links[index].topOrLen];
    dlx.lookupTable[p2.right].left = p2.left;
    dlx.lookupTable[p2.left].right = p2.right;

    // p2 needs to dissapear from all other pairings.
    hidePersonPairings(dlx.links[index], index);

    // This is reported as an output parameter for the pairings we chose.
    return {p1.name, p2.name};
}

/**
 * @brief uncoverPairing  uncovers a pairing that was hidden in Perfect Matching or Max Weight
 *                        Matching. The uncovering process is identical across both algorithms.
 *                        Be sure to provide the same index as the option you covered.
 * @param index           the same index as the index that was covered.
 */
void PartnerLinks::uncoverPairing(int index) {

    index = dlx.links[index].down;

    personName p1 = dlx.lookupTable[dlx.links[index].topOrLen];
    dlx.lookupTable[p1.left].right = dlx.links[index].topOrLen;
    dlx.lookupTable[p1.right].left = dlx.links[index].topOrLen;

    unhidePersonPairings(dlx.links[index], index);

    toPairIndex(index);


    personName p2 = dlx.lookupTable[dlx.links[index].topOrLen];
    dlx.lookupTable[p2.left].right = dlx.links[index].topOrLen;
    dlx.lookupTable[p2.right].left = dlx.links[index].topOrLen;

    unhidePersonPairings(dlx.links[index], index);

}

/**
 * @brief hidePersonPairings  hides other options that include the specified person. This only
 *                            hides pairings including a single person. If you want to hide both
 *                            people in a pair, you must use this on both people.
 * @param start               the starting node of the person we are hide in the selected option
 * @param index               the index of the person we are hiding in the selected option.
 */
void PartnerLinks::hidePersonPairings(personLink& start, int index) {
    personLink nextPairing = start;
    index = start.down;
    while ((nextPairing = dlx.links[nextPairing.down]) != start) {
        // We may need this guard to prevent splicing while on a column header.
        if (index > dlx.lookupTable.size()) {

            // In case the other partner is to the left, just decrement index to go left.
            toPairIndex(index);

            personLink cur = dlx.links[index];

            dlx.links[cur.up].down = cur.down;
            dlx.links[cur.down].up = cur.up;
            dlx.links[cur.topOrLen].topOrLen--;
        }
        index = nextPairing.down;
    }
}

/**
 * @brief unhidePersonPairings  undoes the work of hidePersonPairings if given the same start
 *                              and index.
 * @param start                 the node of the person we unhide in the selected option.
 * @param index                 the index of the person we are unhiding in the selected option.
 */
void PartnerLinks::unhidePersonPairings(personLink& start, int index) {
    personLink nextPairing = start;
    // The order does not truly matter but I distinguish this from hide by going upwards.
    index = start.up;
    while ((nextPairing = dlx.links[nextPairing.up]) != start) {
        if (index > dlx.lookupTable.size()) {

            toPairIndex(index);

            personLink cur = dlx.links[index];

            dlx.links[cur.up].down = index;
            dlx.links[cur.down].up = index;
            dlx.links[cur.topOrLen].topOrLen++;
        }
        index = nextPairing.up;
    }

}

/**
 * @brief toPairIndex  helper function to increment the index to the next partner. We might
 *                     need to move left or right.
 * @param index        the index we take by reference to advance.
 */
inline void PartnerLinks::toPairIndex(int& index) {
    // There are only ever two people in an option so this is a safe increment/decrement.
    if (dlx.links[++index].topOrLen <= 0) {
        index -= 2;
    }
}


/* * * * * * * * * * * * *  Weighted Matching Algorithm X via Dancing Links  * * ** * * * * * * * */


/**
 * @brief getMaxWeightMatching  determines the Max Weight Matching of a PartnerLinks matrix. A
 *                              Max Weight Matching is the greatest sum of edge weights we can
 *                              acheive by partnering up people in a network.
 * @return                      the set representing the Max Weight Matching that we found.
 */
Set<Pair> PartnerLinks::getMaxWeightMatching() {
    if (!dlx.isWeighted) {
        error("Asking for max weight matching of a graph with no weight information provided.\n"
              "For weighted graphs provide a Map<string,Map<string,int>> representing a person\n"
              "and the weights of their preferred connections to the constructor.");
    }
    /* In the spirit of "no copy" recursion by Knuth, we can just fill and remove from one set and
     * record the best snapshot of pairings in the winner set. Have to profile to see if this faster
     * than normal recursion with copies of sets.
     */
    std::pair<int,Set<Pair>> soFar = {};
    std::pair<int,Set<Pair>> winner = {};
    fillWeights(soFar, winner);
    return winner.second;
}

/**
 * @brief fillWeights  recusively finds the maximum weight pairings possible given a dancing
 *                     links network with weighted partners. Uses the soFar set to store all
 *                     possible pairing combinations while the winner output parameter tracks
 *                     snapshots of the best weight and pairs found so far.
 * @param soFar        the pair of weight and pairs we fill with every possible pairing.
 * @param winner       the pair of weight and pairs that records the best weight found.
 */
void PartnerLinks::fillWeights(std::pair<int,Set<Pair>>& soFar, std::pair<int,Set<Pair>>& winner) {
    if (dlx.lookupTable[0].right == 0) {
        return;
    }

    int chosen = chooseWeightedPerson();
    if (chosen == -1) {
        return;
    }
    // Explore every possibility without this person to see if a heavier matching exists without.
    coverPerson(chosen);
    fillWeights(soFar, winner);
    uncoverPerson(chosen);

    // Now loop through every possible option for every combination of people available.
    for (int cur = chosen; dlx.links[cur].down != chosen; cur = dlx.links[cur].down) {

        // Our cover operation is able to pick up the weight and names of pair in a O(1) operation.
        std::pair<int,Pair> match = coverWeightedPair(cur);
        soFar.first += match.first;
        soFar.second += match.second;

        // Go explore every weight that matching this pair produces
        fillWeights(soFar, winner);

        // The winner pair will copy in the weight and Set if its the best so far.
        if (soFar.first > winner.first) {
            winner = soFar;
        }

        // Prepare to explore the next options. Cleanup links and remove previous choice from pair.
        uncoverPairing(cur);
        soFar.first -= match.first;
        soFar.second -= match.second;
    }
}

/**
 * @brief chooseWeightedPerson  choosing a person in Max Weight matching is different than
 *                              Perfect Matching. It is ok if someone cannot be reached by
 *                              partners. We will select next available person who has partners
 *                              available. If everyone that has not been matched has no possible
 *                              connections, that is ok, but we will return -1 to indicate this.
 * @return                      the index of the next person to pair or -1 if no pairs remain.
 */
int PartnerLinks::chooseWeightedPerson() {
    int head = 0;
    for (int cur = dlx.lookupTable[0].right; cur != head; cur = dlx.lookupTable[cur].right) {
        // Take the first available person.
        if (dlx.links[cur].topOrLen != 0) {
            return cur;
        }
    }
    // Every person is alone so no more weights to explore in this recursive branch.
    return -1;
}


/**
 * @brief coverPerson  to generate all possible pairings in any pairing algorithm, we need to
 *                     include every person in future possible pairings and exclude them. To
 *                     exclude a person, we will cover only that person. Instead of eliminating
 *                     every option that includes both people in a Pair, we only eliminate
 *                     other appearances of this individual in other pairings. It is a subtle
 *                     but important difference from covering a pairing.
 * @param index        the index of the person we cover. Chooses option below this index.
 */
void PartnerLinks::coverPerson(int index) {
    index = dlx.links[index].down;

    personName p1 = dlx.lookupTable[dlx.links[index].topOrLen];
    dlx.lookupTable[p1.right].left = p1.left;
    dlx.lookupTable[p1.left].right = p1.right;

    // Only hide pairings for this person.
    hidePersonPairings(dlx.links[index], index);

    toPairIndex(index);
    // Partner will only disapear in this instance of the pairing, not all other instances.
    personLink cur = dlx.links[index];
    dlx.links[cur.up].down = cur.down;
    dlx.links[cur.down].up = cur.up;
    dlx.links[cur.topOrLen].topOrLen--;
}

/**
 * @brief uncoverPerson  undoes the work of covering a person, reinstating all possible pairings
 *                       that include this person. Will undo the same option chosen in
 *                       coverPerson() if given the same index.
 * @param index          the index of the person to uncover. Chooses option below this index.
 */
void PartnerLinks::uncoverPerson(int index) {
    index = dlx.links[index].down;

    personName p1 = dlx.lookupTable[dlx.links[index].topOrLen];
    dlx.lookupTable[p1.left].right = dlx.links[index].topOrLen;
    dlx.lookupTable[p1.right].left = dlx.links[index].topOrLen;

    unhidePersonPairings(dlx.links[index], index);

    toPairIndex(index);
    personLink cur = dlx.links[index];
    dlx.links[cur.up].down = index;
    dlx.links[cur.down].up = index;
    dlx.links[cur.topOrLen].topOrLen++;
}

/**
 * @brief coverWeightedPair  when we cover a weighted pair in Max Weight Matching we should
 *                           report back the weight of the pair and the names of the people in
 *                           the Pair. This is helpful for proof of correct choices. Covering a
 *                           Pair means that both people disappear from all other possible
 *                           pairings, thus eliminating those options.
 * @param index              the index of the weighted pair we cover. Chooses option below this.
 * @return                   an std::pair of the weight of the pair and their names.
 */
std::pair<int,Pair> PartnerLinks::coverWeightedPair(int index) {
    index = dlx.links[index].down;

    personName p1 = dlx.lookupTable[dlx.links[index].topOrLen];
    dlx.lookupTable[p1.right].left = p1.left;
    dlx.lookupTable[p1.left].right = p1.right;

    // p1 needs to dissapear from all other pairings.
    hidePersonPairings(dlx.links[index], index);


    // We can pick up the weight for this pairing in a O(1) sweep to report back.
    std::pair<int,Pair> result = {};
    if (dlx.links[index + 1].topOrLen < 0) {
        result.first = std::abs(dlx.links[index - 2].topOrLen);
        index--;
    } else {
        // p2 needs to dissapear from all other pairings.
        hidePersonPairings(dlx.links[index + 1], index + 1);
        result.first = std::abs(dlx.links[index - 1].topOrLen);
        index++;
    }

    personName p2 = dlx.lookupTable[dlx.links[index].topOrLen];
    dlx.lookupTable[p2.right].left = p2.left;
    dlx.lookupTable[p2.left].right = p2.right;

    result.second = {p1.name,p2.name};
    return result;
}


/* * * * * * * * * * * * * * *   Constructor to Build the Networks  * * * * * * * * * * * * * * * */


/**
 * @brief PartnerLinks   the constructor for a world intended to check for Perfect Matching.
 *                       Provide a map of the person, and the set of people they are willing to
 *                       pair with. This will then build a dancing link matrix in order to solve
 *                       future queries.
 * @param possibleLinks  the map of people and partners they are willing to work with.
 */
PartnerLinks::PartnerLinks(const Map<std::string, Set<std::string>>& possibleLinks) {

    dlx.numPairings = 0;
    dlx.numPeople = 0;
    dlx.hasSingleton = false;
    dlx.isWeighted = false;

    HashMap<std::string, int> columnBuilder = {};

    initializeHeaders(possibleLinks, columnBuilder);

    // Begin building the rows with the negative spacer tiles and the subsequent columns.
    int index = dlx.links.size();
    int spacerTitle = -1;

    Set<Pair> seenPairs = {};
    for (const auto& p : possibleLinks) {

        const Set<std::string>& preferences = possibleLinks[p];
        if (preferences.isEmpty()) {
            dlx.hasSingleton = true;
        }
        setPerfectPairs(p, preferences, columnBuilder, seenPairs, index, spacerTitle);
    }

    dlx.links.add({INT_MIN, index - 2, INT_MIN});

}

/**
 * @brief PartnerLinks   the constructor for a world intended to check for Max Weight Matching.
 *                       Provide a map of the person, and the map of the partners they are
 *                       willing to pair with and the weights of each partnership. If a negative
 *                       weight is given for a partnership, we treat that partnership as if it
 *                       does not exist.
 * @param possibleLinks  the map of people and map of partners and weights.
 */
PartnerLinks::PartnerLinks(const Map<std::string, Map<std::string, int>>& possibleLinks) {
    dlx.numPairings = 0;
    dlx.numPeople = 0;
    dlx.hasSingleton = false;
    dlx.isWeighted = true;

    HashMap<std::string, int> columnBuilder = {};

    initializeHeaders(possibleLinks, columnBuilder);

    // Begin building the rows with the negative spacer tiles and the subsequent columns.
    int index = dlx.links.size();

    Set<Pair> seenPairs = {};
    for (const auto& p : possibleLinks) {

        const Map<std::string,int>& preferences = possibleLinks[p];
        if (preferences.isEmpty()) {
            dlx.hasSingleton = true;
        }
        setWeightedPairs(p, preferences, columnBuilder, seenPairs, index);
    }
    dlx.links.add({INT_MIN, index - 2, INT_MIN});
}

/**
 * @brief initializeHeaders  initializes the headers of the dancing link data structure if
 *                           given a map of pairs with no weights.
 * @param possibleLinks      the map of unweighted connections for every person in the graph.
 * @param columnBuilder      the helping data structure we use to build columns in one array.
 */
void PartnerLinks::initializeHeaders(const Map<std::string, Set<std::string>>& possibleLinks,
                                     HashMap<std::string,int>& columnBuilder) {
    // Set up the headers first. Lookup table and first N headers in links.
    dlx.lookupTable.add({"", 0, 1});
    dlx.links.add({});
    int index = 1;
    for (const auto& p : possibleLinks) {

        columnBuilder[p] = index;

        dlx.lookupTable.add({p, index - 1, index + 1});
        dlx.lookupTable[0].left++;
        // Add the first headers for the item vector. They need count up and down.
        dlx.links.add({0, index, index});

        dlx.numPeople++;
        index++;
    }
    dlx.lookupTable[dlx.lookupTable.size() - 1].right = 0;
}

/**
 * @brief initializeHeaders  initializes the headers of the dancing link data structure if
 *                           given a map of pairs with weights. Negative weights are ignored
 *                           and no pairing is made for that negative match.
 * @param possibleLinks      the map of weighted matches for every person in the graph.
 * @param columnBuilder      the helping data structure we use to build columns in one array.
 */
void PartnerLinks::initializeHeaders(const Map<std::string, Map<std::string,int>>& possibleLinks,
                                     HashMap<std::string,int>& columnBuilder) {
    // Set up the headers first. Lookup table and first N headers in links.
    dlx.lookupTable.add({"", 0, 1});
    dlx.links.add({});
    int index = 1;
    for (const auto& p : possibleLinks) {

        columnBuilder[p] = index;

        dlx.lookupTable.add({p, index - 1, index + 1});
        dlx.lookupTable[0].left++;
        // Add the first headers for the item vector. They need count up and down.
        dlx.links.add({0, index, index});

        dlx.numPeople++;
        index++;
    }
    dlx.lookupTable[dlx.lookupTable.size() - 1].right = 0;
}

/**
 * @brief setPerfectPairs  creates the internal rows and columns of the dancing links data
 *                         structure. Perfect pairs do not have any weight information and
 *                         options will simply be given a number from 1 to N number of matches.
 * @param person           the person who will set all possible matches for.
 * @param personPairs      the set of all people this person is willing to pair with.
 * @param columnBuilder    the structure we use to track and build all columns in one array.
 * @param seenPairs        helper set to keep pairings unique and bidirectional.
 * @param index            we advance the index as an output parameter.
 * @param spacerTitle      we advance the spacerTitle to number each option.
 */
void PartnerLinks::setPerfectPairs(const std::string& person,
                                   const Set<std::string>& preferences,
                                   HashMap<std::string,int>& columnBuilder,
                                   Set<Pair>& seenPairs,
                                   int& index,
                                   int& spacerTitle) {
    for (const auto& pref : preferences) {
        Pair newPair = {person, pref};

        if (!seenPairs.contains(newPair)) {
            dlx.numPairings++;
            // Update the count for this column.
            dlx.links.add({spacerTitle,     // Negative to mark spacer.
                           index - 2,       // First item in previous option
                           index + 2});     // Last item in current option
            index++;
            std::string sortedFirst = newPair.first();
            dlx.links.add({dlx.links[columnBuilder[sortedFirst]].down,index, index});

            // We can always access the column header with down field of last item.
            dlx.links[dlx.links[columnBuilder[sortedFirst]].down].topOrLen++;
            dlx.links[dlx.links[columnBuilder[sortedFirst]].down].up = index;
            // The current node is the new tail in a vertical circular linked list for an item.
            dlx.links[index].up = columnBuilder[sortedFirst];
            dlx.links[index].down = dlx.links[columnBuilder[sortedFirst]].down;
            // Update the old tail to reflect the new addition of an item in its option.
            dlx.links[columnBuilder[sortedFirst]].down = index;
            // Similar to a previous/current coding pattern but in an above/below column.
            columnBuilder[sortedFirst] = index;

            // Repeat the process. We only ever have two items in an option.
            index++;
            std::string sortedSecond = newPair.second();
            dlx.links.add({dlx.links[columnBuilder[sortedSecond]].down, index, index});
            dlx.links[dlx.links[columnBuilder[sortedSecond]].down].topOrLen++;
            dlx.links[dlx.links[columnBuilder[sortedSecond]].down].up = index;
            dlx.links[index].up = columnBuilder[sortedSecond];
            dlx.links[index].down = dlx.links[columnBuilder[sortedSecond]].down;
            dlx.links[columnBuilder[sortedSecond]].down = index;
            columnBuilder[sortedSecond] = index;

            // Pairings are bidirectional but might appear multiple times in Map. Track here.
            seenPairs.add(newPair);
            index++;
            spacerTitle--;
        }
    }
}

/**
 * @brief setWeightedPairs  creates the internal rows and columns of the dancing links data
 *                          structure. Weighted Pairs have weights for each partnership that
 *                          are placed as negative topOrLen fields in the spacer for each row.
 *                          We can then get the weight and names of any partnership while
 *                          recursing easily. Negative weights are ignored and no pairing made.
 * @param person            the person who's weighted matches we will set.
 * @param personPairs       the people this person can pair with and the weight of those pairs.
 * @param columnBuilder     the structure we use to track and build all columns in one array.
 * @param seenPairs         helper set to keep pairings unique and bidirectional.
 * @param index             we advance the index as an output parameter.
 */
void PartnerLinks::setWeightedPairs(const std::string& person,
                                    const Map<std::string,int>& preferences,
                                    HashMap<std::string,int>& columnBuilder,
                                    Set<Pair>& seenPairs,
                                    int& index) {
    for (const auto& pref : preferences) {
        Pair newPair = {person, pref};

        if (!seenPairs.contains(newPair) && preferences[pref] >= 0) {
            dlx.numPairings++;
            // Weight is negative so we know we are on a spacer tile when found.
            dlx.links.add({-preferences[pref],     // Negative weight of the partnership
                           index - 2,              // First item in previous option
                           index + 2});            // Last item in current option
            index++;
            std::string sortedFirst = newPair.first();
            dlx.links.add({dlx.links[columnBuilder[sortedFirst]].down,index, index});

            // We can always access the column header with down field of last item.
            dlx.links[dlx.links[columnBuilder[sortedFirst]].down].topOrLen++;
            dlx.links[dlx.links[columnBuilder[sortedFirst]].down].up = index;
            // The current node is new tail in a vertical circular linked list for an item.
            dlx.links[index].up = columnBuilder[sortedFirst];
            dlx.links[index].down = dlx.links[columnBuilder[sortedFirst]].down;
            // Update the old tail to reflect the new addition of an item in its option.
            dlx.links[columnBuilder[sortedFirst]].down = index;
            // Similar to a previous/current coding pattern but in an above/below column.
            columnBuilder[sortedFirst] = index;

            // Repeat the process. We only ever have two items in an option.
            index++;
            std::string sortedSecond = newPair.second();
            dlx.links.add({dlx.links[columnBuilder[sortedSecond]].down, index, index});
            dlx.links[dlx.links[columnBuilder[sortedSecond]].down].topOrLen++;
            dlx.links[dlx.links[columnBuilder[sortedSecond]].down].up = index;
            dlx.links[index].up = columnBuilder[sortedSecond];
            dlx.links[index].down = dlx.links[columnBuilder[sortedSecond]].down;
            dlx.links[columnBuilder[sortedSecond]].down = index;
            columnBuilder[sortedSecond] = index;

            // Because all pairings are bidirectional, they should only apear once as options.
            seenPairs.add(newPair);
            index++;
        }
    }
}


/* * * * * * * * * * * * * * * *   Overloaded Operators for Debugging   * * * * * * * * * * * * * */


bool operator==(const PartnerLinks::personLink& lhs, const PartnerLinks::personLink& rhs) {
    return lhs.topOrLen == rhs.topOrLen && lhs.up == rhs.up && lhs.down == rhs.down;
}

bool operator!=(const PartnerLinks::personLink& lhs, const PartnerLinks::personLink& rhs) {
    return !(lhs == rhs);
}

bool operator==(const PartnerLinks::personName& lhs, const PartnerLinks::personName& rhs) {
    return lhs.name == rhs.name && lhs.left == rhs.left && lhs.right == rhs.right;
}

bool operator!=(const PartnerLinks::personName& lhs, const PartnerLinks::personName& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, const PartnerLinks::personLink& person) {
    os << "{ topOrLen: " << person.topOrLen
       << ", up: " << person.up << ", down: " << person.down << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const PartnerLinks::personName& name) {
    os << "{ name: " << name.name << ", left: " << name.left << ", right: " << name.right << " }";
    return os;
}

std::ostream& operator<<(std::ostream&os, const Vector<PartnerLinks::personLink>& links) {
    os << "DLX ARRAY" << std::endl;
    for (const auto& item : links) {
        if (item.topOrLen < 0) {
            os << std::endl;
        }
        os << "{" << item.topOrLen << "," << item.up << "," << item.down << "}";
    }
    os << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream&os, const PartnerLinks& links) {
    os << "LOOKUP ARRAY" << std::endl;
    for (const auto& header : links.dlx.lookupTable) {
        os << "{\"" << header.name << "\"," << header.left << "," << header.right << "},";
    }
    os << std::endl;
    os << "DLX ARRAY" << std::endl;
    for (const auto& item : links.dlx.links) {
        if (item.topOrLen < 0) {
            os << std::endl;
        }
        os << "{" << item.topOrLen << "," << item.up << "," << item.down << "},";
    }
    os << std::endl;
    return os;
}


/* * * * * * * * * * * * * * * *      Test Cases Below this Point       * * * * * * * * * * * * * */


/* * * * * * * * * * * * * * * *            WEIGHTED MATCHING           * * * * * * * * * * * * * */


namespace  {

    /* Utility to go from a list of triples to a world. */
       struct WeightedLink {
           std::string from;
           std::string to;
           int cost;
       };
       Map<std::string, Map<std::string, int>> fromWeightedLinks(const Vector<WeightedLink>& links) {
           Map<std::string, Map<std::string, int>> result;
           for (const auto& link: links) {
               result[link.from][link.to] = link.cost;
               result[link.to][link.from] = link.cost;
           }
           return result;
       }

}


/* * * * * * * * * * * * * * * *             Initialization             * * * * * * * * * * * * * */


STUDENT_TEST("Weighted matching initializes straight line correctly.") {
    /*
     *          1    2     3     4      5
     *       C----D-----A-----F------B-----E
     *
     *
     */
    const Map<std::string, Map<std::string,int>> provided = {
        {"A", {{"D", 2}, {"F",3}}},
        {"B", {{"E", 5}, {"F",4}}},
        {"C", {{"D", 1}}},
        {"D", {{"A", 2}, {"C",1}}},
        {"E", {{"B", 5}}},
        {"F", {{"A", 3}, {"B",4}}},
    };
    Vector<PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0}
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /* Standard Line. Two people on ends should have one connection.
         *      A  B  C  D  E  F
         *   1  1        1
         *   2  1              1
         *   3     1        1
         *   4     1           1
         *   5        1  1
         */

        //       0          1A       2B       3C         4D       5E        6F
        /*0*/ {0,0,0},   {2,11,8},{2,17,14},{1,20,20},{2,21,9},{1,15,15},{2,18,12},
        //       7          8A                           9D
        /*1*/ {-2,5,9},  {1,1,11},                    {4,4,21},
        //       10         11A                                             12F
        /*2*/ {-3,8,12}, {1,8,1},                                        {6,6,18},
        //       13                  14B                          15E
        /*3*/ {-5,11,15},         {2,2,17},                     {5,5,5},
        //       16                  17B                                    18F
        /*4*/ {-4,14,18},         {2,14,2},                              {6,12,6},
        //       19                           20C        21D
        /*5*/ {-1,17,21},                   {3,3,3},  {4,9,4},
        //       22
        /*6*/ {INT_MIN,20,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxItems, matches.dlx.links);
}

STUDENT_TEST("Weighted matching does not care about leaving others out.") {
    /* Here's the world:
     *                  10
     *                A --- B
     *                 \   /
     *                2 \ / 2
     *                   C
     *
     * There is no perfect matching here, unfortunately.
     */
    const Map<std::string, Map<std::string,int>> provided = {
        {"A", {{"B",10}, {"C",2}}},
        {"B", {{"A",10}, {"C",2}}},
        {"C", {{"A",2},{"B", 2}}},
    };
    Vector<PartnerLinks::personName> lookup {
        {"",3,1},{"A",0,2},{"B",1,3},{"C",2,0},
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C
         *   1  1  1
         *   3  1     1
         *   2     1  1
         */


        //       0         1A       2B      3C
        /*0*/ {0,0,0},   {2,8,5}, {2,11,6},{2,12,9},
        //       4         5A       6B
        /*1*/ {-10,2,6}, {1,1,8}, {2,2,11},
        //       7         8A               9C
        /*2*/ {-2,5,9},  {1,5,1},          {3,3,12},
        //       10                 11A     12C
        /*3*/ {-2,8,12},          {2,6,2}, {3,9,3},
        //       13
        /*4*/ {INT_MIN,11,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);
}


/* * * * * * * * * * * * * * * *              Cover Logic               * * * * * * * * * * * * * */


STUDENT_TEST("Covering a person in weighted will only take that person's pairs out.") {
    /* Here's the world:
     *                  10
     *                A --- B
     *                 \   /
     *                2 \ / 2
     *                   C
     *
     * There is no perfect matching here, unfortunately.
     */
    const Map<std::string, Map<std::string,int>> provided = {
        {"A", {{"B",10}, {"C",2}}},
        {"B", {{"A",10}, {"C",2}}},
        {"C", {{"A",2},{"B", 2}}},
    };
    Vector<PartnerLinks::personName> lookup {
        {"",3,1},{"A",0,2},{"B",1,3},{"C",2,0},
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C
         *   1  1  1
         *   3  1     1
         *   2     1  1
         */


        //       0         1A       2B      3C
        /*0*/ {0,0,0},   {2,8,5}, {2,11,6},{2,12,9},
        //       4         5A       6B
        /*1*/ {-10,2,6}, {1,1,8}, {2,2,11},
        //       7         8A               9C
        /*2*/ {-2,5,9},  {1,5,1},          {3,3,12},
        //       10                 11B     12C
        /*3*/ {-2,8,12},          {2,6,2}, {3,9,3},
        //       13
        /*4*/ {INT_MIN,11,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);

    matches.coverPerson(1);

    Vector<PartnerLinks::personName> lookupCoverA {
        {"",3,2},{"A",0,2},{"B",0,3},{"C",2,0},
    };
    Vector<PartnerLinks::personLink> dlxCoverA {
        /*
         *       B C
         *    3  1 1
         */


        //       0         1A       2B      3C
        /*0*/ {0,0,0},   {2,8,5}, {1,11,11},{1,12,12},
        //       4         5A       6B
        /*1*/ {-10,2,6}, {1,1,8}, {2,2,11},
        //       7         8A               9C
        /*2*/ {-2,5,9},  {1,5,1},          {3,3,12},
        //       10                 11B     12C
        /*3*/ {-2,8,12},          {2,2,2}, {3,3,3},
        //       13
        /*4*/ {INT_MIN,11,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxCoverA);

    matches.uncoverPerson(1);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);

}

STUDENT_TEST("All weights are unique so we can no that we report the right weight and pair.") {
    /* Here's the world:
     *                  3
     *               A-----B
     *            4  |     |  6
     *               |     |
     *               C-----D
     *                  5
     * There is no perfect matching here, unfortunately.
     */
    const Map<std::string, Map<std::string,int>> provided = {
        {"A", {{"B", 3}, {"C",4}}},
        {"B", {{"A", 3}, {"D",6}}},
        {"C", {{"A", 4}, {"D",5}}},
        {"D", {{"B", 6}, {"C",5}}},
    };
    Vector<PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0},
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D
         *   1  1  1
         *   2  1     1
         *   3     1     1
         *   4        1  1
         */
        //       0         1A      2B        3C        4D
        /*0*/ {0,0,0},   {2,9,6}, {2,12,7}, {2,15,10},{2,16,13},
        //       5         6A      7B
        /*1*/ {-3,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                10C
        /*2*/ {-4,6,10}, {1,6,1},          {3,3,15},
        //       11                12B                 13D
        /*3*/ {-6,9,13},          {2,7,2},            {4,4,16},
        //       14                          15C       16D
        /*4*/ {-5,12,16},                  {3,10,3}, {4,13,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);

    // Cover A, this will select option 2, partners are AB.
    std::pair<int,Pair> match = matches.coverWeightedPair(1);
    EXPECT_EQUAL(match.first, 3);
    EXPECT_EQUAL(match.second, {"A","B"});

    Vector<PartnerLinks::personName> lookupCoverA {
        {"",4,3},{"A",0,2},{"B",0,3},{"C",0,4},{"D",3,0},
    };
    Vector<PartnerLinks::personLink> dlxCoverA {
        /* Smaller links with A covered via option 1.
         *      C  D
         *   4  1  1
         */

        //       0         1A       2B         3C        4D
        /*1*/ {0,0,0},   {2,9,6}, {2,12,7}, {1,15,15},{1,16,16},
        //       5         6A       7B
        /*2*/ {-3,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                 10C
        /*3*/ {-4,6,10}, {1,6,1},           {3,3,15},
        //       11                 12B                  13D
        /*4*/ {-6,9,13},          {2,7,2},            {4,4,16},
        //       14                           15C        16D
        /*5*/ {-5,12,16},                   {3,3,3},  {4,4,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);
}

/* * * * * * * * * * * * * * * *             Solve Problem              * * * * * * * * * * * * * */


PROVIDED_TEST("maximumWeightMatching: Works on a square.") {
    /* This world:
     *
     *         1
     *      A --- B
     *      |     |
     *    8 |     | 2
     *      |     |
     *      D --- C
     *         4
     *
     * Best option is to pick BC/AD.
     */
    const Map<std::string, Map<std::string,int>> provided = {
        {"A", {{"B",1}, {"D",8}}},
        {"B", {{"A",1}, {"C",2}}},
        {"C", {{"B",2},{"D", 4}}},
        {"D", {{"A",8},{"C", 4}}},
    };
    PartnerLinks weights(provided);

    EXPECT_EQUAL(weights.getMaxWeightMatching(), { {"A", "D"}, {"B", "C"} });
}

PROVIDED_TEST("maximumWeightMatching: Works on a line of four people.") {
    /* This world:
     *
     *  A --- B --- C --- D
     *     1     3     1
     *
     * Best option is to pick B -- C, even though this is not a perfect
     * matching.
     */
    const Map<std::string, Map<std::string,int>> links = {
        {"A", {{"B",1}}},
        {"B", {{"A",1}, {"C",3}}},
        {"C", {{"B",3},{"D",1}}},
        {"D", {{"C",1}}},
    };
    PartnerLinks weights(links);

    /* Should pick B--C. */
    EXPECT_EQUAL(weights.getMaxWeightMatching(), { {"B", "C"} });
}

PROVIDED_TEST("maximumWeightMatching: Works on a line of three people.") {
    /* This world:
     *
     *  A --- B --- C
     *     1     2
     *
     * Best option is to pick B -- C.
     */
    auto links = fromWeightedLinks({
        { "A", "B", 1 },
        { "B", "C", 2 },
    });

    PartnerLinks weights(links);
    /* Should pick B--C. */
    EXPECT_EQUAL(weights.getMaxWeightMatching(), { {"B", "C"} });
}

PROVIDED_TEST("maximumWeightMatching: Odd shap that requires us to pick opposite edges.") {
    /* Because Map and Set internally store items in sorted order, the order
     * in which you iterate over people when making decisions is sensitive
     * to the order of those peoples' names. This test looks at a group like
     * this one, trying out all possible orderings of peoples' names:
     *
     *               E
     *               | 1
     *               C
     *            1 / \ 5
     *             A---B
     *          1 /  1  \ 1
     *           F       D
     *
     * (Best option is to pick the 5-cost edge and the opposite-side 1-cost
     * edge.)
     *
     */
    const Map<std::string, Map<std::string,int>> links = {
        {"A", {{"B",1},{"C",1},{"F",1}}},
        {"B", {{"A",1},{"C",5},{"D",1}}},
        {"C", {{"A",1},{"B",5},{"E",1}}},
        {"D", {{"B",1}}},
        {"E", {{"C",1}}},
        {"F", {{"A",1}}},
    };
    Vector<PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *     A  B  C  D  E  F
         *  1  1  1
         *  2  1     1
         *  3  1              1
         *  4     1  1
         *  5     1     1
         *  6        1     1
         *
         */

        //            1A       2B        3C       4D         5E       6F
        {0,0,0},   {3,14,8},{3,20,9},{3,23,12},{1,21,21},{1,24,24},{1,15,15},
        {-1,5,9},  {1,1,11},{2,2,17},
        {-1,8,12}, {1,8,14},         {3,3,18},
        {-1,11,15},{1,11,1},                                       {6,6,6},
        {-5,14,18},         {2,9,20},{3,12,23},
        {-1,17,21},         {2,17,2},          {4,4,4},
        {-1,20,24},                  {3,18,3},            {5,5,5},
        {-2147483648,23,-2147483648},

    };
    PartnerLinks weights(links);

    EXPECT_EQUAL(weights.getMaxWeightMatching(), {{"A","F"},{"B","C"}});
    EXPECT_EQUAL(weights.dlx.links, dlxItems);
    EXPECT_EQUAL(weights.dlx.lookupTable, lookup);
}

PROVIDED_TEST("maximumWeightMatching: Another permutation of the same shape is failing.") {
    /* Because Map and Set internally store items in sorted order, the order
     * in which you iterate over people when making decisions is sensitive
     * to the order of those peoples' names. This test looks at a group like
     * this one, trying out all possible orderings of peoples' names:
     *
     *               E
     *               | 1
     *               D
     *            1 / \ 5
     *             A---C
     *          1 /  1  \ 1
     *           F       B
     *
     * (Best option is to pick the 5-cost edge and the opposite-side 1-cost
     * edge.)
     *
     */
    const Map<std::string, Map<std::string,int>> links = {
        {"A", {{"C",1},{"D",1},{"F",1}}},
        {"B", {{"C",1}}},
        {"C", {{"A",1},{"B",1},{"D",5}}},
        {"D", {{"A",1},{"C",5},{"E",1}}},
        {"E", {{"D",1}}},
        {"F", {{"A",1}}},
    };
    Vector<PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *     A  B  C  D  E  F
         *  1  1     1
         *  2  1        1
         *  3  1              1
         *  4     1  1
         *  5        1  1
         *  6           1  1
         *
         */

        //            1A       2B        3C       4D         5E       6F
        {0,0,0},   {3,14,8},{1,17,17},{3,20,9},{3,23,12},{1,24,24},{1,15,15},
        {-1,5,9},  {1,1,11},          {3,3,18},
        {-1,8,12}, {1,8,14},                   {4,4,21},
        {-1,11,15},{1,11,1},                                       {6,6,6},
        {-1,14,18},         {2,2,2},  {3,9,20},
        {-5,17,21},                   {3,18,3},{4,12,23},
        {-1,20,24},                            {4,21,4}, {5,5,5},
        {INT_MIN,23,INT_MIN},


    };
    PartnerLinks weights(links);

    EXPECT_EQUAL(weights.getMaxWeightMatching(), {{"A","F"},{"C","D"}});
    EXPECT_EQUAL(weights.dlx.links, dlxItems);
    EXPECT_EQUAL(weights.dlx.lookupTable, lookup);
}

PROVIDED_TEST("maximumWeightMatching: The network resets after every run.") {
    /* Because Map and Set internally store items in sorted order, the order
     * in which you iterate over people when making decisions is sensitive
     * to the order of those peoples' names. This test looks at a group like
     * this one, trying out all possible orderings of peoples' names:
     *
     *               E
     *               | 1
     *               C
     *            1 / \ 5
     *             A---B
     *          1 /  1  \ 1
     *           F       D
     *
     * (Best option is to pick the 5-cost edge and the opposite-side 1-cost
     * edge.)
     *
     */
    const Map<std::string, Map<std::string,int>> links = {
        {"A", {{"B",1},{"C",1},{"F",1}}},
        {"B", {{"A",1},{"C",5},{"D",1}}},
        {"C", {{"A",1},{"B",5},{"E",1}}},
        {"D", {{"B",1}}},
        {"E", {{"C",1}}},
        {"F", {{"A",1}}},
    };
    Vector<PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *     A  B  C  D  E  F
         *  1  1  1
         *  1  1     1
         *  1  1              1
         *  5     1  1
         *  1     1     1
         *  1        1     1
         *
         */

        // 0          1A       2B        3C       4D         5E       6F
        {0,0,0},   {3,14,8},{3,20,9},{3,23,12},{1,21,21},{1,24,24},{1,15,15},
        // 7          8A       9B
        {-1,5,9},  {1,1,11},{2,2,17},
        // 10         11A                12C
        {-1,8,12}, {1,8,14},         {3,3,18},
        // 13         14A                                             15F
        {-1,11,15},{1,11,1},                                       {6,6,6},
        // 16                  17B       18C
        {-5,14,18},         {2,9,20},{3,12,23},
        // 19                  20B                21D
        {-1,17,21},         {2,17,2},          {4,4,4},
        // 22                            23C                24E
        {-1,20,24},                  {3,18,3},            {5,5,5},
        {INT_MIN,23,INT_MIN},
    };
    PartnerLinks weights(links);

    for (int i = 0; i < 11; i++) {
        EXPECT_EQUAL(weights.getMaxWeightMatching(), {{"A","F"},{"B","C"}});
        EXPECT_EQUAL(weights.dlx.links, dlxItems);
        EXPECT_EQUAL(weights.dlx.lookupTable, lookup);
    }

}

PROVIDED_TEST("maximumWeightMatching: Small stress test (should take at most a second or two).") {
    /* Because Map and Set internally store items in sorted order, the order
     * in which you iterate over people when making decisions is sensitive
     * to the order of those peoples' names. This test looks at a group like
     * this one, trying out all possible orderings of peoples' names:
     *
     *               *
     *               | 1
     *               *
     *            1 / \ 5
     *             *---*
     *          1 /  1  \ 1
     *           *       *
     *
     * (Best option is to pick the 5-cost edge and the opposite-side 1-cost
     * edge.)
     *
     * There are 6! = 720 possible permutations of the ordering of these
     * people. If your code is able to solve the problem correctly for all of
     * those orderings, there's a good chance that you're correctly tracking
     * who is matched at each step. On the other hand, if your code runs into
     * issues here, it may indicate that there's a bug in how you mark who's
     * paired and who isn't.
     */
    Vector<std::string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        auto links = fromWeightedLinks({
            { people[0], people[1], 5 },
            { people[1], people[2], 1 },
            { people[2], people[0], 1 },
            { people[3], people[0], 1 },
            { people[4], people[1], 1 },
            { people[5], people[2], 1 },
        });

        Set<Pair> expected = {
            { people[0], people[1] },
            { people[2], people[5] }
        };

        PartnerLinks weights(links);
        EXPECT_EQUAL(weights.getMaxWeightMatching(), expected);
    } while (next_permutation(people.begin(), people.end()));
}

PROVIDED_TEST("maximumWeightMatching: Large stress test (should take at most a second or two).") {
    /* Here, we're giving a chain of people, like this:
     *
     *    *---*---*---*---*---*---*---* ... *---*
     *      1   1   1   1   1   1   1         1
     *
     * The number of different matchings in a chain of n people is given by the
     * nth Fibonacci number. (Great exercise - can you explain why?) This means
     * that if we have a chain of 21 people, there are F(21) = 10,946 possible
     * matchings to check. If your program tests every single one of them exactly
     * once, then it should be pretty quick to determine what the best matching
     * here is. (It's any matching that uses exactly floor(21 / 2) = 10 edges.
     *
     * On the other hand, if your implementation repeatedly constructs the same
     * matchings over and over again, then the number of options you need to consider
     * will be too large for your computer to handle in any reasonable time.
     *
     * If you're passing the other tests and this test hangs, double-check your
     * code to make sure you aren't repeatedly constructing the same matchings
     * multiple times.
     */
    const int kNumPeople = 21;
    Vector<WeightedLink> links;
    for (int i = 0; i < kNumPeople - 1; i++) {
        links.add({ std::to_string(i), std::to_string(i + 1), 1 });
    }

    PartnerLinks weighted(fromWeightedLinks(links));

    auto matching = weighted.getMaxWeightMatching();
    EXPECT_EQUAL(matching.size(), kNumPeople / 2);

    /* Confirm it's a matching. */
    Set<std::string> used;
    for (Pair p: matching) {
        /* No people paired more than once. */
        EXPECT(!used.contains(p.first()));
        EXPECT(!used.contains(p.second()));
        used += p.first();
        used += p.second();

        /* Must be a possible links. */
        EXPECT_EQUAL(abs(stringToInteger(p.first()) - stringToInteger(p.second())), 1);
    }
}



/* * * * * * * * * * * * * * * *            PERFECT MATCHING            * * * * * * * * * * * * * */


/* * * * * * * * * * * * * * * *             Initialization             * * * * * * * * * * * * * */


STUDENT_TEST("Line of six but tricky due to natural order.") {
    /*
     *
     *       C--D--A--F--B--E
     *
     *
     */
    const Map<std::string, Set<std::string>> provided = {
        {"A", {"D", "F"}},
        {"B", {"E", "F"}},
        {"C", {"D"}},
        {"D", {"A", "C"}},
        {"E", {"B"}},
        {"F", {"A", "B"}},
    };
    Vector<PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0}
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /* Standard Line. Two people on ends should have one connection.
         *      A  B  C  D  E  F
         *   1  1        1
         *   2  1              1
         *   3     1        1
         *   4     1           1
         *   5        1  1
         */

        //       0          1A       2B       3C         4D       5E        6F
        /*0*/ {0,0,0},   {2,11,8},{2,17,14},{1,20,20},{2,21,9},{1,15,15},{2,18,12},
        //       7          8A                           9D
        /*1*/ {-1,5,9},  {1,1,11},                    {4,4,21},
        //       10         11A                                             12F
        /*2*/ {-2,8,12}, {1,8,1},                                        {6,6,18},
        //       13                  14B                          15E
        /*3*/ {-3,11,15},         {2,2,17},                     {5,5,5},
        //       16                  17B                                    18F
        /*4*/ {-4,14,18},         {2,14,2},                              {6,12,6},
        //       19                           20C        21D
        /*5*/ {-5,17,21},                   {3,3,3},  {4,9,4},
        //       22
        /*6*/ {INT_MIN,20,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxItems, matches.dlx.links);
}

STUDENT_TEST("We will allow setup of worlds that are impossible to match.") {
    /* Here's the world:
     *
     *               A --- B
     *                \   /
     *                 \ /
     *                  C
     *
     * There is no perfect matching here, unfortunately.
     */
    const Map<std::string, Set<std::string>> provided = {
        { "A", {"B"} },
        { "B", {"C"} },
        { "C", {"A"} }
    };
    Vector<PartnerLinks::personName> lookup {
        {"",3,1},{"A",0,2},{"B",1,3},{"C",2,0},
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C
         *   1  1  1
         *   2     1  1
         *   3  1     1
         */


        //       0         1A       2B      3C
        /*0*/ {0,0,0},   {2,11,5},{2,8,6},{2,12,9},
        //       4         5A       6B
        /*1*/ {-1,2,6},  {1,1,11},{2,2,8},
        //       7                  8B      9C
        /*2*/ {-2,5,9},           {2,6,2},{3,3,12},
        //       10        11A              12C
        /*3*/ {-3,8,12}, {1,5,1},         {3,9,3},
        //       13
        /*4*/ {INT_MIN,11,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);
}

STUDENT_TEST("Initialize a world that will have matching.") {
    /* Here's the world:
     *
     *               C --- B
     *             /        \
     *            E          F
     *             \        /
     *               D --- A
     *
     */
    const Map<std::string, Set<std::string>> provided = {
        { "A", {"D", "F"} },
        { "B", {"C", "F"} },
        { "C", {"B", "E"} },
        { "D", {"A", "E"} },
        { "E", {"C", "D"} },
        { "F", {"A", "B"} }
    };
    Vector<PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D  E  F
         *   1  1        1
         *   2  1              1
         *   3     1  1
         *   4     1           1
         *   5        1     1
         *   6           1  1
         */

        //       0          1A       2B       3C         4D       5E        6F
        /*0*/ {0,0,0},   {2,11,8},{2,17,14},{2,20,15},{2,23,9},{2,24,21},{2,18,12},
        //       7          8A                           9D
        /*1*/ {-1,5,9},  {1,1,11},                    {4,4,23},
        //       10         11A                                             12F
        /*2*/ {-2,8,12}, {1,8,1},                                        {6,6,18},
        //       13                  14B      15C
        /*3*/ {-3,11,15},         {2,2,17}, {3,3,20},
        //       16                  17B                                    18F
        /*4*/ {-4,14,18},         {2,14,2},                              {6,12,6},
        //       19                           20C                 21E
        /*5*/ {-5,17,21},                   {3,15,3},          {5,5,24},
        //       22                                      23D      24E
        /*6*/ {-6,20,24},                             {4,9,4}, {5,21,5},
        //       25
        /*7*/ {INT_MIN,23,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxItems, matches.dlx.links);
}

STUDENT_TEST("Setup works on a disconnected hexagon of people and reportes singleton.") {
    /* Here's the world:
     *
     *               C --- B
     *             /        \
     *            E          F
     *             \
     *               D     A
     *
     *
     *
     */
    const Map<std::string, Set<std::string>> provided = {
        { "A", {} },
        { "B", {"C", "F"} },
        { "C", {"B", "E"} },
        { "D", {"E"} },
        { "E", {"C", "D"} },
        { "F", {"B"} }
    };
    Vector<PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         * None-v
         *      A  B  C  D  E  F
         *   1     1  1
         *   2     1           1
         *   3        1     1
         *   4           1  1
         */


        //      NO MATCHES--v
        //       0          1A       2B       3C       4D        5E        6F
        /*0*/ {0,0,0},   {0,1,1}, {2,11,8},{2,14,9},{1,17,17},{2,18,15},{1,12,12},
        /*1*/ {-1,5,9},           {2,2,11},{3,3,14},
        /*2*/ {-2,8,12},          {2,8,2},                              {6,6,6},
        /*3*/ {-3,11,15},                  {3,9,3},           {5,5,18},
        /*4*/ {-4,14,18},                           {4,4,4},  {5,15,5},
        /*6*/ {INT_MIN,17,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT(matches.dlx.hasSingleton);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxItems, matches.dlx.links);
}


/* * * * * * * * * * * * * * * *              Cover Logic               * * * * * * * * * * * * * */


STUDENT_TEST("Simple square any valid partners will work. Cover A.") {
    /* Here's the world:
     *
     *               A-----B
     *               |     |
     *               |     |
     *               C-----D
     *
     * There is no perfect matching here, unfortunately.
     */
    const Map<std::string, Set<std::string>> provided = {
        { "A",{"B","C"}},
        { "B",{"A","D"}},
        { "C",{"A","D"}},
        { "D",{"C","B"}},
    };
    Vector<PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0},
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D
         *   1  1  1
         *   2  1     1
         *   3     1     1
         *   4        1  1
         */
        //       0         1A      2B        3C        4D
        /*0*/ {0,0,0},   {2,9,6}, {2,12,7}, {2,15,10},{2,16,13},
        //       5         6A      7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                10C
        /*2*/ {-2,6,10}, {1,6,1},          {3,3,15},
        //       11                12B                 13D
        /*3*/ {-3,9,13},          {2,7,2},            {4,4,16},
        //       14                          15C       16D
        /*4*/ {-4,12,16},                  {3,10,3}, {4,13,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);

    // Cover A, this will select option 2, partners are AB.
    Pair match = matches.coverPairing(1);
    EXPECT_EQUAL(match, {"A","B"});

    Vector<PartnerLinks::personName> lookupCoverA {
        {"",4,3},{"A",0,2},{"B",0,3},{"C",0,4},{"D",3,0},
    };
    Vector<PartnerLinks::personLink> dlxCoverA {
        /* Smaller links with A covered via option 1.
         *      C  D
         *   4  1  1
         */

        //       0         1A       2B         3C        4D
        /*1*/ {0,0,0},   {2,9,6}, {2,12,7}, {1,15,15},{1,16,16},
        //       5         6A       7B
        /*2*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                 10C
        /*3*/ {-2,6,10}, {1,6,1},           {3,3,15},
        //       11                 12B                  13D
        /*4*/ {-3,9,13},          {2,7,2},            {4,4,16},
        //       14                           15C        16D
        /*5*/ {-4,12,16},                   {3,3,3},  {4,4,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);
}

STUDENT_TEST("There are no perfect pairings, any matching will fail.") {
    /* Here's the world:
     *
     *               A --- B
     *                \   /
     *                 \ /
     *                  C
     *
     * There is no perfect matching here, unfortunately.
     */
    const Map<std::string, Set<std::string>> provided = {
        { "A", {"B"} },
        { "B", {"C"} },
        { "C", {"A"} }
    };
    Vector<PartnerLinks::personName> lookup {
        {"",3,1},{"A",0,2},{"B",1,3},{"C",2,0},
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C
         *   1  1  1
         *   2     1  1
         *   3  1     1
         */


        //       0         1A       2B      3C
        /*0*/ {0,0,0},   {2,11,5},{2,8,6},{2,12,9},
        //       4         5A       6B
        /*1*/ {-1,2,6},  {1,1,11},{2,2,8},
        //       7                  8B      9C
        /*2*/ {-2,5,9},           {2,6,2},{3,3,12},
        //       10        11A              12C
        /*3*/ {-3,8,12}, {1,5,1},         {3,9,3},
        //       13
        /*4*/ {INT_MIN,11,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);

    Pair match = matches.coverPairing(1);
    EXPECT_EQUAL(match, {"A", "B"});
    Vector<PartnerLinks::personName> lookupA {
        {"",3,3},{"A",0,2},{"B",0,3},{"C",0,0},
    };
    Vector<PartnerLinks::personLink> dlxCoverA {
        /*
         * C is left but there are no available options for C.
         *
         *      C
         *    0
         */


        //       0         1A       2B      3C
        /*0*/ {0,0,0},   {2,11,5},{2,8,6},{0,3,3},
        //       4         5A       6B
        /*1*/ {-1,2,6},  {1,1,11},{2,2,8},
        //       7                  8B      9C
        /*2*/ {-2,5,9},           {2,6,2},{3,3,3},
        //       10        11A              12C
        /*3*/ {-3,8,12}, {1,5,1},         {3,9,3},
        //       13
        /*4*/ {INT_MIN,11,INT_MIN},
    };
    EXPECT_EQUAL(lookupA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);
}

STUDENT_TEST("We will quickly learn that A-B is a bad pairing that leaves C out.") {
    /* Here's the world:
     *
     *               A --- B---C
     *                \   /
     *                 \ /
     *                  D
     *
     * There is no perfect matching here, unfortunately.
     */
    const Map<std::string, Set<std::string>> provided = {
        { "A", {"B", "D"} },
        { "B", {"A","C","D"} },
        { "C", {"B"} },
        { "D", {"A","B"}},
    };
    Vector<PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0}
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D
         *   1  1  1
         *   2  1        1
         *   3     1  1
         *   4     1     1
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {3,15,7},{1,13,13},{2,16,10},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                    {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,7,15},{3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},          {4,10,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);

    Pair match = matches.coverPairing(1);
    EXPECT_EQUAL(match, {"A", "B"});
    Vector<PartnerLinks::personName> lookupCoverA {
        {"",4,3},{"A",0,2},{"B",0,3},{"C",0,4},{"D",3,0}
    };
    Vector<PartnerLinks::personLink> dlxCoverA {
        /*
         * We cannot reach C and D if we pair A and B.
         *
         *      C  D
         *   0
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {3,15,7},{0,3,3}, {0,4,4},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                   {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,7,15},{3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},         {4,4,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);
}

STUDENT_TEST("A-D is a good pairing.") {
    /* Here's the world:
     *
     *               A --- B---C
     *                \   /
     *                 \ /
     *                  D
     *
     * There is no perfect matching here, unfortunately.
     */
    const Map<std::string, Set<std::string>> provided = {
        { "A", {"B", "D"} },
        { "B", {"A","C","D"} },
        { "C", {"B"} },
        { "D", {"A","B"}},
    };
    Vector<PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0}
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D
         *   1  1  1
         *   2  1        1
         *   3     1  1
         *   4     1     1
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {3,15,7},{1,13,13},{2,16,10},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                    {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,7,15},{3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},          {4,10,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);

    Pair match = matches.coverPairing(6);
    EXPECT_EQUAL(match, {"A", "D"});
    Vector<PartnerLinks::personName> lookupCoverA {
        {"",3,2},{"A",0,2},{"B",0,3},{"C",2,0},{"D",3,0}
    };
    Vector<PartnerLinks::personLink> dlxCoverA {
        /*
         *      B  C
         *   3  1  1
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {1,12,12},{1,13,13},{2,16,10},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                     {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,2,2},  {3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},           {4,10,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);
}

STUDENT_TEST("Cover A in a world where everyone has two connections.") {
    /* Here's the world:
     *
     *               C --- B
     *             /        \
     *            E          F
     *             \        /
     *               D --- A
     *
     */
    const Map<std::string, Set<std::string>> provided = {
        { "A", {"D", "F"} },
        { "B", {"C", "F"} },
        { "C", {"B", "E"} },
        { "D", {"A", "E"} },
        { "E", {"C", "D"} },
        { "F", {"A", "B"} }
    };
    Vector<PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D  E  F
         *   1  1        1
         *   2  1              1
         *   3     1  1
         *   4     1           1
         *   5        1     1
         *   6           1  1
         */

        //       0          1A       2B       3C         4D       5E        6F
        /*0*/ {0,0,0},   {2,11,8},{2,17,14},{2,20,15},{2,23,9},{2,24,21},{2,18,12},
        //       7          8A                           9D
        /*1*/ {-1,5,9},  {1,1,11},                    {4,4,23},
        //       10         11A                                             12F
        /*2*/ {-2,8,12}, {1,8,1},                                        {6,6,18},
        //       13                  14B      15C
        /*3*/ {-3,11,15},         {2,2,17}, {3,3,20},
        //       16                  17B                                    18F
        /*4*/ {-4,14,18},         {2,14,2},                              {6,12,6},
        //       19                           20C                 21E
        /*5*/ {-5,17,21},                   {3,15,3},          {5,5,24},
        //       22                                      23D      24E
        /*6*/ {-6,20,24},                             {4,9,4}, {5,21,5},
        //       25
        /*7*/ {INT_MIN,23,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxItems, matches.dlx.links);

    Pair match = matches.coverPairing(1);
    EXPECT_EQUAL(match, {"A","D"});

    Vector<PartnerLinks::personName> lookupCoverA {
        {"",6,2},{"A",0,2},{"B",0,3},{"C",2,5},{"D",3,5},{"E",3,6},{"F",5,0},
    };
    Vector<PartnerLinks::personLink> dlxCoverA {
        /*
         *      B  C  E  F
         *   3  1  1
         *   4  1        1
         *   5     1  1
         */

        //       0          1A       2B       3C         4D       5E        6F
        /*0*/ {0,0,0},   {2,11,8},{2,17,14},{2,20,15},{2,23,9},{1,21,21},{1,18,18},
        //       7          8A                           9D
        /*1*/ {-1,5,9},  {1,1,11},                    {4,4,23},
        //       10         11A                                             12F
        /*2*/ {-2,8,12}, {1,8,1},                                        {6,6,18},
        //       13                  14B      15C
        /*3*/ {-3,11,15},         {2,2,17}, {3,3,20},
        //       16                  17B                                    18F
        /*4*/ {-4,14,18},         {2,14,2},                              {6,6,6},
        //       19                           20C                 21E
        /*5*/ {-5,17,21},                   {3,15,3},          {5,5,5},
        //       22                                      23D      24E
        /*6*/ {-6,20,24},                             {4,9,4}, {5,21,5},
        //       25
        /*7*/ {INT_MIN,23,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);
}

STUDENT_TEST("A-D then B-C solves the world.") {
    /* Here's the world:
     *
     *               A --- B---C
     *                \   /
     *                 \ /
     *                  D
     *
     * There is no perfect matching here, unfortunately.
     */
    const Map<std::string, Set<std::string>> provided = {
        { "A", {"B", "D"} },
        { "B", {"A","C","D"} },
        { "C", {"B"} },
        { "D", {"A","B"}},
    };
    Vector<PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0}
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D
         *   1  1  1
         *   2  1        1
         *   3     1  1
         *   4     1     1
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {3,15,7},{1,13,13},{2,16,10},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                    {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,7,15},{3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},          {4,10,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);

    Pair match = matches.coverPairing(6);
    EXPECT_EQUAL(match, {"A", "D"});
    Vector<PartnerLinks::personName> lookupCoverA {
        {"",3,2},{"A",0,2},{"B",0,3},{"C",2,0},{"D",3,0}
    };
    Vector<PartnerLinks::personLink> dlxCoverA {
        /*
         *      B  C
         *   3  1  1
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {1,12,12},{1,13,13},{2,16,10},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                     {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,2,2},  {3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},           {4,10,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);

    match = matches.coverPairing(2);
    EXPECT_EQUAL(match, {"B", "C"});
    Vector<PartnerLinks::personName> lookupCoverB {
        {"",0,0},{"A",0,2},{"B",0,3},{"C",0,0},{"D",3,0}
    };
    Vector<PartnerLinks::personLink> dlxCoverB {
        /*
         * Empty world is solved.
         *
         *     0
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {1,12,12},{1,13,13},{2,16,10},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                     {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,2,2},  {3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},           {4,10,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverB, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverB, matches.dlx.links);
}


/* * * * * * * * * * * * * * * *         Cover/Uncover Logic            * * * * * * * * * * * * * */


STUDENT_TEST("Simple square any valid partners will work. Cover A then uncover.") {
    /* Here's the world:
     *
     *               A-----B
     *               |     |
     *               |     |
     *               C-----D
     *
     * There is no perfect matching here, unfortunately.
     */
    const Map<std::string, Set<std::string>> provided = {
        { "A",{"B","C"}},
        { "B",{"A","D"}},
        { "C",{"A","D"}},
        { "D",{"C","B"}},
    };
    Vector<PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0},
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D
         *   1  1  1
         *   2  1     1
         *   3     1     1
         *   4        1  1
         */
        //       0         1A      2B        3C        4D
        /*0*/ {0,0,0},   {2,9,6}, {2,12,7}, {2,15,10},{2,16,13},
        //       5         6A      7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                10C
        /*2*/ {-2,6,10}, {1,6,1},          {3,3,15},
        //       11                12B                 13D
        /*3*/ {-3,9,13},          {2,7,2},            {4,4,16},
        //       14                          15C       16D
        /*4*/ {-4,12,16},                  {3,10,3}, {4,13,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);

    // Cover A, this will select option 2, partners are AB.
    Pair match = matches.coverPairing(1);
    EXPECT_EQUAL(match, {"A","B"});

    Vector<PartnerLinks::personName> lookupCoverA {
        {"",4,3},{"A",0,2},{"B",0,3},{"C",0,4},{"D",3,0},
    };
    Vector<PartnerLinks::personLink> dlxCoverA {
        /* Smaller links with A covered via option 1.
         *      C  D
         *   4  1  1
         */

        //       0         1A       2B         3C        4D
        /*1*/ {0,0,0},   {2,9,6}, {2,12,7}, {1,15,15},{1,16,16},
        //       5         6A       7B
        /*2*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                 10C
        /*3*/ {-2,6,10}, {1,6,1},           {3,3,15},
        //       11                 12B                  13D
        /*4*/ {-3,9,13},          {2,7,2},            {4,4,16},
        //       14                           15C        16D
        /*5*/ {-4,12,16},                   {3,3,3},  {4,4,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);

    matches.uncoverPairing(1);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);

}

STUDENT_TEST("There are no perfect pairings, any matching will fail. Cover uncover.") {
    /* Here's the world:
     *
     *               A --- B
     *                \   /
     *                 \ /
     *                  C
     *
     * There is no perfect matching here, unfortunately.
     */
    const Map<std::string, Set<std::string>> provided = {
        { "A", {"B"} },
        { "B", {"C"} },
        { "C", {"A"} }
    };
    Vector<PartnerLinks::personName> lookup {
        {"",3,1},{"A",0,2},{"B",1,3},{"C",2,0},
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C
         *   1  1  1
         *   2     1  1
         *   3  1     1
         */


        //       0         1A       2B      3C
        /*0*/ {0,0,0},   {2,11,5},{2,8,6},{2,12,9},
        //       4         5A       6B
        /*1*/ {-1,2,6},  {1,1,11},{2,2,8},
        //       7                  8B      9C
        /*2*/ {-2,5,9},           {2,6,2},{3,3,12},
        //       10        11A              12C
        /*3*/ {-3,8,12}, {1,5,1},         {3,9,3},
        //       13
        /*4*/ {INT_MIN,11,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);

    Pair match = matches.coverPairing(1);
    EXPECT_EQUAL(match, {"A", "B"});
    Vector<PartnerLinks::personName> lookupA {
        {"",3,3},{"A",0,2},{"B",0,3},{"C",0,0},
    };
    Vector<PartnerLinks::personLink> dlxCoverA {
        /*
         * C is left but there are no available options for C.
         *
         *      C
         *    0
         */


        //       0         1A       2B      3C
        /*0*/ {0,0,0},   {2,11,5},{2,8,6},{0,3,3},
        //       4         5A       6B
        /*1*/ {-1,2,6},  {1,1,11},{2,2,8},
        //       7                  8B      9C
        /*2*/ {-2,5,9},           {2,6,2},{3,3,3},
        //       10        11A              12C
        /*3*/ {-3,8,12}, {1,5,1},         {3,9,3},
        //       13
        /*4*/ {INT_MIN,11,INT_MIN},
    };
    EXPECT_EQUAL(lookupA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);

    matches.uncoverPairing(1);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);
}

STUDENT_TEST("We will quickly learn that A-B is a bad pairing that leaves C out. Uncover.") {
    /* Here's the world:
     *
     *               A --- B---C
     *                \   /
     *                 \ /
     *                  D
     *
     * There is no perfect matching here, unfortunately.
     */
    const Map<std::string, Set<std::string>> provided = {
        { "A", {"B", "D"} },
        { "B", {"A","C","D"} },
        { "C", {"B"} },
        { "D", {"A","B"}},
    };
    Vector<PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0}
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D
         *   1  1  1
         *   2  1        1
         *   3     1  1
         *   4     1     1
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {3,15,7},{1,13,13},{2,16,10},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                    {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,7,15},{3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},          {4,10,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);

    Pair match = matches.coverPairing(1);
    EXPECT_EQUAL(match, {"A", "B"});
    Vector<PartnerLinks::personName> lookupCoverA {
        {"",4,3},{"A",0,2},{"B",0,3},{"C",0,4},{"D",3,0}
    };
    Vector<PartnerLinks::personLink> dlxCoverA {
        /*
         * We cannot reach C and D if we pair A and B.
         *
         *      C  D
         *   0
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {3,15,7},{0,3,3}, {0,4,4},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                   {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,7,15},{3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},         {4,4,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);
}

STUDENT_TEST("A-D is a good pairing. Cover then uncover.") {
    /* Here's the world:
     *
     *               A --- B---C
     *                \   /
     *                 \ /
     *                  D
     *
     * There is no perfect matching here, unfortunately.
     */
    const Map<std::string, Set<std::string>> provided = {
        { "A", {"B", "D"} },
        { "B", {"A","C","D"} },
        { "C", {"B"} },
        { "D", {"A","B"}},
    };
    Vector<PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0}
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D
         *   1  1  1
         *   2  1        1
         *   3     1  1
         *   4     1     1
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {3,15,7},{1,13,13},{2,16,10},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                    {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,7,15},{3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},          {4,10,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);

    Pair match = matches.coverPairing(6);
    EXPECT_EQUAL(match, {"A", "D"});
    Vector<PartnerLinks::personName> lookupCoverA {
        {"",3,2},{"A",0,2},{"B",0,3},{"C",2,0},{"D",3,0}
    };
    Vector<PartnerLinks::personLink> dlxCoverA {
        /*
         *      B  C
         *   3  1  1
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {1,12,12},{1,13,13},{2,16,10},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                     {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,2,2},  {3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},           {4,10,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);

    matches.uncoverPairing(6);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);
}

STUDENT_TEST("Cover A in a world where everyone has two connections then uncover.") {
    /* Here's the world:
     *
     *               C --- B
     *             /        \
     *            E          F
     *             \        /
     *               D --- A
     *
     */
    const Map<std::string, Set<std::string>> provided = {
        { "A", {"D", "F"} },
        { "B", {"C", "F"} },
        { "C", {"B", "E"} },
        { "D", {"A", "E"} },
        { "E", {"C", "D"} },
        { "F", {"A", "B"} }
    };
    Vector<PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D  E  F
         *   1  1        1
         *   2  1              1
         *   3     1  1
         *   4     1           1
         *   5        1     1
         *   6           1  1
         */

        //       0          1A       2B       3C         4D       5E        6F
        /*0*/ {0,0,0},   {2,11,8},{2,17,14},{2,20,15},{2,23,9},{2,24,21},{2,18,12},
        //       7          8A                           9D
        /*1*/ {-1,5,9},  {1,1,11},                    {4,4,23},
        //       10         11A                                             12F
        /*2*/ {-2,8,12}, {1,8,1},                                        {6,6,18},
        //       13                  14B      15C
        /*3*/ {-3,11,15},         {2,2,17}, {3,3,20},
        //       16                  17B                                    18F
        /*4*/ {-4,14,18},         {2,14,2},                              {6,12,6},
        //       19                           20C                 21E
        /*5*/ {-5,17,21},                   {3,15,3},          {5,5,24},
        //       22                                      23D      24E
        /*6*/ {-6,20,24},                             {4,9,4}, {5,21,5},
        //       25
        /*7*/ {INT_MIN,23,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxItems, matches.dlx.links);

    Pair match = matches.coverPairing(1);
    EXPECT_EQUAL(match, {"A","D"});

    Vector<PartnerLinks::personName> lookupCoverA {
        {"",6,2},{"A",0,2},{"B",0,3},{"C",2,5},{"D",3,5},{"E",3,6},{"F",5,0},
    };
    Vector<PartnerLinks::personLink> dlxCoverA {
        /*
         *      B  C  E  F
         *   3  1  1
         *   4  1        1
         *   5     1  1
         */

        //       0          1A       2B       3C         4D       5E        6F
        /*0*/ {0,0,0},   {2,11,8},{2,17,14},{2,20,15},{2,23,9},{1,21,21},{1,18,18},
        //       7          8A                           9D
        /*1*/ {-1,5,9},  {1,1,11},                    {4,4,23},
        //       10         11A                                             12F
        /*2*/ {-2,8,12}, {1,8,1},                                        {6,6,18},
        //       13                  14B      15C
        /*3*/ {-3,11,15},         {2,2,17}, {3,3,20},
        //       16                  17B                                    18F
        /*4*/ {-4,14,18},         {2,14,2},                              {6,6,6},
        //       19                           20C                 21E
        /*5*/ {-5,17,21},                   {3,15,3},          {5,5,5},
        //       22                                      23D      24E
        /*6*/ {-6,20,24},                             {4,9,4}, {5,21,5},
        //       25
        /*7*/ {INT_MIN,23,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);

    matches.uncoverPairing(1);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);
}

STUDENT_TEST("Depth two cover and uncover. Cover A then B then uncover B.") {
    /* Here's the world:
     *
     *               C --- B
     *             /        \
     *            E          F
     *             \        /
     *               D --- A
     *
     */
    const Map<std::string, Set<std::string>> provided = {
        { "A", {"D", "F"} },
        { "B", {"C", "F"} },
        { "C", {"B", "E"} },
        { "D", {"A", "E"} },
        { "E", {"C", "D"} },
        { "F", {"A", "B"} }
    };
    Vector<PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D  E  F
         *   1  1        1
         *   2  1              1
         *   3     1  1
         *   4     1           1
         *   5        1     1
         *   6           1  1
         */

        //       0          1A       2B       3C         4D       5E        6F
        /*0*/ {0,0,0},   {2,11,8},{2,17,14},{2,20,15},{2,23,9},{2,24,21},{2,18,12},
        //       7          8A                           9D
        /*1*/ {-1,5,9},  {1,1,11},                    {4,4,23},
        //       10         11A                                             12F
        /*2*/ {-2,8,12}, {1,8,1},                                        {6,6,18},
        //       13                  14B      15C
        /*3*/ {-3,11,15},         {2,2,17}, {3,3,20},
        //       16                  17B                                    18F
        /*4*/ {-4,14,18},         {2,14,2},                              {6,12,6},
        //       19                           20C                 21E
        /*5*/ {-5,17,21},                   {3,15,3},          {5,5,24},
        //       22                                      23D      24E
        /*6*/ {-6,20,24},                             {4,9,4}, {5,21,5},
        //       25
        /*7*/ {INT_MIN,23,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxItems, matches.dlx.links);

    Pair match = matches.coverPairing(1);
    EXPECT_EQUAL(match, {"A","D"});

    Vector<PartnerLinks::personName> lookupCoverA {
        {"",6,2},{"A",0,2},{"B",0,3},{"C",2,5},{"D",3,5},{"E",3,6},{"F",5,0},
    };
    Vector<PartnerLinks::personLink> dlxCoverA {
        /*
         *      B  C  E  F
         *   3  1  1
         *   4  1        1
         *   5     1  1
         */

        //       0          1A       2B       3C         4D       5E        6F
        /*0*/ {0,0,0},   {2,11,8},{2,17,14},{2,20,15},{2,23,9},{1,21,21},{1,18,18},
        //       7          8A                           9D
        /*1*/ {-1,5,9},  {1,1,11},                    {4,4,23},
        //       10         11A                                             12F
        /*2*/ {-2,8,12}, {1,8,1},                                        {6,6,18},
        //       13                  14B      15C
        /*3*/ {-3,11,15},         {2,2,17}, {3,3,20},
        //       16                  17B                                    18F
        /*4*/ {-4,14,18},         {2,14,2},                              {6,6,6},
        //       19                           20C                 21E
        /*5*/ {-5,17,21},                   {3,15,3},          {5,5,5},
        //       22                                      23D      24E
        /*6*/ {-6,20,24},                             {4,9,4}, {5,21,5},
        //       25
        /*7*/ {INT_MIN,23,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);

    // Pair B C but that is a bad choice so we will have to uncover.
    match = matches.coverPairing(2);
    EXPECT_EQUAL(match, {"B","C"});

    Vector<PartnerLinks::personName> lookupCoverB {
        {"",6,5},{"A",0,2},{"B",0,3},{"C",0,5},{"D",3,5},{"E",0,6},{"F",5,0},
    };
    Vector<PartnerLinks::personLink> dlxCoverB {
        /*
         * Pairing up B and C will leave us no way to access E and F. Must uncover.
         *
         *       E  F
         *    0
         */

        //       0          1A       2B       3C         4D       5E        6F
        /*0*/ {0,0,0},   {2,11,8},{2,17,14},{2,20,15},{2,23,9},{0,5,5},{0,6,6},
        //       7          8A                           9D
        /*1*/ {-1,5,9},  {1,1,11},                    {4,4,23},
        //       10         11A                                             12F
        /*2*/ {-2,8,12}, {1,8,1},                                        {6,6,18},
        //       13                  14B      15C
        /*3*/ {-3,11,15},         {2,2,17}, {3,3,20},
        //       16                  17B                                    18F
        /*4*/ {-4,14,18},         {2,14,2},                              {6,6,6},
        //       19                           20C                 21E
        /*5*/ {-5,17,21},                   {3,15,3},          {5,5,5},
        //       22                                      23D      24E
        /*6*/ {-6,20,24},                             {4,9,4}, {5,21,5},
        //       25
        /*7*/ {INT_MIN,23,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverB, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverB, matches.dlx.links);

    matches.uncoverPairing(2);
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);

    matches.uncoverPairing(1);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxItems, matches.dlx.links);
}


/* * * * * * * * * * * * * * * *            Solve Problem               * * * * * * * * * * * * * */


STUDENT_TEST("Largest shape I will do by hand. After successive calls the network should reset.") {
    /* Here's the world:
     *
     *               A --- B ---C
     *             /        \   \
     *       I----J          E---D
     *       |     \        /
     *       H----- G --- F
     *
     *
     *
     */
    const Map<std::string, Set<std::string>> provided = {
        { "A", {"B", "J"} },
        { "B", {"A", "C", "E"} },
        { "C", {"B", "D"} },
        { "D", {"C", "E"} },
        { "E", {"B", "D", "F"} },
        { "F", {"E", "G"} },
        { "G", {"F", "H", "J"} },
        { "H", {"G", "I"} },
        { "I", {"H", "J"} },
        { "J", {"A", "G", "I"} }
    };
    Vector<PartnerLinks::personName> lookup {
        {"",10,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,7},{"G",6,8},{"H",7,9},{"I",8,10},{"J",9,0},
    };
    Vector<PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D  E  F  G  H  I  J
         *   1  1  1
         *   2  1                          1
         *   3     1  1
         *   4     1        1
         *   5        1  1
         *   6           1  1
         *   7              1  1
         *   8                 1  1
         *   9                    1  1
         *  10                    1        1
         *  11                       1  1
         *  12                          1  1
         *
         */

        //              A         B         C        D          E         F         G         H         I        J
        {0,0,0},    {2,15,12},{3,21,13},{2,24,19},{2,27,25},{3,30,22},{2,33,31},{3,39,34},{2,42,37},{2,45,43},{3,46,16},
        {-1,9,13},  {1,1,15}, {2,2,18},
        {-2,12,16}, {1,12,1},                                                                                 {10,10,40},
        {-3,15,19},           {2,13,21},{3,3,24},
        {-4,18,22},           {2,18,2},                     {5,5,28},
        {-5,21,25},                     {3,19,3}, {4,4,27},
        {-6,24,28},                               {4,25,4}, {5,22,30},
        {-7,27,31},                                         {5,28,5}, {6,6,33},
        {-8,30,34},                                                   {6,31,6}, {7,7,36},
        {-9,33,37},                                                             {7,34,39},{8,8,42},
        {-10,36,40},                                                            {7,36,7},                     {10,16,46},
        {-11,39,43},                                                                      {8,37,8}, {9,9,45},
        {-12,42,46},                                                                                {9,43,9}, {10,40,10},
        {INT_MIN,45,INT_MIN},
    };

    PartnerLinks network(provided);
    for (int i = 0; i < 11; i++) {
        Set<Pair> matching = {};
        EXPECT(network.hasPerfectLinks(matching));
        EXPECT_EQUAL(lookup, network.dlx.lookupTable);
        EXPECT_EQUAL(dlxItems, network.dlx.links);
    }
}


namespace {

    /* Pairs to world. */
    Map<std::string, Set<std::string>> fromLinks(const Vector<Pair>& pairs) {
        Map<std::string, Set<std::string>> result;
        for (const auto& link: pairs) {
            result[link.first()].add(link.second());
            result[link.second()].add(link.first());
        }
        return result;
    }

    /* Checks if a set of pairs forms a perfect matching. */
    bool isPerfectMatching(const Map<std::string, Set<std::string>>& possibleLinks,
                           const Set<Pair>& matching) {
        /* Need to check that
         *
         * 1. each pair is indeed a possible link,
         * 2. each person appears in exactly one pair.
         */
        Set<std::string> used;
        for (Pair p: matching) {
            /* Are these folks even in the group of people? */
            if (!possibleLinks.containsKey(p.first())) return false;
            if (!possibleLinks.containsKey(p.second())) return false;

            /* If these people are in the group, are they linked? */
            if (!possibleLinks[p.first()].contains(p.second()) ||
                !possibleLinks[p.second()].contains(p.first())) {
                return false;
            }

            /* Have we seen them before? */
            if (used.contains(p.first()) || used.contains(p.second())) {
                return false;
            }

            /* Add them both. */
            used += p.first();
            used += p.second();
        }

        /* Confirm that's everyone. */
        return used.size() == possibleLinks.size();
    }
}

PROVIDED_TEST("hasPerfectMatching works on a world with just one person.") {
    /* The world is just a single person A, with no others. How sad. :-(
     *
     *                 A
     *
     * There is no perfect matching.
     */

    Set<Pair> unused;
    Map<std::string, Set<std::string>> map = {{"A", {}}};
    PartnerLinks network(map);
    EXPECT(!network.hasPerfectLinks(unused));
}

PROVIDED_TEST("hasPerfectMatching works on an empty set of people.") {
    /* There actually is a perfect matching - the set of no links meets the
     * requirements.
     */
    Set<Pair> unused;
    Map<std::string, Set<std::string>> map = {};
    PartnerLinks network(map);
    EXPECT(network.hasPerfectLinks(unused));
}

PROVIDED_TEST("hasPerfectMatching works on a world with two linked people.") {
    /* This world is a pair of people A and B. There should be a perfect matching.
     *
     *               A -- B
     *
     * The matching is {A, B}
     */
    auto links = fromLinks({
        { "A", "B" }
    });

    Set<Pair> unused;
    PartnerLinks network(links);
    EXPECT(network.hasPerfectLinks(unused));
}

PROVIDED_TEST("hasPerfectMatching works on a world with two linked people, and produces output.") {
    /* This world is a pair of people A and B. There should be a perfect matching.
     *
     *               A -- B
     *
     * The matching is {A, B}
     */
    auto links = fromLinks({
        { "A", "B" }
    });

    Set<Pair> expected = {
        { "A", "B" }
    };

    Set<Pair> matching;
    PartnerLinks network(links);
    EXPECT(network.hasPerfectLinks(matching));
    EXPECT_EQUAL(matching, expected);
}

PROVIDED_TEST("hasPerfectMatching works on a triangle of people.") {
    /* Here's the world:
     *
     *               A --- B
     *                \   /
     *                 \ /
     *                  C
     *
     * There is no perfect matching here, unfortunately.
     */
    auto links = fromLinks({
        { "A", "B" },
        { "B", "C" },
        { "C", "A" }
    });

    Set<Pair> unused;
    PartnerLinks network(links);
    EXPECT(!network.hasPerfectLinks(unused));
}

PROVIDED_TEST("hasPerfectMatching works on a square of people.") {
    /* Here's the world:
     *
     *               A --- B
     *               |     |
     *               |     |
     *               D --- C
     *
     * There are two different perfect matching here: AB / CD, and AD/BD.
     * Either will work.
     */
    auto links = fromLinks({
        { "A", "B" },
        { "B", "C" },
        { "C", "D" },
        { "D", "A" }
    });

    Set<Pair> unused;
    PartnerLinks network(links);
    EXPECT(network.hasPerfectLinks(unused));
}

PROVIDED_TEST("hasPerfectMatching works on a square of people, and produces output.") {
    /* Here's the world:
     *
     *               A --- B
     *               |     |
     *               |     |
     *               C --- D
     *
     * There are two different perfect matching here: AB / CD, and AC/BC.
     * Either will work.
     */
    auto links = fromLinks({
        { "A", "B" },
        { "B", "C" },
        { "C", "D" },
        { "D", "A" }
    });

    Set<Pair> matching;
    PartnerLinks network(links);
    EXPECT(network.hasPerfectLinks(matching));
    EXPECT(isPerfectMatching(links, matching));
}

PROVIDED_TEST("hasPerfectMatching works on a pentagon of people.") {
    /* Here's the world:
     *
     *               A --- B
     *             /       |
     *            E        |
     *             \       |
     *               D --- C
     *
     * There is no perfect matching here, since the cycle has odd
     * length.
     */
    auto links = fromLinks({
        { "A", "B" },
        { "B", "C" },
        { "C", "D" },
        { "D", "E" },
        { "E", "A" }
    });

    Set<Pair> unused;
    PartnerLinks network(links);
    EXPECT(!network.hasPerfectLinks(unused));
}

PROVIDED_TEST("hasPerfectMatching works on a line of six people.") {
    /* Because Map and Set internally store items in sorted order, the order
     * in which you iterate over people when making decisions is sensitive
     * to the order of those peoples' names. This test looks at a group like
     * this one, trying out all possible orderings of peoples' names:
     *
     *
     *
     *        * -- * -- * -- * -- * -- *
     *
     *
     *
     * There are 6! = 720 possible permutations of the ordering of these six
     * people. If your code is able to solve the problem correctly for all of
     * those orderings, there's a good chance that you're correctly tracking
     * who is matched at each step. On the other hand, if your code runs into
     * issues here, it may indicate that there's a bug in how you mark who's
     * paired and who isn't.
     */
    Vector<std::string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        Map<std::string, Set<std::string>> links = fromLinks({
            { people[0], people[1] },
            { people[1], people[2] },
            { people[2], people[3] },
            { people[3], people[4] },
            { people[4], people[5] }
        });

        Set<Pair> matching;
        PartnerLinks network(links);
        EXPECT(network.hasPerfectLinks(matching));
        EXPECT(isPerfectMatching(links, matching));
    } while (next_permutation(people.begin(), people.end()));
}

PROVIDED_TEST("hasPerfectMatching works on a more complex negative example.") {
    /* Because Map and Set internally store items in sorted order, the order
     * in which you iterate over people when making decisions is sensitive
     * to the order of those peoples' names. This test looks at a group like
     * this one, trying out all possible orderings of peoples' names:
     *
     *
     *         *        *
     *          \      /
     *           * -- *
     *          /      \
     *         *        *
     *
     * There are 6! = 720 possible permutations of the ordering of these six
     * people. If your code is able to solve the problem correctly for all of
     * those orderings, there's a good chance that you're correctly tracking
     * who is matched at each step. On the other hand, if your code runs into
     * issues here, it may indicate that there's a bug in how you mark who's
     * paired and who isn't.
     */
    Vector<std::string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        Map<std::string, Set<std::string>> links = fromLinks({
            { people[0], people[2] },
            { people[1], people[2] },
            { people[2], people[3] },
            { people[3], people[4] },
            { people[3], people[5] },
        });

        Set<Pair> matching;
        PartnerLinks network(links);
        EXPECT(!network.hasPerfectLinks(matching));
    } while (next_permutation(people.begin(), people.end()));
}

PROVIDED_TEST("hasPerfectMatching works on a more complex positive example.") {
    /* Because Map and Set internally store items in sorted order, the order
     * in which you iterate over people when making decisions is sensitive
     * to the order of those peoples' names. This test looks at a group like
     * this one, trying out all possible orderings of peoples' names:
     *
     *               *
     *               |
     *               *
     *              / \
     *             *---*
     *            /     \
     *           *       *
     *
     * There are 6! = 720 possible permutations of the ordering of these
     * people. If your code is able to solve the problem correctly for all of
     * those orderings, there's a good chance that you're correctly tracking
     * who is matched at each step. On the other hand, if your code runs into
     * issues here, it may indicate that there's a bug in how you mark who's
     * paired and who isn't.
     */
    Vector<std::string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        Map<std::string, Set<std::string>> links = fromLinks({
            { people[0], people[1] },
            { people[1], people[2] },
            { people[2], people[3] },
            { people[3], people[1] },
            { people[2], people[4] },
            { people[3], people[5] },
        });

        Set<Pair> matching;
        PartnerLinks network(links);
        EXPECT(network.hasPerfectLinks(matching));
        EXPECT(isPerfectMatching(links, matching));
    } while (next_permutation(people.begin(), people.end()));
}

PROVIDED_TEST("hasPerfectMatching works on a caterpillar.") {
    /* Because Map and Set internally store items in sorted order, the order
     * in which you iterate over people when making decisions is sensitive
     * to the order of those peoples' names. This test looks at a group like
     * this one, trying out all possible orderings of peoples' names:
     *
     *         *---*---*
     *         |   |   |
     *         *   *   *
     *
     * There are 6! = 720 possible permutations of the ordering of these six
     * people. If your code is able to solve the problem correctly for all of
     * those orderings, there's a good chance that you're correctly tracking
     * who is matched at each step. On the other hand, if your code runs into
     * issues here, it may indicate that there's a bug in how you mark who's
     * paired and who isn't.
     */
    Vector<std::string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        Map<std::string, Set<std::string>> links = fromLinks({
            { people[0], people[1] },
            { people[1], people[2] },
            { people[0], people[3] },
            { people[1], people[4] },
            { people[2], people[5] },
        });

        Set<Pair> matching;
        PartnerLinks network(links);
        EXPECT(network.hasPerfectLinks(matching));
        EXPECT(isPerfectMatching(links, matching));
    } while (next_permutation(people.begin(), people.end()));
}

PROVIDED_TEST("hasPerfectMatching stress test: negative example (should take under a second).") {
    /* Here, we're giving a "caterpillar" of people, like this:
     *
     *    *   *   *   *     *   *
     *    |   |   |   |     |   |
     *    *---*---*---* ... *---*
     *    |   |   |   |     |   |
     *    *   *   *   *     *   *
     *
     * This doesn't have a perfect matching, However, it may take some searching
     * to confirm this is the case. At this size, however, it should be
     * almost instanteous to find the solution, since the search space is fairly
     * small and most "wrong" decisions can be detected quickly.
     *
     * On the other hand, if your implementation repeatedly constructs the same
     * matchings over and over again, or keeps exploring even when a person who
     * couldn't be paired with the current setup is found, then the number of
     * options you need to consider will be too large for your computer to handle
     * in any reasonable time.
     *
     * If you're passing the other tests and this test hangs, double-check your
     * code to make sure you aren't repeatedly constructing the same matchings
     * multiple times.
     */

    /* Number of "body segments". */
    const int kRowSize = 10;

    Vector<Pair> links;
    for (int i = 0; i < kRowSize - 1; i++) {
        links.add({ std::to_string(i), std::to_string(i + 1) });
    }
    for (int i = 0; i < kRowSize; i++) {
        links.add({ std::to_string(i), std::to_string(i + kRowSize) });
    }
    for (int i = 0; i < kRowSize; i++) {
        links.add({ std::to_string(i), std::to_string(i + 2 * kRowSize) });
    }

    Set<Pair> matching;
    PartnerLinks network(fromLinks(links));
    EXPECT(!network.hasPerfectLinks(matching));
}

PROVIDED_TEST("hasPerfectMatching stress test: positive example (should take under a second).") {
    /* Here, we're giving a "millipede" of people, like this:
     *
     *    *---*---*---* ... *---*
     *    |   |   |   |     |   |
     *    *   *   *   *     *   *
     *
     * This always has a perfect matching, which is found by pairing each person
     * with the person directly below them. However, it may take some searching
     * to find this particular configuration. At this size, however, it should be
     * almost instanteous to find the solution, since the search space is fairly
     * small and most "wrong" decisions can be detected quickly.
     *
     * On the other hand, if your implementation repeatedly constructs the same
     * matchings over and over again, or keeps exploring even when a person who
     * couldn't be paired with the current setup is found, then the number of
     * options you need to consider will be too large for your computer to handle
     * in any reasonable time.
     *
     * If you're passing the other tests and this test hangs, double-check your
     * code to make sure you aren't repeatedly constructing the same matchings
     * multiple times.
     */

    /* Number of "body segments". */
    const int kRowSize = 10;

    Vector<Pair> links;
    for (int i = 0; i < kRowSize - 1; i++) {
        links.add({ std::to_string(i), std::to_string(i + 1) });
    }
    for (int i = 0; i < kRowSize; i++) {
        links.add({ std::to_string(i), std::to_string(i + kRowSize) });
    }

    Set<Pair> matching;
    PartnerLinks network(fromLinks(links));
    EXPECT(network.hasPerfectLinks(matching));
    EXPECT(isPerfectMatching(fromLinks(links), matching));
}
