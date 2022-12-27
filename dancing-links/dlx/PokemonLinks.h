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
#include "Utilities/PokemonUtilities.h"
#include "Utilities/RankedSet.h"


class PokemonLinks {
public:

    typedef enum CoverageType {
        DEFENSE,
        ATTACK
    }CoverageType;

    typedef struct pokeLink {
        int topOrLen;
        int up;
        int down;
        Resistance::Multiplier multiplier;
    }pokeLink;

    typedef struct typeName {
        std::string name;
        int left;
        int right;
    }typeName;
    explicit PokemonLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions,
                          const CoverageType requestedCoverSolution);

    std::priority_queue<RankedSet<std::string>> getAllCoveredTeams();

    std::priority_queue<RankedSet<std::string>> getAllAttackCoverages();


    /* * * * * * * * * * * * *  Overloaded Debugging Operators  * * * * * * * * * * * * * * * * * */


    friend bool operator==(const pokeLink& lhs, const pokeLink& rhs);

    friend bool operator!=(const pokeLink& lhs, const pokeLink& rhs);

    friend bool operator==(const typeName& lhs, const typeName& rhs);

    friend bool operator!=(const typeName& lhs, const typeName& rhs);

    friend std::ostream& operator<<(std::ostream& os, const pokeLink& pokeLink);

    friend std::ostream& operator<<(std::ostream& os, const typeName& pokeLink);

    friend std::ostream& operator<<(std::ostream& os, const std::vector<pokeLink>& links);

    friend std::ostream& operator<<(std::ostream& os, const std::vector<typeName>& items);

    friend std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& options);

    friend std::ostream& operator<<(std::ostream& os, const PokemonLinks& links);

private:

    const int MAX_TEAM_SIZE=6;

    std::vector<std::string> optionTable_;
    std::vector<typeName> itemTable_;
    std::vector<pokeLink> links_;
    int numItems_;
    int numOptions_;
    const int teamSize_;
    CoverageType requestedCoverSolution_;


    void fillCoverages(std::priority_queue<RankedSet<std::string>>& exactCoverages,
                       RankedSet<std::string>& team,
                       int teamPicks);
    int chooseItem() const;

    std::pair<int,std::string> coverAttackType(int indexInOption);
    void uncoverAttackType(int indexInOption);
    void hideOptions(int indexInOption);
    void unhideOptions(int indexInOption);

    void buildDefenseLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions);
    void buildAttackLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions);

    void initializeColumns(const std::map<std::string,std::set<Resistance>>& typeInteractions,
                           std::unordered_map<std::string,int>& columnBuilder);

    // Dancing links is well suited to internal debugging over just plain unit testing.
    ALLOW_TEST_ACCESS();
};

#endif // POKEMONLINKS_H
