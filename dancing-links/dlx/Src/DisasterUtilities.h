#ifndef DISASTERUTILITIES_H
#define DISASTERUTILITIES_H
#include <string>
#include <set>
#include <map>


/* This is a helper function that's useful for designing test cases. You give it a std::map
 * of cities and what they're adjacent to, and it then produces a new std::map where if city
 * A links to city B, then city B links back to city A. We recommend using this helper
 * function when writing tests, though you won't need it in your implementation of the main
 * canBeMadeDisasterReady function.
 */
std::map<std::string, std::set<std::string>> makeMap(const std::map<std::string, std::set<std::string>>& source);

/* This helper function tests whether a city has been covered by a set of supply locations
 * and is used by our testing code. You're welcome to use it in your tests as well!
 */
bool checkCovered(const std::string& city,
                  const std::map<std::string, std::set<std::string>>& roadNetwork,
                  const std::set<std::string>& supplyLocations);

/* This is a helper function that's useful for designing test cases. You give it a std::map
 * of cities and what they're adjacent to, and it then produces a new std::map where if city
 * A links to city B, then city B links back to city A. We recommend using this helper
 * function when writing tests, though you won't need it in your implementation of the main
 * canBeMadeDisasterReady function.
 */
std::map<std::string, std::set<std::string>> makeSymmetric(const std::map<std::string, std::set<std::string>>& source);

/* This helper function tests whether a city has been covered by a set of supply locations
 * and is used by our testing code. You're welcome to use it in your tests as well!
 */
bool isCovered(const std::string& city,
               const std::map<std::string, std::set<std::string>>& roadNetwork,
               const std::set<std::string>& supplyLocations);

#endif // DISASTERUTILITIES_H
