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

/**
 * @brief hasOverlappingCover  determines if it is possible to cover a set of items with options
 *                             that cover various combinations of those items. An overlapping cover
 *                             differs from an exact cover in that it allows for multiple options
 *                             to cover the same items. The goal is to simply cover the items with
 *                             any available combination of options.
 * @param links                the dancing links object we take in to perform our cover search.
 * @param depthLimit           the limit we place on how many options we can choose.
 * @param selectedOptions      the options we select as proof of our cover search.
 * @return                     true if solution false if not. Output param is full for true.
 */
bool hasOverlappingCover(DisasterLinks& links, int depthLimit,
                           std::set<std::string>& selectedOptions);

/**
 * Overloaded version of the above function for a DisasterTags object.
 */
bool hasOverlappingCover(DisasterTags& links, int depthLimit,
                           std::set<std::string>& selectedOptions);

/**
 * @brief getAllOverlappingCovers  finds every possible way to cover items with the provided options
 *                                 in a DisasterLinks object. Overlapping covers allow for multiple
 *                                 options to cover the same items.
 * @param links                    the dancing links class on which we perform a cover search.
 * @param depthLimit               the limit on the number of options we can choose.
 * @return                         the set of sets of options we chose.
 */
std::set<std::set<std::string>> getAllOverlappingCovers(DisasterLinks& links, int depthLimit);

/**
 * Overloaded version of the above function for a DisasterTags object.
 */
std::set<std::set<std::string>> getAllOverlappingCovers(DisasterTags& links, int depthLimit);

/**
 * @brief hasExactCover    determines if an exact cover is possible given the items and options
 *                         available to cover those items. An exact cover is one where the options
 *                         we choose cover every item exactly once across all options. This is
 *                         different from an overlapping cover that allows for multiple options to
 *                         cover the same items.
 * @param links            the dancing links class on which we perform an exact cover search.
 * @param selectedOptions  the options we have selected that satisfy an exact cover of items.
 * @return                 true if the output parameter is full and we found a cover, false if not.
 */
bool hasExactCover(PartnerLinks& links, std::set<Pair>& selectedOptions);

/**
 * @brief getAllExactCovers  finds every possible exact cover of the given items with the provided
 *                           options. Because this is intended for the perfect matching problem on
 *                           a PartnerLinks object, no depth limit is required as everyone must
 *                           be paired off.
 * @param links              the PartnerLinks object on which we perform an exact cover search.
 * @return                   the vector of sets of Pairs that satisfy an exact cover.
 */
std::vector<std::set<Pair>> getAllExactCovers(PartnerLinks& links);

/**
 * @brief getMaxWeightMatching  finds the maximum possible weight matching of items given the
 *                              options to match or "cover" those items.
 * @param links                 the PartnerLinks object on which we search for max weight matching.
 * @return                      the set of Pairs with maximum possible weight.
 */
std::set<Pair> getMaxWeightMatching(PartnerLinks& links);


} // namespace DancingLinks


#endif // DANCINGLINKS_H
