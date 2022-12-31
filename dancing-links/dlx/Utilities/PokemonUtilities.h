#ifndef POKEMONUTILITIES_H
#define POKEMONUTILITIES_H
#include <map>
#include <string>
#include <set>
#include <vector>
#include <ostream>


class Resistance {
public:
    typedef enum Multiplier {
        // The default constructor should initialize our enum with an explicit placeholder.
        EMPTY_=0,
        IMMUNE,
        FRAC14,
        FRAC12,
        NORMAL,
        DOUBLE,
        QUADRU
    }Multiplier;

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


#endif // POKEMONUTILITIES_H
