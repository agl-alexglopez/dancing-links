#pragma once
#include <string>
#include <ostream>
#include "map.h"
#include "set.h"
#include "MatchingUtilities.h"

bool hasFastPerfectMatching(const Map<std::string, Set<std::string>>& possibleLinks, Set<Pair>& matching);
Set<Pair> fastMaxWeightMatching(const Map<std::string, Map<std::string, int>>& possibleLinks);
Set<Pair> fastMWMCMatching(const Map<std::string, Map<std::string, int>>& possibleLinks);

std::ostream& operator<< (std::ostream& out, const Pair& pair);
