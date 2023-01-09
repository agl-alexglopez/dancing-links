/**
 * Author: Alexander Lopez
 * File: Resistance.h
 * ------------------
 * This file contains a utility class for defining a Pokemon Resistance. The intended use is for
 * these resistances to be associated with keys. The keys should either be attack types or defensive
 * types, so this is not a generic class. For example, you could have a defensive type in a map
 * like this:
 *
 *      {"Ground-Rock":{"Electric x0.0","Water x4","Fire x0.5"}},
 *
 * or the flipped version of the same setup for attack types like this:
 *
 *      {"Fire":{"Ground-Rock x0.5","Grass-Bug x4"}}.
 *
 * You can see that multiplier remains constant whether we have the defensive type as a key or as
 * a Resistance in the map. That is why I use this to help make answering questions of exact or
 * overlapping cover for both Attack and Defense Pokemon types faster and more convenient. A
 * resistance should always be tied to a key that it interacts with. Maybe there is a better way,
 * but this was a convenient way I found to deal with the added complexity of different multipliers
 * in the Pokemon Type Coverage Problem.
 */
#ifndef RESISTANCE_H
#define RESISTANCE_H
#include <string>
#include <ostream>


class Resistance {
public:
    enum Multiplier {
        /* It would not make sense for someone to let a multiplier in a Resistance default to
         * IMMUNE, because that is a valuable multiplier to have for a Pokemon. Make sure you
         * initialize multipliers unless you want an EMPTY_ placeholder.
         */
        EMPTY_=0,
        IMMUNE,
        FRAC14,  // x0.25 damage aka the fraction 1/4
        FRAC12,  // x0.5 damage aka the fraction 1/2
        NORMAL,
        DOUBLE,
        QUADRU
    };

    Resistance() = default;

    Resistance(const std::string& type, const Multiplier& multiplier);

    std::string type() const;
    Multiplier multiplier() const;


    bool operator< (const Resistance& rhs) const {
        return this->type() < rhs.type();
    }
    bool operator== (const Resistance& rhs) const {
        return this->type() == rhs.type() && this->multiplier() == rhs.multiplier();
    }
    bool operator> (const Resistance& rhs) const {
        return rhs < *this;
    }
    bool operator>= (const Resistance& rhs) const {
        return !(*this < rhs);
    }
    bool operator<= (const Resistance& rhs) const {
        return !(*this > rhs);
    }
    bool operator!= (const Resistance& rhs) const {
        return !(*this == rhs);
    }
private:
    std::string type_;
    Multiplier multiplier_;

};

std::ostream& operator<<(std::ostream& out, const Resistance& res);
std::ostream& operator<<(std::ostream& out, const Resistance::Multiplier& mult);


#endif // RESISTANCE_H
