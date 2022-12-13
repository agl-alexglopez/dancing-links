#ifndef MATCHINGUTILITIES_H
#define MATCHINGUTILITIES_H
#include <string>
#include "map.h"
#include "vector.h"
#include "set.h"

/* Unordered pair of strings. */
class Pair {
public:
    /* Creates a pair of two empty strings. */
    Pair() = default;

    /* Sort the strings. */
    Pair(const std::string& one, const std::string& two) {
        if (one < two) {
            one_ = one;
            two_ = two;
        } else {
            one_ = two;
            two_ = one;
        }
    }

    const std::string& first()  const {
        return one_;
    }
    const std::string& second() const {
        return two_;
    }

    // Helper to fill a Pair type during dancing links. It does nothing if the pair is complete.
    void add(const std::string& add) {
        if (one_ == "") {
            if (add < two_) {
                one_ = add;
            } else {
                one_ = two_;
                two_ = add;
            }
        } else if (two_ == "") {
            if (add < one_) {
                two_ = one_;
                one_ = add;
            } else {
                two_ = add;
            }
        }
    }

    bool operator< (const Pair& rhs) const {
        /* Compare first components. */
        auto comp = first().compare(rhs.first());
        if (comp != 0) return comp < 0;

        return second() < rhs.second();
    }

    explicit operator bool() const {
        return this->first() != "" || this->second() != "";
    }

    bool operator== (const Pair& rhs) const {
        return first() == rhs.first() && second() == rhs.second();
    }

    bool operator> (const Pair& rhs) const {
        return rhs < *this;
    }
    bool operator>= (const Pair& rhs) const {
        return !(*this < rhs);
    }
    bool operator<= (const Pair& rhs) const {
        return !(*this > rhs);
    }
    bool operator!= (const Pair& rhs) const {
        return !(*this == rhs);
    }

private:
    std::string one_;
    std::string two_;
};

std::ostream& operator<< (std::ostream& out, const Pair& pair);

/* Utility to go from a list of triples to a world. */
struct WeightedLink {
    std::string from;
    std::string to;
    int cost;
};

Map<std::string, Map<std::string, int>> fromWeightedLinks(const Vector<WeightedLink>& links);

/* Pairs to world. */
Map<std::string, Set<std::string>> fromLinks(const Vector<Pair>& pairs);

/* Checks if a set of pairs forms a perfect matching. */
bool isPerfectMatching(const Map<std::string, Set<std::string>>& possibleLinks,
                       const Set<Pair>& matching);


#endif // MATCHINGUTILITIES_H
