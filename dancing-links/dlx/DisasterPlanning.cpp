/**
 * Last Updated: Alex G. Lopez, CS106B-Winter, 2022.02.03
 * Assignment: Recursion to the Rescue, Part 3
 *
 * File: DisasterPlanning.cpp
 * ---------------------------------------------------------
 * This program will determine if a country has coverage for a disaster. The program represents
 * cities as keys in a Map, with all connected cities in a Set of strings. The general strategy
 * of the program is to attempt to supply a city first and then its neighbors and see if starting
 * with either will result in successfull distribution of supplies. We use a recursive
 * implementation here and the recursive insight is as follows: any city is safe if it has supplies
 * if a neighboring city has supplies.
 *
 * Implemention Notes:
 * ----------------------------------------------------------
 * This was an extremely challenging program for me. The final detail that caused multiple days of
 * debugging was what happens when a city does not work out as a viable supply location. I had
 * previously thought of the cities that you are trying to supply as a set of unsafe cities, until
 * you try to give them supplies. Then, they become a safe city, and all surrounding cities are
 * safe. If that doesn't work, the city is no longer safe, so put it back into the unsafe city
 * Set and continue. This is wrong! In order to progress the recursion, that city you have tried
 * must remain out of the picture, even if this is a backtracking algorithm that involves undoing
 * choices. You should think of the cities as a set of trial cities and if they don't work, you must
 * discard them. Finally, as suggested by the Extensions for this assignment, I added a helper
 * function that significantly speeds up the program. It selects the next city to try based on which
 * has the lowest number of connections. It then loops through the nearest neighbors to see if they
 * are the best choice. They likely will be better than the isolated city and we will be required to
 * choose one of the two in order to have any hope of supplying all of the cities. This allows us to
 * solve very hard maps almost immediately and Colorado only takes a few seconds. Further
 * optimizations could include sorting the data beforehand with a PriorityQueue, where the city is
 * the value and the number of connections is the priority. However, I was not able to get this to
 * work in the recursion due to the need to remove all connected cities and then loop through
 * those connected cities. I'm sure with more thought it could be done. The looping should start to
 * hurt performance on very large maps like the US and I'm sure that if you pre-sorted the data,
 * you could make the U.S solvable in a relatively short amount of time.
 *
 * Citations:
 * ----------------------------------------------------------
 * 1. I went to LaIR debugging and LaIR conceptual. In debugging, I received help on what it means
 *    for a city to receive supplies. All adjacent cities are then safe. In conceptual, I discussed
 *    when the doing and undoing of decisions occurs in a backtracking algorithm. I better
 *    understoodthat most changes to a recursive algorithm need to happen within the function call
 *    itself.
 *
 */
#include <climits>
#include "DisasterPlanning.h"
#include "Utilities/DisasterUtilities.h"
using namespace std;

/* Function Prototypes */


bool canBeMadeDisasterReady(const Map<string, Set<string>>& roadNetwork,
                            int numCities,
                            Set<string>& supplyLocations);

bool isReadyRec(const Map<string, Set<string>> & roadNetwork,
                const Set<string> citiesToTry,
                int supplyRemaining,
                Set<string> &supplyLocations);

Set<Set<string>> fillSupplySchemes(const Map<string, Set<string>>& roadNetwork,
                                   const Set<string> citiesToCover,
                                   int numSupplies,
                                   Set<string> supplyLocations);

string getPriorityCity(const Map<string, Set<string>> &roadNetwork,
                       const Set<string>& remaining);

/* Function Implementations */


/**
 * @brief canBeMadeDisasterReady  takes a Map representing a road network and a number of cities it
 *                                can supply and returns a boolean representing whether that city
 *                                can be covered with the supplies. A city is disaster ready if it
 *                                has supplies or connects to a city with supplies.
 * @param roadNetwork             the Map of keys with Set of strings representing connected cities.
 * @param numCities               the number of cities we are able to supply represented with int.
 * @param supplyLocations         the output that the function will return if the map is covered.
 * @return                        a boolean representing if the Map can be made disaster ready.
 * @warning                       entering negative numCities will trigger an error.
 * @note                          an empty Map returns true and is disaster ready
 */
bool canBeMadeDisasterReady(const Map<string, Set<string>>& roadNetwork,
                            int numCities,
                            Set<string>& supplyLocations) {
    if (numCities < 0) {
        error("Parameter, 'numCities' is currently set to a negative number.");
    }
    // Seems the tests want this to be true. Makes sense. Nothing to cover means always ready.
    if (roadNetwork.isEmpty()) {
        return true;
    }

    // Set will make insertion and deletions easier.
    Set<string> citiesToTry = {};
    for (const auto &key : roadNetwork) {
        citiesToTry += key;
    }
    return isReadyRec(roadNetwork, citiesToTry, numCities, supplyLocations);
}

/**
 * @brief isReadyRec       recursively finds whether a map can be adequately covered with the given
 *                         supplies. A city is covered if it has supplies or is connected to a city
 *                         with supplies. The recursive approach is an include/exclude pattern that
 *                         has us choosing a city and then choosing the connected cities and trying
 *                         to supply combinations of those cities. The recursive backtracking is
 *                         required because we must take back supply locations and supplies if a
 *                         city fails.
 * @param roadNetwork      the Map representing the cities and cities that connect to those cities.
 * @param citiesToTry      the Set of cities we will examine as candidates for supply.
 * @param supplyLocations  the output Parameter of locations we will carry back to main function.
 * @param supplyRemaining  the integer count of total supplies we have to distribute.
 * @return                 the boolean value representing wheter we have covered all cities or not.
 */
bool isReadyRec(const Map<string, Set<string>> & roadNetwork,
                const Set<string> citiesToTry,
                int supplyRemaining,
                Set<string> &supplyLocations) {

    // It is common in tests to give out last supply for the last necessary city.
    if (citiesToTry.isEmpty() && supplyRemaining >= 0) {
        return true;
    }
    // Saves a TON of recursive calls that happen if supplyRemaining is 0 only to go -1=false.
    if (supplyRemaining <= 0) {
        return false;
    }

    // See helper below. We can save recursive calls if we pick cities with low connection count.
    string city = getPriorityCity(roadNetwork, citiesToTry);
    Set<string> connectedCities = roadNetwork[city];

    // city is the most isolated so far. Nearest neighbors will almost certainly be best choice
    for (auto &connection : connectedCities) {

        // These are all the cities connected to the adjacent city.
        Set<string> connectedToConnection = roadNetwork[connection];
        if (isReadyRec(roadNetwork,
                       citiesToTry - connection - connectedToConnection,
                       supplyRemaining - 1,
                       supplyLocations)) {

            supplyLocations.add(connection);
            return true;
        }

    }

    // Now we can check if the isolated city would be a good choice. Possible, but less likely.
    if (isReadyRec(roadNetwork,
                   citiesToTry - city - connectedCities,
                   supplyRemaining - 1,
                   supplyLocations)) {

        supplyLocations.add(city);
        return true;
    }
    return false;
}

/**
 * @brief findAllSupplySchemes  finds every possible configuration of supply distribution with the
 *                              given number of supplies. Only use this function once the optimal
 *                              number of supplies is known. Otherwise it will be slower than
 *                              it already is.
 * @param roadNetwork           Map representing the cities and cities that connect to those cities.
 * @param numSupplies           the integer count of total supplies we have to distribute.
 * @return                      Set of Sets of all distributions.
 */
Set<Set<string>> findAllSupplySchemes(const Map<string, Set<string>>& roadNetwork,
                                      int numSupplies) {
    if (numSupplies < 0) {
        error("negative supplies.");
    }
    if (roadNetwork.isEmpty()) {
        return {};
    }
    // Set will make insertion and deletions easier.
    Set<string> citiesToTry = {};
    for (const auto &key : roadNetwork) {
        citiesToTry += key;
    }
    return fillSupplySchemes(roadNetwork, citiesToTry, numSupplies, {});
}

/**
 * @brief fillSupplySchemes  finds all possible distributions of the number of supplies given. This
 *                           is slow and I use a Set to filter out duplicates. I am seeking a
 *                           better way to do this but am not sure the best way to avoid duplicates.
 * @param roadNetwork        the Map representing cities and cities that connect to those cities.
 * @param citiesToCover      the Set of cities we will examine as candidates for supply.
 * @param numSupplies        the integer count of total supplies we have to distribute.
 * @param supplyLocations    the output Parameter of locations we will carry back to main function.
 * @return                   the Set of all supply configurations.
 */
Set<Set<string>> fillSupplySchemes(const Map<string, Set<string>>& roadNetwork,
                                   const Set<string> citiesToCover,
                                   int numSupplies,
                                   Set<string> supplyLocations) {
    if (citiesToCover.isEmpty() && numSupplies >= 0) {
        return {supplyLocations};
    }
    if (numSupplies <= 0) {
        return {};
    }

    // See helper below. We can save recursive calls if we pick cities with low connection count.
    string city = getPriorityCity(roadNetwork, citiesToCover);
    Set<string> connectedCities = roadNetwork[city];

    Set<Set<string>> result = {};
    // city is the most isolated so far. Nearest neighbors will almost certainly be best choice
    for (auto &connection : connectedCities) {

        // These are all the cities connected to the adjacent city.
        Set<string> connectedToConnection = roadNetwork[connection];
        result += fillSupplySchemes(roadNetwork,
                                    citiesToCover - connection - connectedToConnection,
                                    numSupplies - 1,
                                    supplyLocations + connection);
    }

    // Now we can check if the isolated city would be a good choice. Possible, but less likely.
    result += fillSupplySchemes(roadNetwork,
                                citiesToCover - city - connectedCities,
                                numSupplies - 1,
                                supplyLocations + city);
    return result;
}

/**
 * @brief getPriorityCity  finds the city with the least number of connections because we know it is
 *                         likely they will need to be supplied or connected to supplies. If the
 *                         city only has one connection it will need to be supplied by its neighbor.
 *                         Therefore, try to supply the neighbors first and you will likely find
 *                         your answer much more quickly. This speeds up our search.
 * @param roadNetwork      the map we use to find the number of cities connected to each city.
 * @param remaining        the Set of cities we will be giving this function during recursion.
 * @return                 the string value of the city with the lowest number of connections.
 * @note                   while looping on each recursive call is slow, the smart choosing saves
 *                         significant time on the program task overall. VeryHard maps are done
 *                         almost instantly and Colorado only takes a few seconds.
 */
string getPriorityCity(const Map<string, Set<string>> &roadNetwork,
                       const Set<string>& remaining) {
    string result = remaining.first();
    // No city could possibly have this many connections, set as sentinnel.
    int smallestSize = INT_MAX;
    int size = 0;
    for (const auto& city : remaining) {
        size = roadNetwork[city].size();
        if (size < smallestSize) {
            result = city;
            smallestSize = size;
        }
    }
    return result;
}

/* * * * * * * Test Helper Functions Below This Point * * * * * */
#include "GUI/SimpleTest.h"


/* * * * * * Test Cases Below This Point * * * * * */


STUDENT_TEST("Can we tell when the cities are safe without giving out endless supplies?") {

    /*
     *
     *             B
     *             |
     *        A -- C
     *
     *
     *
     */
    const Map<string, Set<string>> cities = {
        {"A", {"C"}},
        {"B", {"C"}},
        {"C", {"A", "B"}},
    };
    Set<string> provided = {};
    EXPECT_EQUAL(canBeMadeDisasterReady(cities, 999, provided), true);
    EXPECT_EQUAL(provided, {"C"});
}

STUDENT_TEST("Most basic example of three way choice.") {
    /*
     *
     *             B
     *             |
     *        A -- C
     *
     *
     *
     */
    const Map<string, Set<string>> cities = {
        {"A", {"C"}},
        {"B", {"C"}},
        {"C", {"A", "B"}},
    };
    Set<string> provided = {};
    EXPECT_EQUAL(canBeMadeDisasterReady(cities, 1, provided), true);
    EXPECT_EQUAL(provided, {"C"});
}

STUDENT_TEST("Test the X configuration from the assignment handout.") {
    // Has to pick one city, should be X.
    const Map <string, Set<string>> cityConfig = {
        {"A", {"X"}},
        {"B", {"X"}},
        {"D", {"X"}},
        {"C", {"X"}},
        {"X", {"A", "B", "C", "D"}}
    };
    auto testMap = makeSymmetric(cityConfig);
    Set<string> provided = {};
    EXPECT_EQUAL(canBeMadeDisasterReady(testMap, 1, provided), true);
    EXPECT_EQUAL(provided, {"X"});
}

STUDENT_TEST("One student config of the line test.") {
    /*
     *
     *        F -- D -- B -- E -- C -- A
     *
     */

    const Map<string, Set<string>> cities = {
        {"A", {"C"}},
        {"B", {"D", "E"}},
        {"C", {"A", "E"}},
        {"D", {"B", "F"}},
        {"E", {"B", "C"}},
        {"F", {"D"}},
    };
    Set<string> provided = {};
    EXPECT_EQUAL(canBeMadeDisasterReady(cities, 2, provided), true);
    EXPECT_EQUAL(provided, {"D", "C"});
}


STUDENT_TEST("Simple Ethene example for testing.") {
    /*
     *
     *             C
     *             |
     *        A -- D -- B -- F
     *                  |
     *                  E
     *
     */
    const Map<string, Set<string>> cities = {
        {"A", {"D"}},
        {"B", {"D", "E", "F"}},
        {"C", {"D"}},
        {"D", {"A", "C", "B"}},
        {"E", {"B"}},
        {"F", {"B"}},
    };
    Set<string> provided = {};
    EXPECT_EQUAL(canBeMadeDisasterReady(cities, 2, provided), true);
    EXPECT_EQUAL(provided, {"B", "D"});
}

STUDENT_TEST("Solves \"Don't be Greedy,\" for a single simple config from handout.") {
    /*
     *
     *     A       E
     *     |       |
     *     B - D - F
     *      \ / \ /
     *       C   G
     *
     */

    const Map<string, Set<string>> cities = {
        {"A", {"B"}},
        {"B", {"A", "C", "D"}},
        {"C", {"B", "D"}},
        {"D", {"B", "C", "F", "G"}},
        {"E", {"F"}},
        {"F", {"D", "E", "G"}},
        {"G", {"D", "F"}},
    };
    Set<string> provided = {};
    EXPECT_EQUAL(canBeMadeDisasterReady(cities, 2, provided), true);
    EXPECT_EQUAL(provided, {"B", "F"});
}

STUDENT_TEST("Test the X configuration from the assignment handout.") {
    // Make all cities available for supplies, should be true.
    const Map <string, Set<string>> cityConfig = {
        {"A", {"X"}},
        {"B", {"X"}},
        {"D", {"X"}},
        {"C", {"X"}},
        {"X", {"A", "B", "C", "D"}}
    };
    auto testMap = makeSymmetric(cityConfig);
    Set<string> provided = {};
    EXPECT_EQUAL(canBeMadeDisasterReady(testMap, 5, provided), true);
}


/* * * * * Provided Tests Below This Point * * * * */

PROVIDED_TEST("Reports an error if numCities < 0") {
    Set<string> supply;
    EXPECT_ERROR(canBeMadeDisasterReady({}, -137, supply));
}

PROVIDED_TEST("Works for map with no cities.") {
    Set<string> locations;

    /* The number of cities we use really doesn't matter here. */
    EXPECT(canBeMadeDisasterReady({}, 0, locations));
    EXPECT_EQUAL(locations.size(), 0);

    EXPECT(canBeMadeDisasterReady({}, 137, locations));
    EXPECT_EQUAL(locations.size(), 0);
}

PROVIDED_TEST("Works for map with one city.") {
    Map<string, Set<string>> map = makeSymmetric({
         { "Solipsist", {} }
    });

    /* Shouldn't matter how many cities we use, as long as it isn't zero! */
    Set<string> locations0, locations1, locations2;
    EXPECT(!canBeMadeDisasterReady(map, 0, locations0));
    EXPECT( canBeMadeDisasterReady(map, 1, locations1));
    EXPECT( canBeMadeDisasterReady(map, 2, locations2));
}

PROVIDED_TEST("Works for map with one city, and produces output.") {
    Map<string, Set<string>> map = makeSymmetric({
         { "Solipsist", {} }
    });

    Set<string> locations0, locations1, locations2;
    EXPECT(!canBeMadeDisasterReady(map, 0, locations0));
    EXPECT(canBeMadeDisasterReady(map, 1, locations1));
    EXPECT(canBeMadeDisasterReady(map, 2, locations2));

    /* Don't check locations0; since the function returned false, the values there
     * can be anything.
     */
    Set<string> expected = { "Solipsist" };
    EXPECT_EQUAL(locations1, expected);
    EXPECT_EQUAL(locations2, expected);
}

PROVIDED_TEST("Works for map with two linked cities.") {
    Map<string, Set<string>> map = makeSymmetric({
         { "A", { "B" } },
         { "B", {     } }
    });

    Set<string> locations0, locations1, locations2;
    EXPECT(!canBeMadeDisasterReady(map, 0, locations0));
    EXPECT(canBeMadeDisasterReady(map, 1, locations1));
    EXPECT(canBeMadeDisasterReady(map, 2, locations2));
}

PROVIDED_TEST("Works for map with two linked cities, and produces output.") {
    Map<string, Set<string>> map = makeSymmetric({
         { "A", { "B" } },
    });

    Set<string> locations0, locations1, locations2;
    EXPECT(!canBeMadeDisasterReady(map, 0, locations0));
    EXPECT(canBeMadeDisasterReady(map, 1, locations1));
    EXPECT(canBeMadeDisasterReady(map, 2, locations2));

    EXPECT_EQUAL(locations1.size(), 1);
    EXPECT(locations1.isSubsetOf({"A", "B"}));

    EXPECT(locations2.size() <= 2);
    EXPECT(locations2.isSubsetOf({"A", "B"}));
}

PROVIDED_TEST("Works for four disconnected cities.") {
    Map<string, Set<string>> map = makeSymmetric({
        { "A", { } },
        { "B", { } },
        { "C", { } },
        { "D", { } }
    });

    Set<string> locations0, locations1, locations2, locations3, locations4;
    EXPECT(!canBeMadeDisasterReady(map, 0, locations0));
    EXPECT(!canBeMadeDisasterReady(map, 1, locations1));
    EXPECT(!canBeMadeDisasterReady(map, 2, locations2));
    EXPECT(!canBeMadeDisasterReady(map, 3, locations3));
    EXPECT(canBeMadeDisasterReady(map, 4, locations4));
}

PROVIDED_TEST("Works for four disconnected cities, and produces output.") {
    Map<string, Set<string>> map = makeSymmetric({
        { "A", { } },
        { "B", { } },
        { "C", { } },
        { "D", { } }
    });

    Set<string> locations0, locations1, locations2, locations3, locations4;
    EXPECT(!canBeMadeDisasterReady(map, 0, locations0));
    EXPECT(!canBeMadeDisasterReady(map, 1, locations1));
    EXPECT(!canBeMadeDisasterReady(map, 2, locations2));
    EXPECT(!canBeMadeDisasterReady(map, 3, locations3));
    EXPECT(canBeMadeDisasterReady(map, 4, locations4));

    Set<string> expected = { "A", "B", "C", "D" };
    EXPECT_EQUAL(locations4, expected);
}

PROVIDED_TEST("Can solve ethene example, regardless of ordering.") {
    /* Because Map and Set internally store items in sorted order, the order
     * in which you iterate over the cities when making decisions is sensitive
     * to the order of those cities' names. This test looks at a map like
     * this one, trying out all possible orderings of the city names:
     *
     *             *
     *             |
     *        * -- * -- * -- *
     *                  |
     *                  *
     *
     * There are 6! = 720 possible permutations of the ordering of these six
     * cities. If your code is able to solve the problem correctly for all of
     * those orderings, there's a good chance that you're correctly
     * covering and uncovering cities at each step. On the other hand, if
     * your code runs into issues here, it may indicate that the way in which
     * you're covering and uncovering cities accidentally uncovers a city that
     * you have previously covered.
     */
    Vector<string> cities = { "A", "B", "C", "D", "E", "F" };
    do {
        /* Ethene arrangement:
         *
         *        0
         *       1234
         *         5
         */
        Map<string, Set<string>> map = makeSymmetric({
            { cities[2], { cities[0], cities[1], cities[3] } },
            { cities[3], { cities[4], cities[5] } }
        });

        /* We should be able to cover everything with two cities:
         * city 2 and city 3.
         */
        Set<string> chosen;
        EXPECT(canBeMadeDisasterReady(map, 2, chosen));

        /* We should not be able to cover everything with one city. */
        chosen.clear();
        EXPECT(!canBeMadeDisasterReady(map, 1, chosen));
    } while (next_permutation(cities.begin(), cities.end()));
}

PROVIDED_TEST("Can solve ethene example, regardless of ordering, and produces output.") {
    /* Because Map and Set internally store items in sorted order, the order
     * in which you iterate over the cities when making decisions is sensitive
     * to the order of those cities' names. This test looks at a map like
     * this one, trying out all possible orderings of the city names:
     *
     *             *
     *             |
     *        * -- * -- * -- *
     *                  |
     *                  *
     *
     * There are 6! = 720 possible permutations of the ordering of these six
     * cities. If your code is able to solve the problem correctly for all of
     * those orderings, there's a good chance that you're correctly
     * covering and uncovering cities at each step. On the other hand, if
     * your code runs into issues here, it may indicate that the way in which
     * you're covering and uncovering cities accidentally uncovers a city that
     * you have previously covered.
     */
    Vector<string> cities = { "A", "B", "C", "D", "E", "F" };
    do {
        /* Ethene arrangement:
         *
         *        0
         *       1234
         *         5
         */
        Map<string, Set<string>> map = makeSymmetric({
            { cities[2], { cities[0], cities[1], cities[3] } },
            { cities[3], { cities[4], cities[5] } }
        });

        /* We should be able to cover everything with two cities:
         * city 2 and city 3.
         */
        Set<string> chosen;
        EXPECT(canBeMadeDisasterReady(map, 2, chosen));

        EXPECT_EQUAL(chosen.size(), 2);
        EXPECT(chosen.contains(cities[2]));
        EXPECT(chosen.contains(cities[3]));

        /* We should not be able to cover everything with one city. */
        chosen.clear();
        EXPECT(!canBeMadeDisasterReady(map, 1, chosen));
    } while (next_permutation(cities.begin(), cities.end()));
}

PROVIDED_TEST("Works for six cities in a line, regardless of order.") {
    /* Because Map and Set internally store items in sorted order, the order
     * in which you iterate over the cities when making decisions is sensitive
     * to the order of those cities' names. This test looks at a map like
     * this one, trying out all possible orderings of the city names:
     *
     *        * -- * -- * -- * -- * -- *
     *
     * There are 6! = 720 possible permutations of the ordering of these six
     * cities in a line. If your code is able to solve the problem correctly
     * for all of those orderings, there's a good chance that you're correctly
     * covering and uncovering cities at each step. On the other hand, if
     * your code runs into issues here, it may indicate that the way in which
     * you're covering and uncovering cities accidentally uncovers a city that
     * you have previously covered.
     */
    Vector<string> cities = { "A", "B", "C", "D", "E", "F" };
    do {
        /* Linear arrangement. */
        Map<string, Set<string>> map;
        for (int i = 1; i + 1 < cities.size(); i++) {
            map[cities[i]] = { cities[i - 1], cities[i + 1] };
        }

        map = makeSymmetric(map);

        /* We should be able to cover everything with two cities, specifically,
         * the cities one spot in from the two sides.
         */
        Set<string> chosen;
        EXPECT(canBeMadeDisasterReady(map, 2, chosen));

        /* We should not be able to cover everything with one city. */
        chosen.clear();
        EXPECT(!canBeMadeDisasterReady(map, 1, chosen));
    } while (next_permutation(cities.begin(), cities.end()));
}

PROVIDED_TEST("Works for six cities in a line, regardless of order, and produces output.") {
    /* Because Map and Set internally store items in sorted order, the order
     * in which you iterate over the cities when making decisions is sensitive
     * to the order of those cities' names. This test looks at a map like
     * this one, trying out all possible orderings of the city names:
     *
     *        * -- * -- * -- * -- * -- *
     *
     * There are 6! = 720 possible permutations of the ordering of these six
     * cities in a line. If your code is able to solve the problem correctly
     * for all of those orderings, there's a good chance that you're correctly
     * covering and uncovering cities at each step. On the other hand, if
     * your code runs into issues here, it may indicate that the way in which
     * you're covering and uncovering cities accidentally uncovers a city that
     * you have previously covered.
     */
    Vector<string> cities = { "A", "B", "C", "D", "E", "F" };
    do {
        /* Linear arrangement. */
        Map<string, Set<string>> map;
        for (int i = 1; i + 1 < cities.size(); i++) {
            map[cities[i]] = { cities[i - 1], cities[i + 1] };
        }

        map = makeSymmetric(map);

        /* We should be able to cover everything with two cities, specifically,
         * the cities one spot in from the two sides.
         */
        Set<string> chosen;
        EXPECT(canBeMadeDisasterReady(map, 2, chosen));

        EXPECT_EQUAL(chosen.size(), 2);
        EXPECT(chosen.contains(cities[1]));
        EXPECT(chosen.contains(cities[4]));

        /* We should not be able to cover everything with one city. */
        chosen.clear();
        EXPECT(!canBeMadeDisasterReady(map, 1, chosen));
    } while (next_permutation(cities.begin(), cities.end()));
}

/* The "Don't Be Greedy" sample world. */
const Map<string, Set<string>> kDontBeGreedy = makeSymmetric({
    { "A", { "B" } },
    { "B", { "C", "D" } },
    { "C", { "D" } },
    { "D", { "F", "G" } },
    { "E", { "F" } },
    { "F", { "G" } },
});

PROVIDED_TEST("Solves \"Don't be Greedy\" from the handout.") {
    Set<string> locations0, locations1, locations2;
    EXPECT(!canBeMadeDisasterReady(kDontBeGreedy, 0, locations0));
    EXPECT(!canBeMadeDisasterReady(kDontBeGreedy, 1, locations1));
    EXPECT( canBeMadeDisasterReady(kDontBeGreedy, 2, locations2));
}

PROVIDED_TEST("Solves \"Don't be Greedy\" from the handout, and produces output.") {
    Set<string> locations0, locations1, locations2;
    EXPECT(!canBeMadeDisasterReady(kDontBeGreedy, 0, locations0));
    EXPECT(!canBeMadeDisasterReady(kDontBeGreedy, 1, locations1));
    EXPECT( canBeMadeDisasterReady(kDontBeGreedy, 2, locations2));

    Set<string> expected = {"B", "F"};
    EXPECT_EQUAL(locations2, expected);
}

PROVIDED_TEST("Solves \"Don't be Greedy,\" regardless of ordering, and produces output.") {
    /* Because Map and Set internally store items in sorted order, the order
     * in which you iterate over the cities when making decisions is sensitive
     * to the order of those cities' names. This test looks at a map like
     * this one, trying out all possible orderings of the city names:
     *
     *     0       4
     *     |       |
     *     1 - 2 - 3
     *      \ / \ /
     *       5   6
     *
     * There are 7! = 5,040 possible permutations of the ordering of these seven
     * cities. If your code is able to solve the problem correctly for all of
     * those orderings, there's a good chance that you're correctly
     * covering and uncovering cities at each step. On the other hand, if
     * your code runs into issues here, it may indicate that the way in which
     * you're covering and uncovering cities accidentally uncovers a city that
     * you have previously covered.
     */
    Vector<string> cities = { "A", "B", "C", "D", "E", "F", "G" };
    do {
        Map<string, Set<string>> map = makeSymmetric({
            { cities[1], { cities[0], cities[2], cities[5] } },
            { cities[2], { cities[3], cities[5], cities[6] } },
            { cities[3], { cities[4], cities[6] } },
        });

        /* We should be able to cover everything with two cities. */
        Set<string> chosen;
        EXPECT(canBeMadeDisasterReady(map, 2, chosen));

        /* Those cities should be 1 and 3. */
        EXPECT_EQUAL(chosen, { cities[1], cities[3] });

        /* We should not be able to cover everything with one city. */
        chosen.clear();
        EXPECT(!canBeMadeDisasterReady(map, 1, chosen));
    } while (next_permutation(cities.begin(), cities.end()));
}

PROVIDED_TEST("Stress test: 6 x 6 grid. (This should take at most a few seconds.)") {
    Map<string, Set<string>> grid;

    /* Build the grid. */
    char maxRow = 'F';
    int  maxCol = 6;
    for (char row = 'A'; row <= maxRow; row++) {
        for (int col = 1; col <= maxCol; col++) {
            if (row != maxRow) {
                grid[row + to_string(col)] += (char(row + 1) + to_string(col));
            }
            if (col != maxCol) {
                grid[row + to_string(col)] += (char(row) + to_string(col + 1));
            }
        }
    }
    grid = makeSymmetric(grid);

    Set<string> locations;
    EXPECT(canBeMadeDisasterReady(grid, 10, locations));
}

PROVIDED_TEST("Stress test: 6 x 6 grid, with output. (This should take at most a few seconds.)") {
    Map<string, Set<string>> grid;

    /* Build the grid. */
    char maxRow = 'F';
    int  maxCol = 6;
    for (char row = 'A'; row <= maxRow; row++) {
        for (int col = 1; col <= maxCol; col++) {
            if (row != maxRow) {
                grid[row + to_string(col)] += (char(row + 1) + to_string(col));
            }
            if (col != maxCol) {
                grid[row + to_string(col)] += (char(row) + to_string(col + 1));
            }
        }
    }
    grid = makeSymmetric(grid);

    Set<string> locations;
    EXPECT(canBeMadeDisasterReady(grid, 10, locations));

    for (char row = 'A'; row <= maxRow; row++) {
        for (int col = 1; col <= maxCol; col++) {
            EXPECT(isCovered(row + to_string(col), grid, locations));
        }
    }
}

STUDENT_TEST("All possible configurations of a square.") {
    /*
     *
     *        A----B
     *        |    |
     *        C----D
     *
     *
     *
     */
    const Map<std::string, Set<std::string>> cities = {
        {"A", {"B","C"}},
        {"B", {"A","D"}},
        {"C", {"A","D"}},
        {"D", {"B","C"}},
    };
    Set<Set<std::string>> allConfigs = {
        {"A","C"},
        {"A","B"},
        {"A","D"},
        {"B","C"},
        {"B","D"},
        {"C","D"},
    };
    Set<Set<std::string>> allFound = findAllSupplySchemes(cities, 2);
    EXPECT_EQUAL(allFound,allConfigs);
}
