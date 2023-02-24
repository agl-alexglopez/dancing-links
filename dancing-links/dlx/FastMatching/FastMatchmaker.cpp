#include "FastMatchmaker.h"
#include "FastMatching/graphtypes.h"
#include <limits>
using namespace std;

std::set<Pair> fastMaxWeightMatching(const std::map<string, std::map<string, int>>& links) {
    /* Linearize links into a predictable order. */
    std::map<int, string> order;
    std::map<string, int> toIndex;
    for (const auto& name: links) {
        int index = order.size() + 1;
        order[index] = name.first;
        toIndex[name.first] = index;
    }

    /* Build the graph. */
    auto graph = EdRothberg::NewGraph(links.size());
    for (const auto& src: links) {
        for (const auto& dst: src.second) {
            if (toIndex[src.first] < toIndex[dst.first]) {
                EdRothberg::AddEdge(graph, toIndex[src.first], toIndex[dst.first], links.at(src.first).at(dst.first));
            }
        }
    }

    /* Run the matching. */
    auto matching = EdRothberg::WeightedMatch(graph);
    EdRothberg::FreeGraph(graph);

    /* Convert to pairs. */
    std::set<Pair> result;
    for (const auto& entry: matching) {
        result.insert({ order[entry.first], order[entry.second] });
    }

    return result;
}

namespace {
    std::set<Pair> fastMaxCardinalityMatching(const std::map<string, std::set<string>>& links) {
        /* Transform to a graph where each edge has cost 1. */
        std::map<string, std::map<string, int>> newLinks;
        for (const auto& src: links) {
            for (const auto& dst: src.second) {
                newLinks[src.first][dst] = 1;
            }
        }

        /* Now find a maximum-weight matching in this new graph. */
        return fastMaxWeightMatching(newLinks);
    }
}

bool hasFastPerfectMatching(const std::map<string, std::set<string>>& possibleLinks, std::set<Pair>& matching) {
    matching = fastMaxCardinalityMatching(possibleLinks);
    return matching.size() * 2 == possibleLinks.size();
}

std::set<Pair> fastMWMCMatching(const std::map<string, std::map<string, int>>& possibleLinks) {
    /* Artificially boost all the edge costs by an amount such that it is always better to
     * have more edges than fewer edges.
     */
    int maxEdge = 0;
    for (const auto& src: possibleLinks) {
        for (const auto& dst: src.second) {
            maxEdge = max(maxEdge, possibleLinks.at(src.first).at(dst.first));
        }
    }

    std::map<string, std::map<string, int>> newLinks;
    for (const auto& src: possibleLinks) {
        for (const auto& dst: src.second) {
            /* Each edge is boosted by the score you'd get if everything was paired at the
             * maximum value. Now, any matching has to include the maximum number of edges.
             */
            newLinks[src.first][dst.first] = possibleLinks.at(src.first).at(dst.first) + (maxEdge + 1) * possibleLinks.size();
        }
    }

    return fastMaxWeightMatching(newLinks);
}
