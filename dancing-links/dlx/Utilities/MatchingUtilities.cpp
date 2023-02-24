#include "MatchingUtilities.h"

std::map<std::string, std::map<std::string, int>> fromWeightedLinks(const std::vector<WeightedLink>& links) {
    std::map<std::string, std::map<std::string, int>> result;
    for (const auto& link: links) {
        result[link.from][link.to] = link.cost;
        result[link.to][link.from] = link.cost;
    }
    return result;
}

/* Pairs to world. */
std::map<std::string, std::set<std::string>> fromLinks(const std::vector<Pair>& pairs) {
    std::map<std::string, std::set<std::string>> result;
    for (const auto& link: pairs) {
        result[link.first()].insert(link.second());
        result[link.second()].insert(link.first());
    }
    return result;
}

/* Checks if a set of pairs forms a perfect matching. */
bool isPerfectMatching(const std::map<std::string, std::set<std::string>>& possibleLinks,
                       const std::set<Pair>& matching) {
    /* Need to check that
     *
     * 1. each pair is indeed a possible link,
     * 2. each person appears in exactly one pair.
     */
    std::set<std::string> used;
    for (const Pair& p: matching) {
        /* Are these folks even in the group of people? */
        if (!possibleLinks.count(p.first())) return false;
        if (!possibleLinks.count(p.second())) return false;

        /* If these people are in the group, are they linked? */
        if (!possibleLinks.at(p.first()).count(p.second()) ||
            !possibleLinks.at(p.second()).count(p.first())) {
            return false;
        }

        /* Have we seen them before? */
        if (used.count(p.first()) || used.count(p.second())) {
            return false;
        }

        /* Add them both. */
        used.insert(p.first());
        used.insert(p.second());
    }

    /* Confirm that's everyone. */
    return used.size() == possibleLinks.size();
}

std::ostream& operator<< (std::ostream& out, const std::set<Pair>& pairs) {
    for (const Pair& p : pairs) {
        out << p;
    }
    return out;
}

std::ostream& operator<< (std::ostream& out, const std::vector<std::set<Pair>>& result) {
    for (const auto& set : result) {
        out << set;
    }
    return out;
}
