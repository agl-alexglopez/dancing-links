#ifndef GENERICOVERLOADS_H
#define GENERICOVERLOADS_H
#include <string>
#include <ostream>
#include <set>
#include <vector>

namespace std {

std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& v);
std::ostream& operator<<(std::ostream& os, const std::set<std::string>& s);
std::ostream& operator<<(std::ostream& os, const std::set<std::set<std::string>>& s);

} // namespace std

#endif // GENERICOVERLOADS_H
