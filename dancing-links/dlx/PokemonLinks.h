#ifndef POKEMONLINKS_H
#define POKEMONLINKS_H
#include <map>
#include <string>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>
#include "GUI/SimpleTest.h"
#include "Utilities/PokemonUtilities.h"
#include "Utilities/RankedSet.h"


class PokemonLinks {
public:

    const std::size_t MAX_OUTPUT_SIZE=10000;

    typedef enum CoverageType {
        DEFENSE,
        ATTACK
    }CoverageType;

    typedef struct pokeLink {
        int topOrLen;
        int up;
        int down;
        Resistance::Multiplier multiplier;
        int depthTag;
    }pokeLink;

    typedef struct typeName {
        std::string name;
        int left;
        int right;
    }typeName;


    explicit PokemonLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions,
                          const CoverageType requestedCoverSolution);

    explicit PokemonLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions,
                          const std::set<std::string>& attackTypes);

    std::set<RankedSet<std::string>> getExactTypeCoverage();

    std::set<RankedSet<std::string>> getOverlappingTypeCoverage();

    bool reachedOutputLimit();


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

    friend std::ostream& operator<<(std::ostream& os, const std::set<RankedSet<std::string>>& solution);

    friend std::ostream& operator<<(std::ostream& os, const PokemonLinks& links);

private:

    const int MAX_TEAM_SIZE=6;
    const int MAX_ATTACK_SLOTS=24;

    std::vector<std::string> optionTable_;
    std::vector<typeName> itemTable_;
    std::vector<pokeLink> links_;
    int numItems_;
    int numOptions_;
    CoverageType requestedCoverSolution_;
    bool hitLimit_;


    void fillExactCoverages(std::set<RankedSet<std::string>>& exactCoverages,
                            RankedSet<std::string>& coverage,
                            int depthLimit);

    void fillOverlappingCoverages(std::set<RankedSet<std::string>>& overlappingCoverages,
                                  RankedSet<std::string>& coverage,
                                  int depthTag);

    int chooseItem() const;

    std::pair<int,std::string> coverType(int indexInOption);
    void uncoverType(int indexInOption);
    void hideOptions(int indexInOption);
    void unhideOptions(int indexInOption);

    std::pair<int,std::string> looseCoverType(int indexInOption, int depthTag);
    void looseUncoverType(int indexInOption);

    void buildDefenseLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions);
    void buildAttackLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions);

    void initializeColumns(const std::map<std::string,std::set<Resistance>>& typeInteractions,
                           std::unordered_map<std::string,int>& columnBuilder,
                           CoverageType requestedCoverage);

    // Dancing links is well suited to internal debugging over just plain unit testing.
    ALLOW_TEST_ACCESS();
};

#endif // POKEMONLINKS_H
