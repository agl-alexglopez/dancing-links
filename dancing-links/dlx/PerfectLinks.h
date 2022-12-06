#pragma once
#ifndef PERFECTLINKS_H
#define PERFECTLINKS_H
#include <limits.h>
#include <string>
#include "Matchmaker.h"
#include "GUI/SimpleTest.h"
#include "vector.h"
#include "hashmap.h"
#include "map.h"
#include "set.h"


class PerfectLinks {

public:

    typedef struct personLink {
        int topOrLen;
        int up;
        int down;
    }personLink;

    typedef struct personName {
        std::string name;
        int left;
        int right;
    }personName;

    typedef struct Network {
        Vector<personName> lookupTable;
        Vector<personLink> links;
        HashMap<std::string,int> headerIndexMap;
        int numPeople;
        int numPairings;
        bool hasSingleton;
    }Network;

    PerfectLinks(const Map<std::string, Set<std::string>>& possibleLinks);

    friend bool operator==(const personLink& lhs, const personLink& rhs);

    friend bool operator!=(const personLink& lhs, const personLink& rhs);

    friend bool operator==(const personName& lhs, const personName& rhs);

    friend bool operator!=(const personName& lhs, const personName& rhs);

    friend std::ostream& operator<<(std::ostream& os, const personLink& city);

    friend std::ostream& operator<<(std::ostream& os, const personName& city);

    friend std::ostream& operator<<(std::ostream&os, const Vector<personLink>& links);

    friend std::ostream& operator<<(std::ostream&os, const PerfectLinks& links);

    bool hasPerfectLinks(Set<Pair>& pairs);

private:

    Network dlx;

    Pair coverPerson(int index);
    void hidePairing(int start);

    ALLOW_TEST_ACCESS();
};

#endif // PERFECTLINKS_H
