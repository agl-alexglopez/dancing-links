#ifndef POKEMONLINKS_H
#define POKEMONLINKS_H
#include <map>
#include <string>
#include <set>
#include <unordered_map>
#include <queue>
#include <utility>
#include <vector>
#include "GUI/SimpleTest.h"
#include "PokemonUtilities.h"


class PokemonLinks {
public:

    typedef enum CoverageType {
        DEFENSE,
        ATTACK
    }CoverageType;

    explicit PokemonLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions,
                          const CoverageType requestedCoverSolution);

    std::priority_queue<RankedCover> getAllCoveredTeams();

    std::priority_queue<RankedCover> getAllAttackCoverages();

private:

    typedef struct pokeLink {
        int topOrLen;
        int up;
        int down;
        Resistance::Multiplier multiplier;
    }defenseLink;

    typedef struct typeName {
        std::string name;
        int left;
        int right;
    }typeName;

    std::vector<std::string> optionTable_;
    std::vector<typeName> itemTable_;
    std::vector<pokeLink> pokeLinks_;
    int numItems_;
    int numOptions_;
    CoverageType requestedCoverSolution_;

    void buildDefenseLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions);
    void buildAttackLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions);

    void initializeColumns(const std::map<std::string,std::set<Resistance>>& typeInteractions,
                           std::unordered_map<std::string,int>& columnBuilder);


    // Dancing links is well suited to internal debugging over just plain unit testing.
    ALLOW_TEST_ACCESS();
};

#endif // POKEMONLINKS_H
