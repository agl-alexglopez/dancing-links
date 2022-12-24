#include "PokemonUtilities.h"


/* * * * * * * * * * * * *     Resistance Helper Class      * * * * * * * * * * * * * * * * * * * */


Resistance::Resistance() : type_(""), multiplier_(ZERO){}

Resistance::Resistance(const std::string& type, const Multiplier& multiplier) :
                       type_(type),
                       multiplier_(multiplier){}

Resistance::Multiplier Resistance::multiplier() const {
    return multiplier_;
}

std::string Resistance::type() const {
    return type_;
}

std::ostream& operator<<(std::ostream& out, const Resistance& res) {
    out << res.type_ << "x";
    switch(res.multiplier_) {
        case Resistance::ZERO:
            out << "0.0";
        break;
        case Resistance::ONE_QUARTER:
            out << "0.25";
        break;
        case Resistance::ONE_HALF:
            out << "0.5";
        break;
        case Resistance::ONE:
            out << "1.0";
        break;
        case Resistance::TWO:
            out << "2.0";
        break;
        case Resistance::FOUR:
            out << "4.0";
        break;
    }
    out << std::endl;
    return out;
}

/* * * * * * * * * * * * *     Ranked Cover Helper Class    * * * * * * * * * * * * * * * * * * * */


RankedCover::RankedCover() : rank_(0), cover_({}){}

RankedCover::RankedCover(int rank, const std::set<std::string>& coverage) :
                               rank_(rank),
                               cover_(coverage) {}

std::size_t RankedCover::size() const {
    return cover_.size();
}

int RankedCover::rank() const {
    return rank_;
}

void RankedCover::insert(const std::string& pokemonType) {
    cover_.insert(pokemonType);
}

void RankedCover::remove(const std::string& pokemonType) {
    cover_.erase(pokemonType);
}

void RankedCover::add(const int rankChange) {
    rank_ += rankChange;
}

void RankedCover::subtract(const int rankChange) {
    rank_ -= rankChange;
}

std::ostream& operator<<(std::ostream& out, const RankedCover& rc) {
    out << "{" << rc.rank_ << ",{";
    for (const auto& s : rc.cover_) {
        out << "\"" << s << "\",";
    }
    out << std::endl;
    return out;
}
