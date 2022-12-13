#include "MatchingUtilities.h"

Map<std::string, Map<std::string, int>> fromWeightedLinks(const Vector<WeightedLink>& links) {
    Map<std::string, Map<std::string, int>> result;
    for (const auto& link: links) {
        result[link.from][link.to] = link.cost;
        result[link.to][link.from] = link.cost;
    }
    return result;
}

/* Pairs to world. */
Map<std::string, Set<std::string>> fromLinks(const Vector<Pair>& pairs) {
    Map<std::string, Set<std::string>> result;
    for (const auto& link: pairs) {
        result[link.first()].add(link.second());
        result[link.second()].add(link.first());
    }
    return result;
}

/* Checks if a set of pairs forms a perfect matching. */
bool isPerfectMatching(const Map<std::string, Set<std::string>>& possibleLinks,
                       const Set<Pair>& matching) {
    /* Need to check that
     *
     * 1. each pair is indeed a possible link,
     * 2. each person appears in exactly one pair.
     */
    Set<std::string> used;
    for (const Pair& p: matching) {
        /* Are these folks even in the group of people? */
        if (!possibleLinks.containsKey(p.first())) return false;
        if (!possibleLinks.containsKey(p.second())) return false;

        /* If these people are in the group, are they linked? */
        if (!possibleLinks[p.first()].contains(p.second()) ||
            !possibleLinks[p.second()].contains(p.first())) {
            return false;
        }

        /* Have we seen them before? */
        if (used.contains(p.first()) || used.contains(p.second())) {
            return false;
        }

        /* Add them both. */
        used += p.first();
        used += p.second();
    }

    /* Confirm that's everyone. */
    return used.size() == possibleLinks.size();
}
