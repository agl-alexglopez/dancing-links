#include "PerfectLinks.h"

PerfectLinks::PerfectLinks(const Map<std::string, Set<std::string>>& possibleLinks) {
    (void) possibleLinks;
}


/* * * * * * * * * * * * * * * *   Overloaded Operators for Debugging   * * * * * * * * * * * * * */


bool operator==(const PerfectLinks::personLink& lhs, const PerfectLinks::personLink& rhs) {
    return lhs.topOrLen == rhs.topOrLen && lhs.up == rhs.up && lhs.down == rhs.down;
}

bool operator!=(const PerfectLinks::personLink& lhs, const PerfectLinks::personLink& rhs) {
    return !(lhs == rhs);
}

bool operator==(const PerfectLinks::personName& lhs, const PerfectLinks::personName& rhs) {
    return lhs.name == rhs.name && lhs.left == rhs.left && lhs.right == rhs.right;
}

bool operator!=(const PerfectLinks::personName& lhs, const PerfectLinks::personName& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, const PerfectLinks::personLink& person) {
    os << "{ topOrLen: " << person.topOrLen
       << ", up: " << person.up << ", down: " << person.down << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const PerfectLinks::personName& name) {
    os << "{ name: " << name.name << ", left: " << name.left << ", right: " << name.right << " }";
    return os;
}

std::ostream& operator<<(std::ostream&os, const Vector<PerfectLinks::personLink>& links) {
    for (const auto& item : links) {
        if (item.topOrLen <= 0) {
            os << std::endl;
        }
        os << "{" << item.topOrLen << "," << item.up << "," << item.down << "}";
    }
    os << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream&os, const PerfectLinks& links) {
    for (const auto& header : links.dlx.lookupTable) {
        os << "{" << header.name << "," << header.left << "," << header.right << "},";
    }
    os << std::endl;
    for (const auto& item : links.dlx.links) {
        if (item.topOrLen <= 0) {
            os << std::endl;
        }
        os << "{" << item.topOrLen << "," << item.up << "," << item.down << "}";
    }
    os << std::endl;
    return os;
}
