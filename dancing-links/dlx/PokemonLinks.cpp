#include "PokemonLinks.h"
#include <cstdlib>

PokemonLinks::PokemonLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions,
                           const CoverageType requestedCoverSolution) :
                           optionTable_({}),
                           itemTable_({}),
                           pokeLinks_({}),
                           numItems_(0),
                           numOptions_(0),
                           requestedCoverSolution_(requestedCoverSolution){
    if (requestedCoverSolution == DEFENSE) {
        buildDefenseLinks(typeInteractions);
    } else if (requestedCoverSolution == ATTACK){
        buildAttackLinks(typeInteractions);
    } else {
        std::cerr << "Invalid requested cover solution. Choose ATTACK or DEFENSE." << std::endl;
        std::abort();
    }
}

void PokemonLinks::buildDefenseLinks(const std::map<std::string,std::set<Resistance>>&
                                     typeInteractions) {
    std::set<std::string> generationTypes = {};
    for (const Resistance& res : typeInteractions.begin()->second) {
        generationTypes.insert(res.type());
    }
    std::unordered_map<std::string,int> columnBuilder = {};
    optionTable_.push_back("");
    itemTable_.push_back({"", 0, 1});
    pokeLinks_.push_back({0, 0, 0, Resistance::ZERO});
    int index = 1;
    for (const std::string& type : generationTypes) {

        columnBuilder[type] = index;

        itemTable_.push_back({"", index - 1, index + 1});
        itemTable_[0].left++;

        pokeLinks_.push_back({0, index, index, Resistance::ZERO});

        numItems_++;
        index++;
    }
    itemTable_[itemTable_.size() - 1].right = 0;

    initializeColumns(typeInteractions, columnBuilder);

}

void PokemonLinks::initializeColumns(const std::map<std::string,std::set<Resistance>>&
                                     typeInteractions,
                                     std::unordered_map<std::string,int>& columnBuilder) {
    int previousSetSize = pokeLinks_.size();
    int currentLinksIndex = pokeLinks_.size();
    int typeLookupIndex = 1;
    for (const auto& type : typeInteractions) {

        int setSize = type.second.size();
        pokeLinks_.push_back({-typeLookupIndex,
                              currentLinksIndex - previousSetSize,
                              currentLinksIndex + setSize,
                              Resistance::ZERO});

        for (const Resistance& singleType : type.second) {

            /* Important consideration for this algorithm. I am only interested in damage
             * resistances better than normal. So "covered" for a pokemon team means you found at
             * most 6 Pokemon that give you some level of resistance to all types in the game. You
             * could have Pokemon with x0.0, x0.25, or x0.5 resistances, but no higher.
             */

            if (singleType.multiplier() < Resistance::ONE) {
                currentLinksIndex++;

                std::string sType = singleType.type();
                pokeLinks_[pokeLinks_[columnBuilder[sType]].down].topOrLen++;

                // A single item in a circular doubly linked list points to itself.
                pokeLinks_.push_back({pokeLinks_[columnBuilder[sType]].down,
                                      currentLinksIndex, currentLinksIndex,
                                      singleType.multiplier()});

                // This is the necessary adjustment to the column header's up field for a given item.
                pokeLinks_[pokeLinks_[columnBuilder[sType]].down].up = currentLinksIndex;
                // The current node is now the new tail in a vertical circular linked list for an item.
                pokeLinks_[currentLinksIndex].up = columnBuilder[sType];
                pokeLinks_[currentLinksIndex].down = pokeLinks_[columnBuilder[sType]].down;
                // Update the old tail to reflect the new addition of an item in its option.
                pokeLinks_[columnBuilder[sType]].down = currentLinksIndex;
                // Similar to a previous/current coding pattern but in an above/below column.
                columnBuilder[sType] = currentLinksIndex;
            }
        }

        optionTable_.push_back(type.first);
        typeLookupIndex++;
        currentLinksIndex++;
        previousSetSize = setSize;
    }

}

void PokemonLinks::buildAttackLinks(const std::map<std::string,std::set<Resistance>>&
                                    typeInteractions) {
    (void) typeInteractions;
}
