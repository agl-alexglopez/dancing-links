#ifndef RANKEDSET_H
#define RANKEDSET_H
#include <iterator>
#include <ostream>
#include <set>
#include <string>

template<class valueType>
class RankedSet {
public:
    RankedSet() = default;

    RankedSet(const int& rank, const std::set<valueType>& set) : rank_(rank),
                                                                 set_(set) {}

    std::size_t size() const {
        return set_.size();
    }

    int rank() const {
        return rank_;
    }

    void insert(const valueType& elem) {
        set_.insert(elem);
    }

    void insert(const int rank, const valueType& elem) {
        rank_ += rank;
        set_.insert(elem);
    }

    void remove(const valueType& elem) {
        set_.erase(elem);
    }

    void remove(const int rank, const valueType& elem) {
        rank_ -= rank;
        set_.erase(elem);
    }

    void add(const int rankChange) {
        rank_ += rankChange;
    }

    void subtract(const int rankChange) {
        rank_ -= rankChange;
    }

    using container = typename std::set<valueType>;
    using iterator = typename container::iterator;
    using const_iterator = typename container::const_iterator;

    iterator begin() const {
        return set_.begin();
    }

    const_iterator cbegin() const {
        return set_.cbegin();
    }

    iterator end() const {
        return set_.end();
    }

    const_iterator cend() const {
        return set_.cend();
    }

    friend std::ostream& operator<<(std::ostream& out, const RankedSet<valueType>& rs) {
        out << "{" << rs.rank_ << ",{";
        for (const auto& s : rs.set_) {
            out << "\"" << s << "\",";
        }
        out << "}}" << std::endl;
        return out;
    }

    bool operator< (const RankedSet& rhs) const {
        return this->rank_ < rhs.rank_;
    }
    explicit operator bool() const {
        return this->rank_ != 0 || this->cover_.size() != 0;
    }
    bool operator== (const RankedSet<valueType>& rhs) const {
        return this->rank_ == rhs.rank_ && this->set_ == rhs.set_;
    }
    bool operator> (const RankedSet<valueType>& rhs) const {
        return rhs < *this;
    }
    bool operator>= (const RankedSet<valueType>& rhs) const {
        return !(*this < rhs);
    }
    bool operator<= (const RankedSet<valueType>& rhs) const {
        return !(*this > rhs);
    }
    bool operator!= (const RankedSet<valueType>& rhs) const {
        return !(*this == rhs);
    }

private:
    int rank_;
    std::set<valueType> set_;
};

#endif // RANKEDSET_H
