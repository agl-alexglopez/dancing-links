#pragma once
#include "Utilities/MatchingUtilities.h"
#include <string>
#include <ostream>
#include <map>
#include <set>
#include <vector>


bool hasPerfectMatching(const std::map<std::string, std::set<std::string>>& possibleLinks, std::set<Pair>& matching);

std::vector<std::set<Pair>>
getAllPerfectMatchings(const std::map<std::string, std::set<std::string>>& possibleLinks);

std::set<Pair> maximumWeightMatching(const std::map<std::string, std::map<std::string, int>>& possibleLinks);
