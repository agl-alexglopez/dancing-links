#pragma once
#include <string>
#include <ostream>
#include "map.h"
#include "set.h"
#include "MatchingUtilities.h"


bool hasPerfectMatching(const Map<std::string, Set<std::string>>& possibleLinks, Set<Pair>& matching);
Set<Pair> maximumWeightMatching(const Map<std::string, Map<std::string, int>>& possibleLinks);
