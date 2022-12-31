#pragma once
#include <string>
#include <ostream>
#include "map.h"
#include "set.h"
#include "Utilities/MatchingUtilities.h"


bool hasPerfectMatching(const Map<std::string, Set<std::string>>& possibleLinks, Set<Pair>& matching);

Vector<Set<Pair>>
getAllPerfectMatchings(const Map<std::string, Set<std::string>>& possibleLinks);

Set<Pair> maximumWeightMatching(const Map<std::string, Map<std::string, int>>& possibleLinks);
