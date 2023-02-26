#pragma once
#include <string>
#include <ostream>
#include <map>
#include <set>
#include "Src/MatchingUtilities.h"

bool hasFastPerfectMatching(const std::map<std::string, std::set<std::string>>& possibleLinks, std::set<Pair>& matching);
std::set<Pair> fastMaxWeightMatching(const std::map<std::string, std::map<std::string, int>>& possibleLinks);
std::set<Pair> fastMWMCMatching(const std::map<std::string, std::map<std::string, int>>& possibleLinks);

std::ostream& operator<< (std::ostream& out, const Pair& pair);
