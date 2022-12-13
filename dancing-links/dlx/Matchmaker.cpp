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
#include "hashset.h"
#include <climits>
using namespace std;

/* Function Prototypes */

// Perfect Pairs
bool hasPerfectMatching(const Map<string, Set<string>>& possibleLinks,
                        Set<Pair>& matching);
bool hasPerfectMatchRec(const Map<string, Set<string>>& possibleLinks,
                        const HashSet<string> linkKeys,
                        Set<Pair> &perfectMatch);

// Weighted pairs
Set<Pair> maximumWeightMatching(const Map<string, Map<string, int>>& possibleLinks);
Set<Pair> maxWeightRec(const Map<string, Map<string, int>> &possibleLinks,
                       const HashSet<string> linkKeys,
                       const Set<Pair> bestSet);
int sumWeights(const Map<string, Map<string, int>> &possibleLinks,
               const Set<Pair> &pairsToWeigh);

/* Function Implementations */

/**
 * @brief hasPerfectMatching  takes in a Map of possible partner connections and returns a boolean
 *                            representing whether or not everyone in the group can be paired based
 *                            on preference.
 * --------------------------------------------------------------
 * @param possibleLinks       the Map representing the possible connections for partners
 * @param matching            the designated output paramater for this function. The perfect pairs.
 * @return                    true if everone was matched, false if not.
 */
bool hasPerfectMatching(const Map<string, Set<string>>& possibleLinks, Set<Pair>& matching) {
    // Working with a set will make recursion and base case easier in our recursive function.
    HashSet<string> toMatch = {};
    for (auto &key : possibleLinks) {
        toMatch += key;
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
 * @param toMatch            the HashSet representing each key in the map. Progresses recursion.
 * @param perfectMatch       the output parameter that carries the perfect matches if they are found.
 * @return                   true if matches are found false if there is not a perfect match.
 */
bool hasPerfectMatchRec(const Map<string, Set<string>>& possibleLinks,
                        const HashSet<string> toMatch,
                        Set<Pair> &perfectMatch) {
    // We have examined all possible partners.
    if (toMatch.isEmpty()) {
        return true;
    }

    // We take pairs out two at a time, so if one is left, we fail.
    if (toMatch.size() == 1) {
        return false;
    }

    string needs_match = toMatch.first();
    Set<string> preferences = possibleLinks[needs_match];

    // Much smaller problem if we loop through the preferences not the entire map
    for (const string &preference : preferences) {
        // Someone in these preferences might have been taken out of the matches already during rec
        if (toMatch.contains(preference)) {
            Pair match (needs_match, preference);

            if (hasPerfectMatchRec(possibleLinks,
                                   toMatch - needs_match - preference,
                                   perfectMatch)) {

                // One extra step for the output parameter.
                perfectMatch += match;
                return true;
            }
        }
    }
    // Cleanup our output parameter.
    perfectMatch = {};
    return false;
}

/**
 * @brief maximumWeightMatching takes in a map with a nested map of partners and their partner
 *                              strength score. The function returns a Set of the highest weight
 *                              pairings possible to achieve for the group
 * -----------------------------------------------------------------------------------------
 * @param possibleLinks         the Map of strings and nested maps with strings and ints with weights.
 * @return                      the Set representing the highest weight possible to achieve.
 * @warning                     negative weights and 0 are valid in this map. Negative = dislike.
 */
Set<Pair> maximumWeightMatching(const Map<string, Map<string, int>>& possibleLinks) {
    HashSet<string> toMatch = {};
    for (const auto &key : possibleLinks) {
        toMatch += key;
    }
    return maxWeightRec(possibleLinks, toMatch, {});
}

/**
 * @brief maxWeightRec   recursively finds the highest weight score possible with the given map of
 *                       possible partner scores. The function uses an include, exclude recursive
 *                       pattern to compare possible choices for the best weight.
 * --------------------------------------------------------------------------------------------
 * @param possibleLinks  the Map of strings and nested maps with strings and ints with weights.
 * @param toMatch        the HashSet of highest level keys from possibleLinks.
 * @param maxWeight      the Set to be returned that represents the partners with the highest score.
 * @return               the Set of Pairs that represent the highest weight possible.
 */
Set<Pair> maxWeightRec(const Map<string, Map<string, int>> &possibleLinks,
                       const HashSet<string> toMatch,
                       const Set<Pair> maxWeight) {
    // Once we have found the highest weight return it.
    if (toMatch.isEmpty()) {
        return maxWeight;
    }
    string p1 = toMatch.first();
    // We exclude person in our potential matches and then run again.
    Set<Pair> exclude = maxWeightRec(possibleLinks, toMatch - p1, maxWeight);
    int excludeWeight = sumWeights(possibleLinks, exclude);

    // We don't need to loop through everything in the map, our recursion does, just loop weights
    const Map<string, int> weights = possibleLinks[p1];

    // We need to setup for including this person in the potential weighting. Need the best set.
    Set<Pair> bestInclude = {};
    int includeWeight = -INT_MAX;
    for (auto &p2 : weights) {
        if (toMatch.contains(p2)) {
            Pair match(p1, p2);

            // We include person in our potential matches
            Set<Pair> include = maxWeightRec(possibleLinks,
                                             toMatch - p1 - p2,
                                             maxWeight + match);
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
 * @brief sumWeights     takes in a Map with a nested Map of string int pairs to sum up the weight
 *                       of a given Set. The Set has Pairs. Each pair has a weight that must found
 *                       in the map and totaled.
 * -------------------------------------------------------------------------------------------
 * @param possibleLinks  the reference Map used to find the weights of a partnership.
 * @param pairsToWeigh   the Set of pairs to weigh. Score is symmetrical. Search for either partner.
 * @return               the integer representing the total weight of the Set.
 */
int sumWeights(const Map<string, Map<string, int>> &possibleLinks,
               const Set<Pair> &pairsToWeigh) {
    int totalWeight = 0;
    for (const Pair &p : pairsToWeigh) {
        totalWeight += possibleLinks[p.first()][p.second()];
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
    const Map<string, Set<string>> provided = {
        {"A", {"D", "F"}},
        {"B", {"E", "F"}},
        {"C", {"D"}},
        {"D", {"A", "C"}},
        {"E", {"B"}},
        {"F", {"A", "B"}},
    };
    Set<Pair> matching = {};
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
    const Map<string, Set<string>> provided = {
        { "A", {"B"} },
        { "B", {"C"} },
        { "C", {"A"} }
    };
    Set<Pair> matching = {};
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
    const Map<string, Set<string>> provided = {
        { "A", {"D", "F"} },
        { "B", {"C", "F"} },
        { "C", {"B", "E"} },
        { "D", {"A", "E"} },
        { "E", {"C", "D"} },
        { "F", {"A", "B"} }
    };

    Set<Pair> matching = {};
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
    const Map<string, Set<string>> provided = {
        { "A", {} },
        { "B", {"C", "F"} },
        { "C", {"B", "E"} },
        { "D", {"E"} },
        { "E", {"C", "D"} },
        { "F", {"B"} }
    };

    Set<Pair> matching = {};
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
    const Map<string, Set<string>> provided = {
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

    Set<Pair> matching = {};
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
    const Map<string, Set<string>> provided = {
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

    Set<Pair> matching = {};
    EXPECT_EQUAL(hasPerfectMatching(provided, matching), true);
}

STUDENT_TEST("Simple case of one. Can't have a partner.") {
    const Map<string, Set<string>> provided = {
        { "A", {} },
    };
    Set<Pair> matching = {};
    EXPECT_EQUAL(hasPerfectMatching(provided, matching), false);
}

STUDENT_TEST("Simple case of two. Should Work") {
    const Map<string, Set<string>> provided = {
        { "A", {"B"} },
        { "B", {"A"} },
    };
    Set<Pair> matching = {};
    EXPECT_EQUAL(hasPerfectMatching(provided, matching), true);
}

STUDENT_TEST("Line of six but easier to test this way.") {
    /*
     *
     *       C--D--F--E--B--A
     *
     *
     */
    const Map<string, Set<string>> provided = {
        {"A", {"B"}},
        {"B", {"A", "E"}},
        {"C", {"D"}},
        {"D", {"C", "F"}},
        {"E", {"B", "F"}},
        {"F", {"D", "E"}},
    };
    Set<Pair> matching = {};
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

    Set<Pair> unused;
    EXPECT(!hasPerfectMatching({ { "A", {} } }, unused));
}

PROVIDED_TEST("hasPerfectMatching works on an empty set of people.") {
    /* There actually is a perfect matching - the set of no links meets the
     * requirements.
     */
    Set<Pair> unused;
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

    Set<Pair> unused;
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

    Set<Pair> expected = {
        { "A", "B" }
    };

    Set<Pair> matching;
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

    Set<Pair> unused;
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

    Set<Pair> unused;
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

    Set<Pair> matching;
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

    Set<Pair> unused;
    EXPECT(!hasPerfectMatching(links, unused));
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
    Vector<string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        Map<string, Set<string>> links = fromLinks({
            { people[0], people[1] },
            { people[1], people[2] },
            { people[2], people[3] },
            { people[3], people[4] },
            { people[4], people[5] }
        });

        Set<Pair> matching;
        EXPECT(hasPerfectMatching(links, matching));
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
    Vector<string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        Map<string, Set<string>> links = fromLinks({
            { people[0], people[2] },
            { people[1], people[2] },
            { people[2], people[3] },
            { people[3], people[4] },
            { people[3], people[5] },
        });

        Set<Pair> matching;
        EXPECT(!hasPerfectMatching(links, matching));
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
    Vector<string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        Map<string, Set<string>> links = fromLinks({
            { people[0], people[1] },
            { people[1], people[2] },
            { people[2], people[3] },
            { people[3], people[1] },
            { people[2], people[4] },
            { people[3], people[5] },
        });

        Set<Pair> matching;
        EXPECT(hasPerfectMatching(links, matching));
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
    Vector<string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        Map<string, Set<string>> links = fromLinks({
            { people[0], people[1] },
            { people[1], people[2] },
            { people[0], people[3] },
            { people[1], people[4] },
            { people[2], people[5] },
        });

        Set<Pair> matching;
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

    Vector<Pair> links;
    for (int i = 0; i < kRowSize - 1; i++) {
        links.add({ to_string(i), to_string(i + 1) });
    }
    for (int i = 0; i < kRowSize; i++) {
        links.add({ to_string(i), to_string(i + kRowSize) });
    }
    for (int i = 0; i < kRowSize; i++) {
        links.add({ to_string(i), to_string(i + 2 * kRowSize) });
    }

    Set<Pair> matching;
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

    Vector<Pair> links;
    for (int i = 0; i < kRowSize - 1; i++) {
        links.add({ to_string(i), to_string(i + 1) });
    }
    for (int i = 0; i < kRowSize; i++) {
        links.add({ to_string(i), to_string(i + kRowSize) });
    }

    Set<Pair> matching;
    EXPECT(hasPerfectMatching(fromLinks(links), matching));
    EXPECT(isPerfectMatching(fromLinks(links), matching));
}

PROVIDED_TEST("maximumWeightMatching: Works for empty group.") {
    EXPECT_EQUAL(maximumWeightMatching({}), {});
}

PROVIDED_TEST("maximumWeightMatching: Works for group of one person.") {
    Map<string, Map<string, int>> links = {
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
    Vector<string> people = { "A", "B", "C", "D", "E", "F" };
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
    Vector<WeightedLink> links;
    for (int i = 0; i < kNumPeople - 1; i++) {
        links.add({ to_string(i), to_string(i + 1), 1 });
    }

    auto matching = maximumWeightMatching(fromWeightedLinks(links));
    EXPECT_EQUAL(matching.size(), kNumPeople / 2);

    /* Confirm it's a matching. */
    Set<string> used;
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
