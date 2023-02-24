/**
 * Last Updated: Alex G. Lopez, CS106B-Winter, 2022.02.02
 * Assignment: Recursion to the Rescue, Part 2
 *
 * File: Matchmaker.cpp
 * -----------------------------------------------------
 * This program uses two different strategies to match partners. The first approach has us simply
 * finiding partners simply based off preference. If two people have each other as preferences for
 * matching, they should be matched. The recursive insight of this problem is that a recursive
 * boolean function can try out simple pair for you until one works or there are none. If you
 * simplify the data that you give the recursive function, the recursion is simple. The weighted
 * pairs approach optimizes groups for overall best weight scores. This approach ignores all
 * other factors to partner matching that do not involve high scores between partners. This means
 * people can end up without a partner.
 *
 * Implementation Notes:
 * -----------------------------------------------------
 * This assigment challenged me in how different the two approaches are to solving the partner
 * problem. I have found that I am much more comfortable with an include exclude approach to
 * recursion, especially if we are bringing back a tangible set data structure back with us as the
 * return statement. I struggle more with functions that ask us to try many combinations or
 * permutations, especially if they are boolean functions and have auxillary data structures for
 * output parameters. Overall, my strategy of stripping out partners for perfecte matching that
 * only have one partner was a brute force approach to simplify the problem. I wonder if it is
 * faster or slower to have done that.
 *
 * Citations:
 * -----------------------------------------------------------
 * 1. I went to LaIR debugging for this problem. While there I discussed with tutors how an include
 *    exclude pattern works and they helped me see that an exclude option rarely needs to go inside
 *    a loop. It can usually be the first thing you do once choosing your next object from a
 *    container because it is being excluded.
 *
 */
#include "Matchmaker.h"
#include "GUI/SimpleTest.h"
#include <unordered_set>
#include <climits>
using namespace std;

/* Function Prototypes */

// Perfect Pairs
bool hasPerfectMatching(const std::map<string, std::set<string>>& possibleLinks,
                        std::set<Pair>& matching);
bool hasPerfectMatchRec(const std::map<string, std::set<string>>& possibleLinks,
                          std::unordered_set<string>& linkKeys,
                          std::set<Pair> &perfectMatch);

std::vector<std::set<Pair>>
getAllPerfectMatchings(const std::map<std::string, std::set<std::string>>& possibleLinks);

void fillAllPerfectMatchings(const std::map<std::string, std::set<std::string>>& possibleLinks,
                               std::unordered_set<string>& toMatch,
                               std::set<Pair>& matching,
                               std::vector<set<Pair>>& result);

// Weighted pairs
std::set<Pair> maximumWeightMatching(const std::map<string, std::map<string, int>>& possibleLinks);
std::set<Pair> maxWeightRec(const std::map<string, std::map<string, int>> &possibleLinks,
                             std::unordered_set<string>& linkKeys,
                             std::set<Pair>& bestSet);
int sumWeights(const std::map<string, std::map<string, int>> &possibleLinks,
               const std::set<Pair> &pairsToWeigh);

/* Function Implementations */

/**
 * @brief hasPerfectMatching  takes in a std::map of possible partner connections and returns a boolean
 *                            representing whether or not everyone in the group can be paired based
 *                            on preference.
 * --------------------------------------------------------------
 * @param possibleLinks       the std::map representing the possible connections for partners
 * @param matching            the designated output paramater for this function. The perfect pairs.
 * @return                    true if everone was matched, false if not.
 */
bool hasPerfectMatching(const std::map<string, std::set<string>>& possibleLinks, std::set<Pair>& matching) {
    // Working with a set will make recursion and base case easier in our recursive function.
    std::unordered_set<string> toMatch = {};
    for (auto &key : possibleLinks) {
        toMatch.insert(key.first);
    }
    // Now hand our recursive function a map with all required pairs removed. See what we find.
    return hasPerfectMatchRec(possibleLinks, toMatch, matching);
}

/**
 * @brief hasPerfectMatchRec uses recursion to find the perfect matching of partners based on
 *                           preferences found in a reference map. There is an additional output
 *                           parameter of the pairs.
 * --------------------------------------------------------
 * @param possibleLinks      the reference map that tells us who people prefer for partners.
 * @param toMatch            the std::unordered_set representing each key in the map. Progresses recursion.
 * @param perfectMatch       the output parameter that carries the perfect matches if they are found.
 * @return                   true if matches are found false if there is not a perfect match.
 */
bool hasPerfectMatchRec(const std::map<string, std::set<string>>& possibleLinks,
                        std::unordered_set<string>& toMatch,
                        std::set<Pair> &perfectMatch) {
    // We have examined all possible partners.
    if (toMatch.empty()) {
        return true;
    }

    // We take pairs out two at a time, so if one is left, we fail.
    if (toMatch.size() == 1) {
        return false;
    }

    string needsMatch = *toMatch.begin();

    // Much smaller problem if we loop through the preferences not the entire map
    for (const string &preference : possibleLinks.at(needsMatch)) {
        // Someone in these preferences might have been taken out of the matches already during rec
        if (toMatch.count(preference)) {
            Pair match (needsMatch, preference);

            toMatch.erase(needsMatch);
            toMatch.erase(preference);
            if (hasPerfectMatchRec(possibleLinks,
                                   toMatch,
                                   perfectMatch)) {

                // One extra step for the output parameter.
                perfectMatch.insert(match);
                return true;
            }
            toMatch.insert(needsMatch);
            toMatch.insert(preference);
        }
    }
    // Cleanup our output parameter.
    perfectMatch = {};
    return false;
}

/**
 * @brief getAllPerfectMatchings  finds all possible perfect matching configurations with the
 *                                provided map of partners.
 * @param possibleLinks           the map of people and their partner preferences
 * @return                        a vector of sets of valid perfect matchings.
 */
std::vector<std::set<Pair>>
getAllPerfectMatchings(const std::map<std::string, std::set<std::string>>& possibleLinks) {
    std::unordered_set<string> toMatch = {};
    for (auto &key : possibleLinks) {
        toMatch.insert(key.first);
    }
    std::set<Pair> matching;
    std::vector<std::set<Pair>> result;
    fillAllPerfectMatchings(possibleLinks, toMatch, matching, result);
    return result;
}

/**
 * @brief fillAllPerfectMatchings  recursively finds all perfect matching configurations. This
 *                                 method uses copies of sets through stack frames to form perfect
 *                                 matchings and return all that are found. All matches are unique.
 * @param possibleLinks            the map of people and their preferences.
 * @param toMatch                  the hashset that helps us know when everyone is matched.
 * @param matching                 a helper set we build with partners to place in our return.
 * @return                         a std::vector of sets of valid perfect matches.
 */
void fillAllPerfectMatchings(const std::map<std::string, std::set<std::string>>& possibleLinks,
                               std::unordered_set<string>& toMatch,
                               std::set<Pair>& matching,
                               std::vector<std::set<Pair>>& result) {
    if (toMatch.empty()) {
        result.push_back(matching);
        return;
    }
    if (toMatch.size() == 1) {
        return;
    }

    string needsMatch = *toMatch.begin();

    for (const string& preference : possibleLinks.at(needsMatch)) {
        if (toMatch.count(preference)) {
            Pair match (needsMatch, preference);
            toMatch.erase(needsMatch);
            toMatch.erase(preference);
            matching.insert(match);
            fillAllPerfectMatchings(possibleLinks, toMatch, matching, result);
            matching.erase(match);
            toMatch.insert(needsMatch);
            toMatch.insert(preference);
        }
    }
}

/**
 * @brief maximumWeightMatching takes in a map with a nested map of partners and their partner
 *                              strength score. The function returns a std::set of the highest weight
 *                              pairings possible to achieve for the group
 * -----------------------------------------------------------------------------------------
 * @param possibleLinks         the std::map of strings and nested maps with strings and ints with weights.
 * @return                      the std::set representing the highest weight possible to achieve.
 * @warning                     negative weights and 0 are valid in this map. Negative = dislike.
 */
std::set<Pair> maximumWeightMatching(const std::map<string, std::map<string, int>>& possibleLinks) {
    std::unordered_set<string> toMatch = {};
    for (const auto &key : possibleLinks) {
        toMatch.insert(key.first);
    }
    std::set<Pair> maxWeight;
    return maxWeightRec(possibleLinks, toMatch, maxWeight);
}

/**
 * @brief maxWeightRec   recursively finds the highest weight score possible with the given map of
 *                       possible partner scores. The function uses an include, exclude recursive
 *                       pattern to compare possible choices for the best weight.
 * --------------------------------------------------------------------------------------------
 * @param possibleLinks  the std::map of strings and nested maps with strings and ints with weights.
 * @param toMatch        the std::unordered_set of highest level keys from possibleLinks.
 * @param maxWeight      the std::set to be returned that represents the partners with the highest score.
 * @return               the std::set of Pairs that represent the highest weight possible.
 */
std::set<Pair> maxWeightRec(const std::map<string, std::map<string, int>> &possibleLinks,
                             std::unordered_set<string>& toMatch,
                             std::set<Pair>& maxWeight) {
    // Once we have found the highest weight return it.
    if (toMatch.empty()) {
        return maxWeight;
    }
    string p1 = *toMatch.begin();
    // We exclude person in our potential matches and then run again.
    toMatch.erase(p1);
    std::set<Pair> exclude = maxWeightRec(possibleLinks, toMatch, maxWeight);
    int excludeWeight = sumWeights(possibleLinks, exclude);
    toMatch.insert(p1);

    // We don't need to loop through everything in the map, our recursion does, just loop weights
    const std::map<string, int> weights = possibleLinks.at(p1);

    // We need to setup for including this person in the potential weighting. Need the best set.
    std::set<Pair> bestInclude = {};
    int includeWeight = -INT_MAX;
    for (auto &p2 : weights) {
        if (toMatch.count(p2.first)) {
            Pair match(p1, p2.first);
            toMatch.erase(p1);
            toMatch.erase(p2.first);
            maxWeight.insert(match);

            // We include person in our potential matches
            std::set<Pair> include = maxWeightRec(possibleLinks, toMatch, maxWeight);

            maxWeight.erase(match);
            toMatch.insert(p1);
            toMatch.insert(p2.first);
            int totalWeight = sumWeights(possibleLinks, include);

            if (totalWeight > includeWeight) {
                includeWeight = totalWeight;
                bestInclude = include;
            }
        }
    }
    // compare two sets and higher total weight wins.
    return (includeWeight > excludeWeight) ? bestInclude : exclude;
}

/**
 * @brief sumWeights     takes in a std::map with a nested std::map of string int pairs to sum up the weight
 *                       of a given std::set. The std::set has Pairs. Each pair has a weight that must found
 *                       in the map and totaled.
 * -------------------------------------------------------------------------------------------
 * @param possibleLinks  the reference std::map used to find the weights of a partnership.
 * @param pairsToWeigh   the std::set of pairs to weigh. Score is symmetrical. Search for either partner.
 * @return               the integer representing the total weight of the std::set.
 */
int sumWeights(const std::map<string, std::map<string, int>> &possibleLinks,
               const std::set<Pair> &pairsToWeigh) {
    int totalWeight = 0;
    for (const Pair &p : pairsToWeigh) {
        totalWeight += possibleLinks.at(p.first()).at(p.second());
    }
    return totalWeight;
}

/* * * * * * * * Student Tests * * * * * * * * * * * * */

STUDENT_TEST("Line of six but tricky due to natural order.") {
    /*
     *
     *       C--D--A--F--B--E
     *
     *
     */
    const std::map<string, std::set<string>> provided = {
        {"A", {"D", "F"}},
        {"B", {"E", "F"}},
        {"C", {"D"}},
        {"D", {"A", "C"}},
        {"E", {"B"}},
        {"F", {"A", "B"}},
    };
    std::set<Pair> matching = {};
    EXPECT_EQUAL(hasPerfectMatching(provided, matching), true);
}

STUDENT_TEST("hasPerfectMatching works on a triangle of people.") {
    /* Here's the world:
     *
     *               A --- B
     *                \   /
     *                 \ /
     *                  C
     *
     * There is no perfect matching here, unfortunately.
     */
    const std::map<string, std::set<string>> provided = {
        { "A", {"B"} },
        { "B", {"C"} },
        { "C", {"A"} }
    };
    std::set<Pair> matching = {};
    EXPECT_EQUAL(hasPerfectMatching(provided, matching), false);
}

STUDENT_TEST("hasPerfectMatching works on a hexagon of people.") {
    /* Here's the world:
     *
     *               C --- B
     *             /        \
     *            E          F
     *             \        /
     *               D --- A
     * This is a weird shape where E and F could be easily left
     * out due to natural order of the map.
     *
     */
    const std::map<string, std::set<string>> provided = {
        { "A", {"D", "F"} },
        { "B", {"C", "F"} },
        { "C", {"B", "E"} },
        { "D", {"A", "E"} },
        { "E", {"C", "D"} },
        { "F", {"A", "B"} }
    };

    std::set<Pair> matching = {};
    EXPECT_EQUAL(hasPerfectMatching(provided, matching), true);
}

STUDENT_TEST("hasPerfectMatching works on a disconnected hexagon of people.") {
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
    const std::map<string, std::set<string>> provided = {
        { "A", {} },
        { "B", {"C", "F"} },
        { "C", {"B", "E"} },
        { "D", {"E"} },
        { "E", {"C", "D"} },
        { "F", {"B"} }
    };

    std::set<Pair> matching = {};
    EXPECT_EQUAL(hasPerfectMatching(provided, matching), false);
}

STUDENT_TEST("hasPerfectMatching works on a difficult shape with no single pairs.") {
    /* Here's the world:
     *
     *               C --- B ---G
     *             /        \   \
     *       I----E          F---H
     *       |     \        /
     *       J----- D --- A
     *
     *
     *
     */
    const std::map<string, std::set<string>> provided = {
        { "A", {"D", "F"} },
        { "B", {"C", "F", "G"} },
        { "C", {"B", "E"} },
        { "D", {"A", "E", "J"} },
        { "E", {"C", "D", "I"} },
        { "F", {"A", "B", "H"} },
        { "G", {"B", "H"} },
        { "H", {"F", "G"} },
        { "I", {"E", "J"} },
        { "J", {"D", "I"} }
    };

    std::set<Pair> matching = {};
    EXPECT_EQUAL(hasPerfectMatching(provided, matching), true);
}

STUDENT_TEST("hasPerfectMatching works on a difficult shape in alpha order.") {
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
    const std::map<string, std::set<string>> provided = {
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

    std::set<Pair> matching = {};
    EXPECT_EQUAL(hasPerfectMatching(provided, matching), true);
}

STUDENT_TEST("Simple case of one. Can't have a partner.") {
    const std::map<string, std::set<string>> provided = {
        { "A", {} },
    };
    std::set<Pair> matching = {};
    EXPECT_EQUAL(hasPerfectMatching(provided, matching), false);
}

STUDENT_TEST("Simple case of two. Should Work") {
    const std::map<string, std::set<string>> provided = {
        { "A", {"B"} },
        { "B", {"A"} },
    };
    std::set<Pair> matching = {};
    EXPECT_EQUAL(hasPerfectMatching(provided, matching), true);
}

STUDENT_TEST("Line of six but easier to test this way.") {
    /*
     *
     *       C--D--F--E--B--A
     *
     *
     */
    const std::map<string, std::set<string>> provided = {
        {"A", {"B"}},
        {"B", {"A", "E"}},
        {"C", {"D"}},
        {"D", {"C", "F"}},
        {"E", {"B", "F"}},
        {"F", {"D", "E"}},
    };
    std::set<Pair> matching = {};
    EXPECT_EQUAL(hasPerfectMatching(provided, matching), true);
}


/* * * * * *  * * Provided Tests * * * * * * * * * * * */


PROVIDED_TEST("hasPerfectMatching works on a world with just one person.") {
    /* The world is just a single person A, with no others. How sad. :-(
     *
     *                 A
     *
     * There is no perfect matching.
     */

    std::set<Pair> unused;
    EXPECT(!hasPerfectMatching({ { "A", {} } }, unused));
}

PROVIDED_TEST("hasPerfectMatching works on an empty set of people.") {
    /* There actually is a perfect matching - the set of no links meets the
     * requirements.
     */
    std::set<Pair> unused;
    EXPECT(hasPerfectMatching({}, unused));
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

    std::set<Pair> unused;
    EXPECT(hasPerfectMatching(links, unused));
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

    std::set<Pair> expected = {
        { "A", "B" }
    };

    std::set<Pair> matching;
    EXPECT(hasPerfectMatching(links, matching));
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

    std::set<Pair> unused;
    EXPECT(!hasPerfectMatching(links, unused));
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

    std::set<Pair> unused;
    EXPECT(hasPerfectMatching(links, unused));
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

    std::set<Pair> matching;
    EXPECT(hasPerfectMatching(links, matching));
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

    std::set<Pair> unused;
    EXPECT(!hasPerfectMatching(links, unused));
}

PROVIDED_TEST("hasPerfectMatching works on a line of six people.") {
    /* Because std::map and std::set internally store items in sorted order, the order
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
    std::vector<string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        std::map<string, std::set<string>> links = fromLinks({
            { people[0], people[1] },
            { people[1], people[2] },
            { people[2], people[3] },
            { people[3], people[4] },
            { people[4], people[5] }
        });

        std::set<Pair> matching;
        EXPECT(hasPerfectMatching(links, matching));
        EXPECT(isPerfectMatching(links, matching));
    } while (next_permutation(people.begin(), people.end()));
}

PROVIDED_TEST("hasPerfectMatching works on a more complex negative example.") {
    /* Because std::map and std::set internally store items in sorted order, the order
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
    std::vector<string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        std::map<string, std::set<string>> links = fromLinks({
            { people[0], people[2] },
            { people[1], people[2] },
            { people[2], people[3] },
            { people[3], people[4] },
            { people[3], people[5] },
        });

        std::set<Pair> matching;
        EXPECT(!hasPerfectMatching(links, matching));
    } while (next_permutation(people.begin(), people.end()));
}

PROVIDED_TEST("hasPerfectMatching works on a more complex positive example.") {
    /* Because std::map and std::set internally store items in sorted order, the order
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
    std::vector<string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        std::map<string, std::set<string>> links = fromLinks({
            { people[0], people[1] },
            { people[1], people[2] },
            { people[2], people[3] },
            { people[3], people[1] },
            { people[2], people[4] },
            { people[3], people[5] },
        });

        std::set<Pair> matching;
        EXPECT(hasPerfectMatching(links, matching));
        EXPECT(isPerfectMatching(links, matching));
    } while (next_permutation(people.begin(), people.end()));
}

PROVIDED_TEST("hasPerfectMatching works on a caterpillar.") {
    /* Because std::map and std::set internally store items in sorted order, the order
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
    std::vector<string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        std::map<string, std::set<string>> links = fromLinks({
            { people[0], people[1] },
            { people[1], people[2] },
            { people[0], people[3] },
            { people[1], people[4] },
            { people[2], people[5] },
        });

        std::set<Pair> matching;
        EXPECT(hasPerfectMatching(links, matching));
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

    std::vector<Pair> links;
    for (int i = 0; i < kRowSize - 1; i++) {
        links.push_back({ to_string(i), to_string(i + 1) });
    }
    for (int i = 0; i < kRowSize; i++) {
        links.push_back({ to_string(i), to_string(i + kRowSize) });
    }
    for (int i = 0; i < kRowSize; i++) {
        links.push_back({ to_string(i), to_string(i + 2 * kRowSize) });
    }

    std::set<Pair> matching;
    EXPECT(!hasPerfectMatching(fromLinks(links), matching));
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

    std::vector<Pair> links;
    for (int i = 0; i < kRowSize - 1; i++) {
        links.push_back({ to_string(i), to_string(i + 1) });
    }
    for (int i = 0; i < kRowSize; i++) {
        links.push_back({ to_string(i), to_string(i + kRowSize) });
    }

    std::set<Pair> matching;
    EXPECT(hasPerfectMatching(fromLinks(links), matching));
    EXPECT(isPerfectMatching(fromLinks(links), matching));
}

PROVIDED_TEST("maximumWeightMatching: Works for empty group.") {
    EXPECT_EQUAL(maximumWeightMatching({}), {});
}

PROVIDED_TEST("maximumWeightMatching: Works for group of one person.") {
    std::map<string, std::map<string, int>> links = {
        { "A", {} }
    };

    EXPECT_EQUAL(maximumWeightMatching(links), {});
}

PROVIDED_TEST("maximumWeightMatching: Works for a single pair of people.") {
    /* This world:
     *
     *  A --- B
     *     1
     *
     * Best option is to pick A -- B.
     */
    auto links = fromWeightedLinks({
        { "A", "B", 1 }
    });

    /* Should pick A--B. */
    EXPECT_EQUAL(maximumWeightMatching(links), {{"A", "B"}});
}

PROVIDED_TEST("maximumWeightMatching: Won't pick a negative edge.") {
    /* This world:
     *
     *  A --- B
     *     -1
     *
     * Best option is to not match anyone!
     */
    auto links = fromWeightedLinks({
        { "A", "B", -1 }
    });

    /* Should pick A--B. */
    EXPECT_EQUAL(maximumWeightMatching(links), {});
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

    /* Should pick B--C. */
    EXPECT_EQUAL(maximumWeightMatching(links), { {"B", "C"} });
}

PROVIDED_TEST("maximumWeightMatching: Works on a triangle.") {
    /* This world:
     *
     *         A
     *      1 / \ 2
     *       B---C
     *         3
     *
     * Best option is to pick B -- C.
     */
    auto links = fromWeightedLinks({
        { "A", "B", 1 },
        { "B", "C", 3 },
        { "A", "C", 2 }
    });

    /* Should pick B--C. */
    EXPECT_EQUAL(maximumWeightMatching(links), { {"B", "C"} });
}

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
    auto links = fromWeightedLinks({
        { "A", "B", 1 },
        { "B", "C", 2 },
        { "C", "D", 4 },
        { "D", "A", 8 },
    });

    EXPECT_EQUAL(maximumWeightMatching(links), { {"A", "D"}, {"B", "C"} });
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
    auto links = fromWeightedLinks({
        { "A", "B", 1 },
        { "B", "C", 3 },
        { "C", "D", 1 },
    });

    /* Should pick B--C. */
    EXPECT_EQUAL(maximumWeightMatching(links), { {"B", "C"} });
}

PROVIDED_TEST("maximumWeightMatching: Small stress test (should take at most a second or two).") {
    /* Because std::map and std::set internally store items in sorted order, the order
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
    std::vector<string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        auto links = fromWeightedLinks({
            { people[0], people[1], 5 },
            { people[1], people[2], 1 },
            { people[2], people[0], 1 },
            { people[3], people[0], 1 },
            { people[4], people[1], 1 },
            { people[5], people[2], 1 },
        });

        std::set<Pair> expected = {
            { people[0], people[1] },
            { people[2], people[5] }
        };

        EXPECT_EQUAL(maximumWeightMatching(links), expected);
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
    std::vector<WeightedLink> links;
    for (int i = 0; i < kNumPeople - 1; i++) {
        links.push_back({ to_string(i), to_string(i + 1), 1 });
    }

    auto matching = maximumWeightMatching(fromWeightedLinks(links));
    EXPECT_EQUAL(matching.size(), kNumPeople / 2);

    /* Confirm it's a matching. */
    std::set<string> used;
    for (Pair p: matching) {
        /* No people paired more than once. */
        EXPECT(!used.count(p.first()));
        EXPECT(!used.count(p.second()));
        used.insert(p.first());
        used.insert(p.second());

        /* Must be a possible links. */
        EXPECT_EQUAL(abs(stringToInteger(p.first()) - stringToInteger(p.second())), 1);
    }
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
    std::vector<std::set<Pair>> allMatches = {
        {{"A","B"}, {"C","D"}},
        {{"A","D"}, {"B","C"}}
    };
    EXPECT_EQUAL(getAllPerfectMatchings(links), allMatches);
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
    const std::map<std::string, std::set<std::string>> provided = {
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
    std::vector<std::set<Pair>> allMatches = {
        {{ "A", "J" }, { "B", "E" }, { "C", "D" }, { "F", "G" }, { "H", "I" }},
        {{ "A", "J" }, { "B", "C" }, { "D", "E" }, { "F", "G" }, { "H", "I" }},
        {{ "A", "B" }, { "C", "D" }, { "E", "F" }, { "G", "J" }, { "H", "I" }},
        {{ "A", "B" }, { "C", "D" }, { "E", "F" }, { "G", "H" }, { "I", "J" }},
    };
    EXPECT_EQUAL(getAllPerfectMatchings(provided), allMatches);
}
