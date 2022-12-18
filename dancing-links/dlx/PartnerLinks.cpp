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
#include <limits.h>
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
    if (hasSingleton_ || numPeople_ % 2 != 0) {
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
    if (table_[0].right == 0) {
        return true;
    }
    // If our previous pairings led to someone that can no longer be reached stop recursion.
    int chosenPerson = choosePerson();
    if (chosenPerson == -1) {
        return false;
    }

    for (int cur = links_[chosenPerson].down; cur != chosenPerson; cur = links_[cur].down) {

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
 * @brief getAllPerfectLinks  retrieves every configuration of a graph of people that will
 *                            produce a Perfect Matching. This is not every possible pairing
 *                            of people, rather, only those pairings that will produce
 *                            Perfect Matching.
 * @return                    set of sets. Each set is a unique Perfect Matching configuration.
 */
Vector<Set<Pair>> PartnerLinks::getAllPerfectLinks() {
    if (hasSingleton_ || numPeople_ % 2 != 0) {
        return {};
    }
    /* Going with a pass by reference method here becuase I like the "no copy recursion" principle
     * behind Knuth's dancing links. I can profile to see if this is any faster than creating
     * copies of sets through the stack frames and returning the desired result as the return type.
     */
    Vector<Set<Pair>> result = {};
    Set<Pair> soFar = {};
    fillPerfectMatchings(soFar, result);
    return result;
}

/**
 * @brief fillPerfectMatchings  finds all available Perfect Matchings for a network. Fills the
 *                              sets that complete this task as pass by reference output
 *                              parameters. Every Perfect Matching configuration is unique.
 * @param soFar                 the helper set we insert/delete from as we build Matchings.
 * @param result                the output parameter we fill with any Perfect Matchings we find.
 */
void PartnerLinks::fillPerfectMatchings(Set<Pair>& soFar, Vector<Set<Pair>>& result) {
    if (table_[0].right == 0) {
        result.add(soFar);
        return;
    }

    int chosen = choosePerson();
    // A person is left by themselves so backtrack to better choice.
    if (chosen == -1) {
        return;
    }

    /* The chosen person must be included in any Perfect Matching configuration or else it would
     * not be Perfect Matching. We will explore all ways to pair them off and include each in
     * the output parameter. Some ways might leave loners and we won't include those.
     */

    for (int cur = links_[chosen].down; cur != chosen; cur = links_[cur].down) {

        Pair match = coverPairing(cur);
        soFar += match;

        fillPerfectMatchings(soFar, result);

        uncoverPairing(cur);
        soFar -= match;
    }
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
    for (int cur = table_[0].right; cur != 0; cur = table_[cur].right) {
        // Someone has become inaccessible due to other matches.
        if (links_[cur].topOrLen == 0) {
            return -1;
        }
    }
    return table_[0].right;
}

/**
 * @brief coverPairing  when we cover a pairing in a Perfect Matching we report back the
 *                      partnership as a Pair. This is helpful for proving the recursive
 *                      selections are correct. This selects the option beneath the index given.
 *                      Covering a pair means that both people will dissapear from all other
 *                      partnerships they could have matched with, eliminating those options.
 * @param indexInPair   the index of the pair we want to cover.
 * @return              the pair we have created by selecting this option.
 */
Pair PartnerLinks::coverPairing(int indexInPair) {

    /* We now must cover the two people in this option in the lookup table. Then go through all
     * other options and eliminate the other pairings in which each appears because they are paired
     * off and therefore no longer accessible to other people that want to pair with them.
     */

    personName p1 = table_[links_[indexInPair].topOrLen];
    table_[p1.right].left = p1.left;
    table_[p1.left].right = p1.right;

    // p1 needs to dissapear from all other pairings.
    hidePersonPairings(indexInPair);

    // In case I ever apply a selection heuristic, partner might not be to the right.
    indexInPair = toPairIndex(indexInPair);

    personName p2 = table_[links_[indexInPair].topOrLen];
    table_[p2.right].left = p2.left;
    table_[p2.left].right = p2.right;

    // p2 needs to dissapear from all other pairings.
    hidePersonPairings(indexInPair);

    return {p1.name, p2.name};
}

/**
 * @brief uncoverPairing  uncovers a pairing that was hidden in Perfect Matching or Max Weight
 *                        Matching. The uncovering process is identical across both algorithms.
 *                        Be sure to provide the same index as the option you covered.
 * @param indexInPair     the same index as the index that was covered.
 */
void PartnerLinks::uncoverPairing(int indexInPair) {

    personName p1 = table_[links_[indexInPair].topOrLen];
    table_[p1.left].right = links_[indexInPair].topOrLen;
    table_[p1.right].left = links_[indexInPair].topOrLen;

    unhidePersonPairings(indexInPair);

    indexInPair = toPairIndex(indexInPair);

    personName p2 = table_[links_[indexInPair].topOrLen];
    table_[p2.left].right = links_[indexInPair].topOrLen;
    table_[p2.right].left = links_[indexInPair].topOrLen;

    unhidePersonPairings(indexInPair);
}

/**
 * @brief hidePersonPairings  hides other options that include the specified person. This only
 *                            hides pairings including a single person. If you want to hide both
 *                            people in a pair, you must use this on both people.
 * @param start               the starting node of the person we are hide in the selected option
 * @param indexInPair         the index of the person we are hiding in the selected option.
 */
void PartnerLinks::hidePersonPairings(int indexInPair) {
    for (int i = links_[indexInPair].down; i != indexInPair; i = links_[i].down) {
         // We need this guard to prevent splicing while on a column header.
        if (i > links_[indexInPair].topOrLen) {
            // In case the other partner is to the left, just decrement index to go left.
            personLink cur = links_[toPairIndex(i)];
            links_[cur.up].down = cur.down;
            links_[cur.down].up = cur.up;
            links_[cur.topOrLen].topOrLen--;
        }
    }
}

/**
 * @brief unhidePersonPairings  undoes the work of hidePersonPairings if given the same start
 *                              and index.
 * @param start                 the node of the person we unhide in the selected option.
 * @param indexInPair           the index of the person we are unhiding in the selected option.
 */
void PartnerLinks::unhidePersonPairings(int indexInPair) {
    // The direction does not truly matter but I distinguish this from hide by going upwards.
    for (int i = links_[indexInPair].up; i != indexInPair; i = links_[i].up) {
        if (i > links_[indexInPair].topOrLen) {
            int partnerIndex = toPairIndex(i);
            personLink cur = links_[partnerIndex];
            links_[cur.up].down = partnerIndex;
            links_[cur.down].up = partnerIndex;
            links_[cur.topOrLen].topOrLen++;
        }
    }
}

/**
 * @brief toPairIndex  helper function to increment the index to the next partner. We might
 *                     need to move left or right.
 * @param indexInPair  the index we take by reference to advance.
 */
inline int PartnerLinks::toPairIndex(int indexInPair) {
    // There are only ever two people in an option so this is a safe increment/decrement.
    if (links_[++indexInPair].topOrLen <= 0) {
        indexInPair -= 2;
    }
    return indexInPair;
}


/* * * * * * * * * * * * *  Weighted Matching Algorithm X via Dancing Links  * * ** * * * * * * * */


/**
 * @brief getMaxWeightMatching  determines the Max Weight Matching of a PartnerLinks matrix. A
 *                              Max Weight Matching is the greatest sum of edge weights we can
 *                              acheive by partnering up people in a network.
 * @return                      the set representing the Max Weight Matching that we found.
 */
Set<Pair> PartnerLinks::getMaxWeightMatching() {
    if (!isWeighted_) {
        error("Asking for max weight matching of a graph with no weight information provided.\n"
              "For weighted graphs provide a Map<string,Map<string,int>> representing a person\n"
              "and the weights of their preferred connections to the constructor.");
    }
    /* In the spirit of "no copy" recursion by Knuth, we can just fill and remove from one set and
     * record the best snapshot of pairings in the winner set. Have to profile to see if adding
     * and removing from a set during recursion is faster than creating new sets in the stack frames
     * of recursive calls. Possible space vs speed tradeoff?
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
    if (table_[0].right == 0) {
        return;
    }

    int chosen = chooseWeightedPerson();
    if (chosen == -1) {
        return;
    }
    // Explore every possibility without this person to see if a heavier matching exists without.
    hidePerson(chosen);
    fillWeights(soFar, winner);
    unhidePerson(chosen);

    // Now loop through every possible option for every combination of people available.
    for (int cur = links_[chosen].down; cur != chosen; cur = links_[cur].down) {

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
    for (int cur = table_[0].right; cur != head; cur = table_[cur].right) {
        // Take the first available person.
        if (links_[cur].topOrLen != 0) {
            return cur;
        }
    }
    // Every person is alone so no more weights to explore in this recursive branch.
    return -1;
}

/**
 * @brief hidePerson   to generate all possible pairings in any pairing algorithm, we need to
 *                     include every person in future possible pairings and exclude them. To
 *                     exclude a person, we will cover only that person. Instead of eliminating
 *                     every option that includes both people in a Pair, we only eliminate
 *                     other appearances of this individual in other pairings. It is a subtle
 *                     but important difference from covering a pairing.
 * @param indexInPair  the index of the person we cover. Chooses option below this index.
 */
void PartnerLinks::hidePerson(int indexInPair) {
    indexInPair = links_[indexInPair].down;

    personName p1 = table_[links_[indexInPair].topOrLen];
    table_[p1.right].left = p1.left;
    table_[p1.left].right = p1.right;

    // Only hide pairings for this person.
    hidePersonPairings(indexInPair);

    indexInPair = toPairIndex(indexInPair);
    // Partner will only disapear in this instance of the pairing, not all other instances.
    personLink cur = links_[indexInPair];
    links_[cur.up].down = cur.down;
    links_[cur.down].up = cur.up;
    links_[cur.topOrLen].topOrLen--;
}

/**
 * @brief unhidePerson  undoes the work of covering a person, reinstating all possible pairings
 *                      that include this person. Will undo the same option chosen in
 *                      hidePerson() if given the same index.
 * @param indexInPair   the index of the person to uncover. Chooses option below this index.
 */
void PartnerLinks::unhidePerson(int indexInPair) {
    indexInPair = links_[indexInPair].down;
    personName p1 = table_[links_[indexInPair].topOrLen];
    table_[p1.left].right = links_[indexInPair].topOrLen;
    table_[p1.right].left = links_[indexInPair].topOrLen;

    unhidePersonPairings(indexInPair);

    indexInPair = toPairIndex(indexInPair);
    personLink cur = links_[indexInPair];
    links_[cur.up].down = indexInPair;
    links_[cur.down].up = indexInPair;
    links_[cur.topOrLen].topOrLen++;
}

/**
 * @brief coverWeightedPair  when we cover a weighted pair in Max Weight Matching we should
 *                           report back the weight of the pair and the names of the people in
 *                           the Pair. This is helpful for proof of correct choices. Covering a
 *                           Pair means that both people disappear from all other possible
 *                           pairings, thus eliminating those options.
 * @param indexInPair        the index of the weighted pair we cover. Chooses option below this.
 * @return                   an std::pair of the weight of the pair and their names.
 */
std::pair<int,Pair> PartnerLinks::coverWeightedPair(int indexInPair) {

    personName p1 = table_[links_[indexInPair].topOrLen];
    table_[p1.right].left = p1.left;
    table_[p1.left].right = p1.right;

    // p1 needs to dissapear from all other pairings.
    hidePersonPairings(indexInPair);


    // We can pick up the weight for this pairing in a O(1) sweep to report back.
    std::pair<int,Pair> result = {};
    if (links_[indexInPair + 1].topOrLen < 0) {
        result.first = std::abs(links_[indexInPair - 2].topOrLen);
        indexInPair--;
    } else {
        result.first = std::abs(links_[indexInPair - 1].topOrLen);
        indexInPair++;
    }

    personName p2 = table_[links_[indexInPair].topOrLen];
    table_[p2.right].left = p2.left;
    table_[p2.left].right = p2.right;
    // p2 needs to dissapear from all other pairings.
    hidePersonPairings(indexInPair);

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
PartnerLinks::PartnerLinks(const Map<std::string, Set<std::string>>& possibleLinks) :
                           table_({}),
                           links_({}),
                           numPeople_(0),
                           numPairings_(0),
                           hasSingleton_(false),
                           isWeighted_(false) {

    HashMap<std::string, int> columnBuilder = {};

    initializeHeaders(possibleLinks, columnBuilder);

    // Begin building the rows with the negative spacer tiles and the subsequent columns.
    int index = links_.size();
    int spacerTitle = -1;

    Set<Pair> seenPairs = {};
    for (const auto& p : possibleLinks) {

        const Set<std::string>& preferences = possibleLinks[p];
        if (preferences.isEmpty()) {
            hasSingleton_ = true;
        }
        setPerfectPairs(p, preferences, columnBuilder, seenPairs, index, spacerTitle);
    }
    links_.push_back({INT_MIN, index - 2, INT_MIN});
}

/**
 * @brief PartnerLinks   the constructor for a world intended to check for Max Weight Matching.
 *                       Provide a map of the person, and the map of the partners they are
 *                       willing to pair with and the weights of each partnership. If a negative
 *                       weight is given for a partnership, we treat that partnership as if it
 *                       does not exist.
 * @param possibleLinks  the map of people and map of partners and weights.
 */
PartnerLinks::PartnerLinks(const Map<std::string, Map<std::string, int>>& possibleLinks) :
                           table_({}),
                           links_({}),
                           numPeople_(0),
                           numPairings_(0),
                           hasSingleton_(false),
                           isWeighted_(true) {

    HashMap<std::string, int> columnBuilder = {};

    initializeHeaders(possibleLinks, columnBuilder);

    // Begin building the rows with the negative spacer tiles and the subsequent columns.
    int index = links_.size();

    Set<Pair> seenPairs = {};
    for (const auto& p : possibleLinks) {

        const Map<std::string,int>& preferences = possibleLinks[p];
        if (preferences.isEmpty()) {
            hasSingleton_ = true;
        }
        setWeightedPairs(p, preferences, columnBuilder, seenPairs, index);
    }
    links_.push_back({INT_MIN, index - 2, INT_MIN});
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
    table_.push_back({"", 0, 1});
    links_.push_back({});
    int index = 1;
    for (const auto& p : possibleLinks) {

        columnBuilder[p] = index;

        table_.push_back({p, index - 1, index + 1});
        table_[0].left++;
        // Add the first headers for the item vector. They need count up and down.
        links_.push_back({0, index, index});

        numPeople_++;
        index++;
    }
    table_[table_.size() - 1].right = 0;
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
    table_.push_back({"", 0, 1});
    links_.push_back({});
    int index = 1;
    for (const auto& p : possibleLinks) {

        columnBuilder[p] = index;

        table_.push_back({p, index - 1, index + 1});
        table_[0].left++;
        // Add the first headers for the item vector. They need count up and down.
        links_.push_back({0, index, index});

        numPeople_++;
        index++;
    }
    table_[table_.size() - 1].right = 0;
}

/**
 * @brief setPerfectPairs  creates the internal rows and columns of the dancing links data
 *                         structure. Perfect pairs do not have any weight information and
 *                         options will simply be given a number from 1 to N number of matches.
 * @param person           the person who will set all possible matches for.
 * @param preferences      the set of all people this person is willing to pair with.
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
            numPairings_++;
            // Update the count for this column.
            links_.push_back({spacerTitle,     // Negative to mark spacer.
                           index - 2,       // First item in previous option
                           index + 2});     // Last item in current option
            index++;
            std::string sortedFirst = newPair.first();
            links_.push_back({links_[columnBuilder[sortedFirst]].down, index, index});

            // We can always access the column header with down field of last item.
            links_[links_[columnBuilder[sortedFirst]].down].topOrLen++;
            links_[links_[columnBuilder[sortedFirst]].down].up = index;
            // The current node is the new tail in a vertical circular linked list for an item.
            links_[index].up = columnBuilder[sortedFirst];
            links_[index].down = links_[columnBuilder[sortedFirst]].down;
            // Update the old tail to reflect the new addition of an item in its option.
            links_[columnBuilder[sortedFirst]].down = index;
            // Similar to a previous/current coding pattern but in an above/below column.
            columnBuilder[sortedFirst] = index;

            // Repeat the process. We only ever have two items in an option.
            index++;
            std::string sortedSecond = newPair.second();
            links_.push_back({links_[columnBuilder[sortedSecond]].down, index, index});
            links_[links_[columnBuilder[sortedSecond]].down].topOrLen++;
            links_[links_[columnBuilder[sortedSecond]].down].up = index;
            links_[index].up = columnBuilder[sortedSecond];
            links_[index].down = links_[columnBuilder[sortedSecond]].down;
            links_[columnBuilder[sortedSecond]].down = index;
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
 * @param preferences       the people this person can pair with and the weight of those pairs.
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
            numPairings_++;
            // Weight is negative so we know we are on a spacer tile when found.
            links_.push_back({-preferences[pref],     // Negative weight of the partnership
                              index - 2,              // First item in previous option
                              index + 2});            // Last item in current option
            index++;
            std::string sortedFirst = newPair.first();
            links_.push_back({links_[columnBuilder[sortedFirst]].down,index, index});

            // We can always access the column header with down field of last item.
            links_[links_[columnBuilder[sortedFirst]].down].topOrLen++;
            links_[links_[columnBuilder[sortedFirst]].down].up = index;
            // The current node is new tail in a vertical circular linked list for an item.
            links_[index].up = columnBuilder[sortedFirst];
            links_[index].down = links_[columnBuilder[sortedFirst]].down;
            // Update the old tail to reflect the new addition of an item in its option.
            links_[columnBuilder[sortedFirst]].down = index;
            // Similar to a previous/current coding pattern but in an above/below column.
            columnBuilder[sortedFirst] = index;

            // Repeat the process. We only ever have two items in an option.
            index++;
            std::string sortedSecond = newPair.second();
            links_.push_back({links_[columnBuilder[sortedSecond]].down, index, index});
            links_[links_[columnBuilder[sortedSecond]].down].topOrLen++;
            links_[links_[columnBuilder[sortedSecond]].down].up = index;
            links_[index].up = columnBuilder[sortedSecond];
            links_[index].down = links_[columnBuilder[sortedSecond]].down;
            links_[columnBuilder[sortedSecond]].down = index;
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

std::ostream& operator<<(std::ostream&os, const std::vector<PartnerLinks::personLink>& links) {
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

std::ostream& operator<<(std::ostream&os, const std::vector<PartnerLinks::personName>& links) {
    os << "LOOKUP TABLE" << std::endl;
    for (const auto& item : links) {
        os << "{\"" << item.name << "\"," << item.left << "," << item.right << "}" << std::endl;
    }
    return os;
}

std::ostream& operator<<(std::ostream&os, const PartnerLinks& links) {
    os << "LOOKUP ARRAY" << std::endl;
    for (const auto& header : links.table_) {
        os << "{\"" << header.name << "\"," << header.left << "," << header.right << "},";
    }
    os << std::endl;
    os << "DLX ARRAY" << std::endl;
    for (const auto& item : links.links_) {
        if (item.topOrLen < 0) {
            os << std::endl;
        }
        os << "{" << item.topOrLen << "," << item.up << "," << item.down << "},";
    }
    os << std::endl;
    os << "Number of People: " << links.numPeople_ << std::endl;
    os << "Number of Pairs: " << links.numPairings_ << std::endl;
    os << "Has Singleton: " << links.hasSingleton_ << std::endl;
    os << "Is Weighted: " << links.isWeighted_ << std::endl;
    return os;
}


/* * * * * * * * * * * * * * * *      Test Cases Below this Point       * * * * * * * * * * * * * */


/* * * * * * * * * * * * * * * *            WEIGHTED MATCHING           * * * * * * * * * * * * * */


/* * * * * * * * * * * * * * * *             Initialization             * * * * * * * * * * * * * */


STUDENT_TEST("Empty is empty.") {
    const Map<std::string, Map<std::string,int>> provided = {};
    std::vector<PartnerLinks::personName> lookup {
        {"",0,0}
    };
    std::vector<PartnerLinks::personLink> dlxItems {
        {0,0,0},
        {INT_MIN,-1,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(dlxItems, matches.links_);
}

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
    std::vector<PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0}
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(dlxItems, matches.links_);
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
    std::vector<PartnerLinks::personName> lookup {
        {"",3,1},{"A",0,2},{"B",1,3},{"C",2,0},
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);
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
    std::vector<PartnerLinks::personName> lookup {
        {"",3,1},{"A",0,2},{"B",1,3},{"C",2,0},
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    matches.hidePerson(5);

    std::vector<PartnerLinks::personName> lookupCoverA {
        {"",3,2},{"A",0,2},{"B",0,3},{"C",2,0},
    };
    std::vector<PartnerLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxCoverA);

    matches.unhidePerson(5);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

}

STUDENT_TEST("All weights are unique so we can know that we report the right weight and pair.") {
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
    std::vector<PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0},
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    // Cover A, this will select option 2, partners are AB.
    std::pair<int,Pair> match = matches.coverWeightedPair(6);
    EXPECT_EQUAL(match.first, 3);
    EXPECT_EQUAL(match.second, {"A","B"});

    std::vector<PartnerLinks::personName> lookupCoverA {
        {"",4,3},{"A",0,2},{"B",0,3},{"C",0,4},{"D",3,0},
    };
    std::vector<PartnerLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);
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
    std::vector<PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(weights.links_, dlxItems);
    EXPECT_EQUAL(weights.table_, lookup);
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
    std::vector<PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(weights.links_, dlxItems);
    EXPECT_EQUAL(weights.table_, lookup);
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
    std::vector<PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
        EXPECT_EQUAL(weights.links_, dlxItems);
        EXPECT_EQUAL(weights.table_, lookup);
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


STUDENT_TEST("Empty is empty perfect matching.") {
    const Map<std::string, Set<std::string>> provided = {};
    std::vector<PartnerLinks::personName> lookup {
        {"",0,0}
    };
    std::vector<PartnerLinks::personLink> dlxItems {
        {0,0,0},
        {INT_MIN,-1,INT_MIN},
    };
    PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(dlxItems, matches.links_);
}

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
    std::vector<PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0}
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(dlxItems, matches.links_);
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
    std::vector<PartnerLinks::personName> lookup {
        {"",3,1},{"A",0,2},{"B",1,3},{"C",2,0},
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);
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
    std::vector<PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(dlxItems, matches.links_);
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
    std::vector<PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT(matches.hasSingleton_);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(dlxItems, matches.links_);
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
    std::vector<PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0},
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    // Cover A, this will select option 2, partners are AB.
    Pair match = matches.coverPairing(6);
    EXPECT_EQUAL(match, {"A","B"});

    std::vector<PartnerLinks::personName> lookupCoverA {
        {"",4,3},{"A",0,2},{"B",0,3},{"C",0,4},{"D",3,0},
    };
    std::vector<PartnerLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);
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
    std::vector<PartnerLinks::personName> lookup {
        {"",3,1},{"A",0,2},{"B",1,3},{"C",2,0},
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    Pair match = matches.coverPairing(5);
    EXPECT_EQUAL(match, {"A", "B"});
    std::vector<PartnerLinks::personName> lookupA {
        {"",3,3},{"A",0,2},{"B",0,3},{"C",0,0},
    };
    std::vector<PartnerLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);
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
    std::vector<PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0}
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    Pair match = matches.coverPairing(6);
    EXPECT_EQUAL(match, {"A", "B"});
    std::vector<PartnerLinks::personName> lookupCoverA {
        {"",4,3},{"A",0,2},{"B",0,3},{"C",0,4},{"D",3,0}
    };
    std::vector<PartnerLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);
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
    std::vector<PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0}
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    Pair match = matches.coverPairing(9);
    EXPECT_EQUAL(match, {"A", "D"});
    std::vector<PartnerLinks::personName> lookupCoverA {
        {"",3,2},{"A",0,2},{"B",0,3},{"C",2,0},{"D",3,0}
    };
    std::vector<PartnerLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);
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
    std::vector<PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(dlxItems, matches.links_);

    Pair match = matches.coverPairing(8);
    EXPECT_EQUAL(match, {"A","D"});

    std::vector<PartnerLinks::personName> lookupCoverA {
        {"",6,2},{"A",0,2},{"B",0,3},{"C",2,5},{"D",3,5},{"E",3,6},{"F",5,0},
    };
    std::vector<PartnerLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);
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
    std::vector<PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0}
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    Pair match = matches.coverPairing(9);
    EXPECT_EQUAL(match, {"A", "D"});
    std::vector<PartnerLinks::personName> lookupCoverA {
        {"",3,2},{"A",0,2},{"B",0,3},{"C",2,0},{"D",3,0}
    };
    std::vector<PartnerLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);

    match = matches.coverPairing(12);
    EXPECT_EQUAL(match, {"B", "C"});
    std::vector<PartnerLinks::personName> lookupCoverB {
        {"",0,0},{"A",0,2},{"B",0,3},{"C",0,0},{"D",3,0}
    };
    std::vector<PartnerLinks::personLink> dlxCoverB {
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
    EXPECT_EQUAL(lookupCoverB, matches.table_);
    EXPECT_EQUAL(dlxCoverB, matches.links_);
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
    std::vector<PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0},
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    // Cover A, partners are AB.
    Pair match = matches.coverPairing(6);
    EXPECT_EQUAL(match, {"A","B"});

    std::vector<PartnerLinks::personName> lookupCoverA {
        {"",4,3},{"A",0,2},{"B",0,3},{"C",0,4},{"D",3,0},
    };
    std::vector<PartnerLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);

    matches.uncoverPairing(6);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

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
    std::vector<PartnerLinks::personName> lookup {
        {"",3,1},{"A",0,2},{"B",1,3},{"C",2,0},
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    Pair match = matches.coverPairing(5);
    EXPECT_EQUAL(match, {"A", "B"});
    std::vector<PartnerLinks::personName> lookupA {
        {"",3,3},{"A",0,2},{"B",0,3},{"C",0,0},
    };
    std::vector<PartnerLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);

    matches.uncoverPairing(5);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);
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
    std::vector<PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0}
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    Pair match = matches.coverPairing(6);
    EXPECT_EQUAL(match, {"A", "B"});
    std::vector<PartnerLinks::personName> lookupCoverA {
        {"",4,3},{"A",0,2},{"B",0,3},{"C",0,4},{"D",3,0}
    };
    std::vector<PartnerLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);
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
    std::vector<PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0}
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    Pair match = matches.coverPairing(9);
    EXPECT_EQUAL(match, {"A", "D"});
    std::vector<PartnerLinks::personName> lookupCoverA {
        {"",3,2},{"A",0,2},{"B",0,3},{"C",2,0},{"D",3,0}
    };
    std::vector<PartnerLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);

    matches.uncoverPairing(9);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);
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
    std::vector<PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(dlxItems, matches.links_);

    Pair match = matches.coverPairing(8);
    EXPECT_EQUAL(match, {"A","D"});

    std::vector<PartnerLinks::personName> lookupCoverA {
        {"",6,2},{"A",0,2},{"B",0,3},{"C",2,5},{"D",3,5},{"E",3,6},{"F",5,0},
    };
    std::vector<PartnerLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);

    matches.uncoverPairing(8);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);
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
    std::vector<PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(dlxItems, matches.links_);

    Pair match = matches.coverPairing(8);
    EXPECT_EQUAL(match, {"A","D"});

    std::vector<PartnerLinks::personName> lookupCoverA {
        {"",6,2},{"A",0,2},{"B",0,3},{"C",2,5},{"D",3,5},{"E",3,6},{"F",5,0},
    };
    std::vector<PartnerLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);

    // Pair B C but that is a bad choice so we will have to uncover.
    match = matches.coverPairing(14);
    EXPECT_EQUAL(match, {"B","C"});

    std::vector<PartnerLinks::personName> lookupCoverB {
        {"",6,5},{"A",0,2},{"B",0,3},{"C",0,5},{"D",3,5},{"E",0,6},{"F",5,0},
    };
    std::vector<PartnerLinks::personLink> dlxCoverB {
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
    EXPECT_EQUAL(lookupCoverB, matches.table_);
    EXPECT_EQUAL(dlxCoverB, matches.links_);

    matches.uncoverPairing(14);
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);

    matches.uncoverPairing(8);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(dlxItems, matches.links_);
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
    std::vector<PartnerLinks::personName> lookup {
        {"",10,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,7},{"G",6,8},{"H",7,9},{"I",8,10},{"J",9,0},
    };
    std::vector<PartnerLinks::personLink> dlxItems {
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
        EXPECT_EQUAL(lookup, network.table_);
        EXPECT_EQUAL(dlxItems, network.links_);
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
    EXPECT(PartnerLinks(fromLinks(links)).hasPerfectLinks(matching));
    EXPECT(isPerfectMatching(fromLinks(links), matching));
}


/* * * * * * * * * * * * *     Bonus: Find All Perfect Matchings        * * * * * * * * * * * * * */


/* It would be nice to find a way to cycle through all perfect matchings as an option in the graph
 * viewer. As of now, there is no way to see the results of this function in any meaningful way.
 * Need to learn more about how drawing works for this application then possibly add to it.
 */

PROVIDED_TEST("getAllPerfectMatching works on a square of people, and produces output.") {
    /* Here's the world:
     *
     *               A --- B
     *               |     |
     *               |     |
     *               D --- C
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
    Vector<Set<Pair>> allMatches = {
        {{"A","B"}, {"D","C"}},
        {{"A","D"}, {"B","C"}}
    };
    PartnerLinks network(links);
    EXPECT_EQUAL(network.getAllPerfectLinks(), allMatches);
}

STUDENT_TEST("All possible pairings is huge, but all perfect matching configs is just 4.") {
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
    Vector<Set<Pair>> allMatches = {
        {{ "A", "B" }, { "C", "D" }, { "E", "F" }, { "G", "H" }, { "I", "J" }},
        {{ "A", "B" }, { "C", "D" }, { "E", "F" }, { "G", "J" }, { "H", "I" }},
        {{ "A", "J" }, { "B", "C" }, { "D", "E" }, { "F", "G" }, { "H", "I" }},
        {{ "A", "J" }, { "B", "E" }, { "C", "D" }, { "F", "G" }, { "H", "I" }},
    };
    PartnerLinks network(provided);
    EXPECT_EQUAL(network.getAllPerfectLinks(), allMatches);
}
