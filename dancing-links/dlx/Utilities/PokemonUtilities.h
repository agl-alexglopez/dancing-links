#ifndef POKEMONUTILITIES_H
#define POKEMONUTILITIES_H
#include <map>
#include <string>
#include <set>
#include <vector>
#include <iostream>


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

class RankedCover {

public:

    RankedCover() = default;

    RankedCover(const int& rank, const std::set<std::string>& cover);

    std::size_t size() const;
    int rank() const;
    void insert(const std::string& pokemonType);
    void remove(const std::string& pokemonType);
    void add(const int rankChange);
    void subtract(const int rankChange);


    using container = std::set<std::string>;
    using iterator = container::iterator;
    using const_iterator = container::const_iterator;
    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    friend std::ostream& operator<<(std::ostream& out, const RankedCover& rc);

    bool operator< (const RankedCover& rhs) const {
        return this->rank_ < rhs.rank_;
    }
    explicit operator bool() const {
        return this->rank_ != 0 || this->cover_.size() != 0;
    }
    bool operator== (const RankedCover& rhs) const {
        return this->rank_ == rhs.rank_ && this->cover_ == rhs.cover_;
    }
    bool operator> (const RankedCover& rhs) const {
        return rhs < *this;
    }
    bool operator>= (const RankedCover& rhs) const {
        return !(*this < rhs);
    }
    bool operator<= (const RankedCover& rhs) const {
        return !(*this > rhs);
    }
    bool operator!= (const RankedCover& rhs) const {
        return !(*this == rhs);
    }

private:
    int rank_;
    std::set<std::string> cover_;
};



#endif // POKEMONUTILITIES_H
