#include "Resistance.h"


/* * * * * * * * * * * * *     Resistance Helper Class      * * * * * * * * * * * * * * * * * * * */

Resistance::Resistance(const std::string& type, const Multiplier& multiplier)
    : type_(type),
      multiplier_(multiplier) {
}

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
