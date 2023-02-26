#ifndef GENERICOVERLOADS_H
#define GENERICOVERLOADS_H
#include <string>
#include <ostream>
#include <set>
#include <vector>

std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& v);
std::ostream& operator<<(std::ostream& os, const std::set<std::string>& s);
std::ostream& operator<<(std::ostream& os, const std::set<std::set<std::string>>& s);

#endif // GENERICOVERLOADS_H
