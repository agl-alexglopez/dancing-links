#pragma once
#ifndef PartnerLinks_H
#define PartnerLinks_H
#include <limits.h>
#include <string>
#include "Matchmaker.h"
#include "GUI/SimpleTest.h"
#include "vector.h"
#include "hashmap.h"
#include "map.h"
#include "set.h"


class PartnerLinks {

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
        int numPeople;
        int numPairings;
        bool hasSingleton;
        bool isWeighted;
    }Network;

    PartnerLinks(const Map<std::string, Set<std::string>>& possibleLinks);
    PartnerLinks(const Map<std::string, Map<std::string, int>>& possibleLinks);

    friend bool operator==(const personLink& lhs, const personLink& rhs);

    friend bool operator!=(const personLink& lhs, const personLink& rhs);

    friend bool operator==(const personName& lhs, const personName& rhs);

    friend bool operator!=(const personName& lhs, const personName& rhs);

    friend std::ostream& operator<<(std::ostream& os, const personLink& city);

    friend std::ostream& operator<<(std::ostream& os, const personName& city);

    friend std::ostream& operator<<(std::ostream&os, const Vector<personLink>& links);

    friend std::ostream& operator<<(std::ostream&os, const PartnerLinks& links);

    bool hasPerfectLinks(Set<Pair>& pairs);
    Set<Pair> getMaxWeightMatching();

private:

    Network dlx;

    bool isPerfectMatching(Set<Pair>& pairs);
    void fillWeights(std::pair<int,Set<Pair>>& soFar,
                     std::pair<int,Set<Pair>>& winner);

    int choosePerson();
    int chooseWeightedPerson();

    Pair coverPairing(int index);
    std::pair<int,Pair> coverWeightedPair(int index);
    void coverPerson(int index);
    void uncoverPerson(int index);

    void uncoverPairing(int index);
    void hidePairings(personLink& start, int index);
    void unhidePairings(personLink&start, int index);
    inline void toPairIndex(int& index);


    void initializeHeaders(const Map<std::string, Set<std::string>>& possibleLinks,
                           HashMap<std::string,int>& columnBuilder);
    void initializeHeaders(const Map<std::string, Map<std::string,int>>& possibleLinks,
                           HashMap<std::string,int>& columnBuilder);

    void setPerfectPairs(const std::string& person,
                         const Set<std::string>& personPairs,
                         HashMap<std::string,int>& columnBuilder,
                         Set<Pair>& seenPairs,
                         int& index,
                         int& spacerTitle);
    void setWeightedPairs(const std::string& person,
                          const Map<std::string,int>& personPairs,
                          HashMap<std::string,int>& columnBuilder,
                          Set<Pair>& seenPairs,
                          int& index);

    ALLOW_TEST_ACCESS();
};

#endif // PartnerLinks_H
