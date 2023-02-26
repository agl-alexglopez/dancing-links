#include "DisasterUtilities.h"


std::map<std::string, std::set<std::string>> makeMap(const std::map<std::string, std::set<std::string>>& source) {
    std::map<std::string, std::set<std::string>> result = source;

    for (const auto& from: source) {
        for (const std::string& to: from.second) {
            result[from.first].insert(to);
            result[to].insert(from.first);
        }
    }

    return result;
}

bool checkCovered(const std::string& city,
                  const std::map<std::string, std::set<std::string>>& roadNetwork,
                  const std::set<std::string>& supplyLocations) {
    if (supplyLocations.count(city)) return true;

    for (const std::string& neighbor: roadNetwork.at(city)) {
        if (supplyLocations.count(neighbor)) return true;
    }

    return false;
}

std::map<std::string, std::set<std::string>> makeSymmetric(const std::map<std::string, std::set<std::string>>& source) {
    std::map<std::string, std::set<std::string>> result = source;

    for (const auto& from: source) {
        for (const std::string& to: from.second) {
            result[from.first].insert(to);
            result[to].insert(from.first);
        }
    }

    return result;
}

bool isCovered(const std::string& city,
               const std::map<std::string, std::set<std::string>>& roadNetwork,
               const std::set<std::string>& supplyLocations) {
    if (supplyLocations.count(city)) return true;

    for (const std::string& neighbor: roadNetwork.at(city)) {
        if (supplyLocations.count(neighbor)) return true;
    }

    return false;
}
