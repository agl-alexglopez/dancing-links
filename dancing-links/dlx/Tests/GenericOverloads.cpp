#include "GenericOverloads.h"
#include "Src/DisasterLinks.h"
#include "Src/DisasterTags.h"

std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& v) {
    for (const std::string& s: v) {
        os << s << ',';
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::set<std::string>& s) {
    for (const std::string& elem: s) {
        os << elem << ',';
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::set<std::set<std::string>>& s) {
    for (const std::set<std::string>& elem: s) {
        os << elem << ',';
    }
    return os;

}
