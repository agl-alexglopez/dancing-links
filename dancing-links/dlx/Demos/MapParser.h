#ifndef MapParser_Included
#define MapParser_Included
#include "set.h"
#include "map.h"
#include "gtypes.h"
#include <string>
#include <istream>

/**
 * Type representing a test case for the Disaster Preparation problem.
 */
struct MapTest {
    Map<std::string, Set<std::string>> network; // The road network
    Map<std::string, GPoint> cityLocations;     // Where each city should be drawn
};

/**
 * Given a stream pointing at a test case for Disaster Preparation,
 * pulls the data from that test case.
 *
 * @param source The stream containing the test case.
 * @return A test case from the file.
 * @throws ErrorException If an error occurs or the file is invalid.
 */
MapTest loadDisaster(std::istream& source);

#endif
