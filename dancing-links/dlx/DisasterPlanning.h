#ifndef DisasterPlanning_Included
#define DisasterPlanning_Included

#include <limits.h>
#include <string>
#include "set.h"
#include "map.h"


/**
 * Given a transportation grid for a country or region, along with the number of cities where disaster
 * supplies can be stockpiled, returns whether it's possible to stockpile disaster supplies in at most
 * the specified number of cities such that each city either has supplies or is adjacent to a city that
 * does.
 * <p>
 * This function can assume that every city is a key in the transportation map and that roads are
 * bidirectional: if there's a road from City A to City B, then there's a road from City B back to
 * City A as well.
 * <p>
 * The number of cities can be zero, but it should never be negative. If it is negative, you
 * should report an error by calling the error() function.
 *
 * @param roadNetwork     The underlying transportation network.
 * @param numCities       How many cities you can afford to put supplies in.
 * @param supplyLocations An outparameter filled in with which cities to choose if a solution exists.
 * @return Whether a solution exists.
 */
bool canBeMadeDisasterReady(const Map<std::string, Set<std::string>>& roadNetwork,
                            int numCities,
                            Set<std::string>& supplyLocations);
/**
 * @brief findAllSupplySchemes  finds every possible configuration of supply distribution with the
 *                              given number of supplies. Only use this function once the optimal
 *                              number of supplies is known. Otherwise it will be slower than
 *                              it already is.
 * @param roadNetwork           Map representing the cities and cities that connect to those cities.
 * @param numSupplies           the integer count of total supplies we have to distribute.
 * @return                      Set of Sets of all distributions.
 */
Set<Set<std::string>> findAllSupplySchemes(const Map<std::string, Set<std::string>>& roadNetwork,
                                           int numSupplies);
#endif
