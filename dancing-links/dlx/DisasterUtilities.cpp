#include "DisasterUtilities.h"


Map<std::string, Set<std::string>> makeMap(const Map<std::string, Set<std::string>>& source) {
    Map<std::string, Set<std::string>> result = source;

    for (const std::string& from: source) {
        for (const std::string& to: source[from]) {
            result[from] += to;
            result[to] += from;
        }
    }

    return result;
}

bool checkCovered(const std::string& city,
                  const Map<std::string, Set<std::string>>& roadNetwork,
                  const Set<std::string>& supplyLocations) {
    if (supplyLocations.contains(city)) return true;

    for (const std::string& neighbor: roadNetwork[city]) {
        if (supplyLocations.contains(neighbor)) return true;
    }

    return false;
}

Map<std::string, Set<std::string>> makeSymmetric(const Map<std::string, Set<std::string>>& source) {
    Map<std::string, Set<std::string>> result = source;

    for (const std::string& from: source) {
        for (const std::string& to: source[from]) {
            result[from] += to;
            result[to] += from;
        }
    }

    return result;
}

bool isCovered(const std::string& city,
               const Map<std::string, Set<std::string>>& roadNetwork,
               const Set<std::string>& supplyLocations) {
    if (supplyLocations.contains(city)) return true;

    for (const std::string& neighbor: roadNetwork[city]) {
        if (supplyLocations.contains(neighbor)) return true;
    }

    return false;
}
