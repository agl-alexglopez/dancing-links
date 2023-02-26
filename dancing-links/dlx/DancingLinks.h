#ifndef DANCINGLINKS_H
#define DANCINGLINKS_H
#include <string>
#include <set>
#include <vector>
#include "Src/MatchingUtilities.h"
#include "Src/DisasterLinks.h"
#include "Src/DisasterTags.h"
#include "Src/PartnerLinks.h"


namespace DancingLinks {
class DisasterLinks;
class DisasterTags;
class PartnerLinks;

bool hasOverlappingCover(DisasterLinks& links, int depthLimit, std::set<std::string>& selectedOptions);

bool hasOverlappingCover(DisasterTags& links, int depthLimit, std::set<std::string>& selectedOptions);

std::set<std::set<std::string>> getAllOverlappingCovers(DisasterLinks& links, int numSupplies);

std::set<std::set<std::string>> getAllOverlappingCovers(DisasterTags& links, int numSupplies);

bool hasExactCover(PartnerLinks& links, std::set<Pair>& selectedOptions);

std::vector<std::set<Pair>> getAllExactCovers(PartnerLinks& links);

std::set<Pair> getMaxWeightMatching(PartnerLinks& links);

} // namespace DancingLinks

#endif // DANCINGLINKS_H
