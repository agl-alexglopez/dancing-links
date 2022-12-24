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
        ZERO,
        ONE_QUARTER,
        ONE_HALF,
        ONE,
        TWO,
        FOUR
    }Multiplier;

    explicit Resistance();
    explicit Resistance(const std::string& type, const Multiplier& multiplier);

    Multiplier multiplier() const;
    std::string type() const;

    friend std::ostream& operator<<(std::ostream& out, const Resistance& res);

    bool operator< (const Resistance& rhs) const {
        return this->type_ < rhs.type_;
    }
    bool operator== (const Resistance& rhs) const {
        return this->type_ == rhs.type_ && this->multiplier_ == rhs.multiplier_;
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


class RankedCover {

public:

    explicit RankedCover();
    explicit RankedCover(int rank, const  std::set<std::string>& coverage);

    std::size_t size() const;
    int rank() const;
    void insert(const std::string& pokemonType);
    void remove(const std::string& pokemonType);
    void add(const int rankChange);
    void subtract(const int rankChange);

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
