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
#include <cmath>
#include <limits.h>
#include "PartnerLinks.h"

namespace DancingLinks {


/* * * * * * * * * * * * *    Free Functions for DancingLinks Namespace   * * * * * * * * * * * * */


bool hasExactCover(PartnerLinks& links, std::set<Pair>& selectedOptions) {
    return links.hasPerfectLinks(selectedOptions);
}

std::vector<std::set<Pair>> getAllExactCovers(PartnerLinks& links) {
    return links.getAllPerfectLinks();
}

std::set<Pair> getMaxWeightMatching(PartnerLinks& links) {
    return links.getMaxWeightMatching();
}


/* * * * * * * * * * * * *  Perfect Matching Algorithm X via Dancing Links  * * * * * * * * * * * */


bool PartnerLinks::hasPerfectLinks(std::set<Pair>& pairs) {
    // Mathematically impossible perfect links no work necessary.
    if (hasSingleton_ || numPeople_ % 2 != 0) {
        return false;
    }
    return isPerfectMatching(pairs);
}

bool PartnerLinks::isPerfectMatching(std::set<Pair>& pairs) {
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
            pairs.insert(match);
            uncoverPairing(cur);
            return true;
        }
        // Selecting to partner chosenPerson with this option did not work out. Retry other option.
        uncoverPairing(cur);
    }
    return false;
}

std::vector<std::set<Pair>> PartnerLinks::getAllPerfectLinks() {
    if (hasSingleton_ || numPeople_ % 2 != 0) {
        return {};
    }
    /* Going with a pass by reference method here becuase I like the "no copy recursion" principle
     * behind Knuth's dancing links. I can profile to see if this is any faster than creating
     * copies of sets through the stack frames and returning the desired result as the return type.
     */
    std::vector<std::set<Pair>> result = {};
    std::set<Pair> soFar = {};
    fillPerfectMatchings(soFar, result);
    return result;
}

void PartnerLinks::fillPerfectMatchings(std::set<Pair>& soFar, std::vector<std::set<Pair>>& result) {
    if (table_[0].right == 0) {
        result.push_back(soFar);
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
        soFar.insert(match);

        fillPerfectMatchings(soFar, result);

        uncoverPairing(cur);
        soFar.erase(match);
    }
}

int PartnerLinks::choosePerson() const {
    for (int cur = table_[0].right; cur != 0; cur = table_[cur].right) {
        // Someone has become inaccessible due to other matches.
        if (links_[cur].topOrLen == 0) {
            return -1;
        }
    }
    return table_[0].right;
}

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

inline int PartnerLinks::toPairIndex(int indexInPair) {
    // There are only ever two people in an option so this is a safe increment/decrement.
    if (links_[++indexInPair].topOrLen <= 0) {
        indexInPair -= 2;
    }
    return indexInPair;
}


/* * * * * * * * * * * * *  Weighted Matching Algorithm X via Dancing Links  * * ** * * * * * * * */


std::set<Pair> PartnerLinks::getMaxWeightMatching() {
    if (!isWeighted_) {
        error("Asking for max weight matching of a graph with no weight information provided.\n"
              "For weighted graphs provide a std::map<string,std::map<string,int>> representing a person\n"
              "and the weights of their preferred connections to the constructor.");
    }
    /* In the spirit of "no copy" recursion by Knuth, we can just fill and remove from one set and
     * record the best snapshot of pairings in the winner set. Have to profile to see if adding
     * and removing from a set during recursion is faster than creating new sets in the stack frames
     * of recursive calls. Possible space vs speed tradeoff?
     */
    std::pair<int,std::set<Pair>> soFar = {};
    std::pair<int,std::set<Pair>> winner = {};
    fillWeights(soFar, winner);
    return winner.second;
}

void PartnerLinks::fillWeights(std::pair<int,std::set<Pair>>& soFar, std::pair<int,std::set<Pair>>& winner) {
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
        soFar.second.insert(match.second);

        // Go explore every weight that matching this pair produces
        fillWeights(soFar, winner);

        // The winner pair will copy in the weight and std::set if its the best so far.
        if (soFar.first > winner.first) {
            winner = soFar;
        }

        // Prepare to explore the next options. Cleanup links and remove previous choice from pair.
        uncoverPairing(cur);
        soFar.first -= match.first;
        soFar.second.erase(match.second);
    }
}

int PartnerLinks::chooseWeightedPerson() const {
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


PartnerLinks::PartnerLinks(const std::map<std::string, std::set<std::string>>& possibleLinks)
    : table_(),
      links_(),
      numPeople_(0),
      numPairings_(0),
      hasSingleton_(false),
      isWeighted_(false) {

    std::unordered_map<std::string, int> columnBuilder = {};

    initializeHeaders(possibleLinks, columnBuilder);

    // Begin building the rows with the negative spacer tiles and the subsequent columns.
    int index = links_.size();
    int spacerTitle = -1;

    std::set<Pair> seenPairs = {};
    for (const auto& p : possibleLinks) {

        const std::set<std::string>& preferences = p.second;
        if (preferences.empty()) {
            hasSingleton_ = true;
        }
        setPerfectPairs(p.first, preferences, columnBuilder, seenPairs, index, spacerTitle);
    }
    links_.push_back({INT_MIN, index - 2, INT_MIN});
}

PartnerLinks::PartnerLinks(const std::map<std::string, std::map<std::string, int>>& possibleLinks)
    : table_(),
      links_(),
      numPeople_(0),
      numPairings_(0),
      hasSingleton_(false),
      isWeighted_(true) {

    std::unordered_map<std::string, int> columnBuilder = {};

    initializeHeaders(possibleLinks, columnBuilder);

    // Begin building the rows with the negative spacer tiles and the subsequent columns.
    int index = links_.size();

    std::set<Pair> seenPairs = {};
    for (const auto& p : possibleLinks) {

        const std::map<std::string,int>& preferences = p.second;
        if (preferences.empty()) {
            hasSingleton_ = true;
        }
        setWeightedPairs(p.first, preferences, columnBuilder, seenPairs, index);
    }
    links_.push_back({INT_MIN, index - 2, INT_MIN});
}

void PartnerLinks::initializeHeaders(const std::map<std::string, std::set<std::string>>& possibleLinks,
                                     std::unordered_map<std::string,int>& columnBuilder) {
    // std::set up the headers first. Lookup table and first N headers in links.
    table_.push_back({"", 0, 1});
    links_.push_back({});
    int index = 1;
    for (const auto& p : possibleLinks) {

        columnBuilder[p.first] = index;

        table_.push_back({p.first, index - 1, index + 1});
        table_[0].left++;
        // Add the first headers for the item vector. They need count up and down.
        links_.push_back({0, index, index});

        numPeople_++;
        index++;
    }
    table_[table_.size() - 1].right = 0;
}

void PartnerLinks::initializeHeaders(const std::map<std::string, std::map<std::string,int>>& possibleLinks,
                                     std::unordered_map<std::string,int>& columnBuilder) {
    // std::set up the headers first. Lookup table and first N headers in links.
    table_.push_back({"", 0, 1});
    links_.push_back({});
    int index = 1;
    for (const auto& p : possibleLinks) {

        columnBuilder[p.first] = index;

        table_.push_back({p.first, index - 1, index + 1});
        table_[0].left++;
        // Add the first headers for the item vector. They need count up and down.
        links_.push_back({0, index, index});

        numPeople_++;
        index++;
    }
    table_[table_.size() - 1].right = 0;
}

void PartnerLinks::setPerfectPairs(const std::string& person,
                                   const std::set<std::string>& preferences,
                                   std::unordered_map<std::string,int>& columnBuilder,
                                   std::set<Pair>& seenPairs,
                                   int& index,
                                   int& spacerTitle) {
    for (const auto& pref : preferences) {
        Pair newPair = {person, pref};

        if (!seenPairs.count(newPair)) {
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

            // Pairings are bidirectional but might appear multiple times in std::map. Track here.
            seenPairs.insert(newPair);
            index++;
            spacerTitle--;
        }
    }
}

void PartnerLinks::setWeightedPairs(const std::string& person,
                                    const std::map<std::string,int>& preferences,
                                    std::unordered_map<std::string,int>& columnBuilder,
                                    std::set<Pair>& seenPairs,
                                    int& index) {
    for (const auto& pref : preferences) {
        Pair newPair = {person, pref.first};

        if (!seenPairs.count(newPair) && pref.second >= 0) {
            numPairings_++;
            // Weight is negative so we know we are on a spacer tile when found.
            links_.push_back({-pref.second,     // Negative weight of the partnership
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
            seenPairs.insert(newPair);
            index++;
        }
    }
}

} // namespace DancingLinks
