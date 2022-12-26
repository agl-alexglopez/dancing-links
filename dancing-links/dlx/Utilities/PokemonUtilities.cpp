#include "PokemonUtilities.h"


/* * * * * * * * * * * * *     Resistance Helper Class      * * * * * * * * * * * * * * * * * * * */

Resistance::Resistance(const std::string& type, const Multiplier& multiplier) :
                       type_(type),
                       multiplier_(multiplier){}

std::string Resistance::type() const {
    return type_;
}

Resistance::Multiplier Resistance::multiplier() const {
    return multiplier_;
}

std::ostream& operator<<(std::ostream& out, const Resistance& res) {
    out << res.type() << " x";
    switch(res.multiplier()) {
        case Resistance::EMPTY_:
            out << "NIL";
        break;
        case Resistance::IMMUNE:
            out << "0.0";
        break;
        case Resistance::FRAC14:
            out << "0.25";
        break;
        case Resistance::FRAC12:
            out << "0.5";
        break;
        case Resistance::NORMAL:
            out << "1.0";
        break;
        case Resistance::DOUBLE:
            out << "2.0";
        break;
        case Resistance::QUADRU:
            out << "4.0";
        break;
    }
    out << std::endl;
    return out;
}

std::ostream& operator<<(std::ostream& out, const Resistance::Multiplier& mult) {
    out << "Resistance::";
    switch(mult) {
        case Resistance::EMPTY_:
            out << "EMPTY_";
        break;
        case Resistance::IMMUNE:
            out << "IMMUNE";
        break;
        case Resistance::FRAC14:
            out << "FRAC14";
        break;
        case Resistance::FRAC12:
            out << "FRAC12";
        break;
        case Resistance::NORMAL:
            out << "NORMAL";
        break;
        case Resistance::DOUBLE:
            out << "DOUBLE";
        break;
        case Resistance::QUADRU:
            out << "QUADRU";
        break;
    }
    return out;
}

/* * * * * * * * * * * * *     Ranked Cover Helper Class    * * * * * * * * * * * * * * * * * * * */


RankedCover::RankedCover(const int& rank, const std::set<std::string>& cover) : rank_(rank),
                                                                                cover_(cover){}

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

RankedCover::iterator RankedCover::begin() {
    return cover_.begin();
}

RankedCover::const_iterator RankedCover::begin() const {
    return cover_.begin();
}

RankedCover::iterator RankedCover::end() {
    return cover_.end();
}

RankedCover::const_iterator RankedCover::end() const {
    return cover_.end();
}

std::ostream& operator<<(std::ostream& out, const RankedCover& rc) {
    out << "{" << rc.rank_ << ",{";
    for (const auto& s : rc.cover_) {
        out << "\"" << s << "\",";
    }
    out << "}}" << std::endl;
    return out;
}
