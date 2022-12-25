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
        generationTypes.insert(res.type);
    }
    std::unordered_map<std::string,int> columnBuilder = {};
    optionTable_.push_back("");
    itemTable_.push_back({"", 0, 1});
    pokeLinks_.push_back({0, 0, 0, Resistance::ZERO});
    int index = 1;
    for (const std::string& type : generationTypes) {

        columnBuilder[type] = index;

        itemTable_.push_back({type, index - 1, index + 1});
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
        // We will lookup our defense options in a seperate array with an O(1) index.
        pokeLinks_.push_back({-typeLookupIndex,
                              currentLinksIndex - previousSetSize,
                              currentLinksIndex + setSize,
                              Resistance::ZERO});

        for (const Resistance& singleType : type.second) {

            /* Important consideration for this algorithm. I am only interested in damage
             * resistances better than normal. So "covered" for a pokemon team means you found at
             * most 6 Pokemon that give you some level of resistance to all types in the game and
             * no pokemon on your team overlap by resisting the same types. You could have Pokemon
             * with x0.0, x0.25, or x0.5 resistances, but no higher.
             */

            if (singleType.multiplier < Resistance::ONE) {
                currentLinksIndex++;

                std::string sType = singleType.type;
                pokeLinks_[pokeLinks_[columnBuilder[sType]].down].topOrLen++;

                // A single item in a circular doubly linked list points to itself.
                pokeLinks_.push_back({pokeLinks_[columnBuilder[sType]].down,
                                      currentLinksIndex, currentLinksIndex,
                                      singleType.multiplier});

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
    pokeLinks_.push_back({INT_MIN,
                          currentLinksIndex + 1 - previousSetSize,
                          INT_MIN,
                          Resistance::ZERO});
}

void PokemonLinks::buildAttackLinks(const std::map<std::string,std::set<Resistance>>&
                                    typeInteractions) {
    (void) typeInteractions;
}


/* * * * * * * * * * * * * * * * *        Debugging Operators           * * * * * * * * * * * * * */


bool operator==(const PokemonLinks::pokeLink& lhs, const PokemonLinks::pokeLink& rhs) {
    return lhs.topOrLen == rhs.topOrLen
            && lhs.up == rhs.up && lhs.down == rhs.down
             && lhs.multiplier == rhs.multiplier;
}

bool operator!=(const PokemonLinks::pokeLink& lhs, const PokemonLinks::pokeLink& rhs) {
    return !(lhs == rhs);
}

bool operator==(const PokemonLinks::typeName& lhs, const PokemonLinks::typeName& rhs) {
    return lhs.name == rhs.name && lhs.left == rhs.left && lhs.right == rhs.right;
}

bool operator!=(const PokemonLinks::typeName& lhs, const PokemonLinks::typeName& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, const PokemonLinks::pokeLink& type) {
    os << "{ topOrLen: " << type.topOrLen
       << ", up: " << type.up << ", down: " << type.down
       << ", multiplier: ";
        if (type.multiplier == Resistance::ZERO) {
            os << "Resistance::ZERO";
        } else if (type.multiplier == Resistance::ONE_QUARTER) {
            os << "Resistance::ONE_QUARTER";
        } else if (type.multiplier == Resistance::ONE_HALF) {
            os << "Resistance::ONE_HALF";
        } else if (type.multiplier == Resistance::ONE) {
            os << "Resistance::ONE";
        } else if (type.multiplier == Resistance::TWO) {
            os << "Resistance::TWO";
        } else if (type.multiplier == Resistance::FOUR) {
            os << "Resistance::FOUR";
        }
    return os;
}

std::ostream& operator<<(std::ostream& os, const PokemonLinks::typeName& name) {
    os << "{ name: " << name.name << ", left: " << name.left << ", right: " << name.right << " }";
    return os;
}

std::ostream& operator<<(std::ostream&os, const std::vector<PokemonLinks::typeName>& items) {
    os << "LOOKUP TABLE" << std::endl;
    for (const auto& item : items) {
        os << "{\"" << item.name << "\"," << item.left << "," << item.right << "}" << std::endl;
    }
    os << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream&os, const std::vector<std::string>& options) {
    for (const auto& opt : options) {
        os << "{\"" << opt << "\"},";
    }
    os << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream&os, const std::vector<PokemonLinks::pokeLink>& links) {
    os << "DLX ARRAY" << std::endl;
    int index = 0;
    for (const auto& item : links) {
        if (item.topOrLen < 0) {
            os << std::endl;
        }
        os << "{" << item.topOrLen << "," << item.up << "," << item.down << ",";
        if (item.multiplier == Resistance::ZERO) {
            os << "Resistance::ZERO";
        } else if (item.multiplier == Resistance::ONE_QUARTER) {
            os << "Resistance::ONE_QUARTER";
        } else if (item.multiplier == Resistance::ONE_HALF) {
            os << "Resistance::ONE_HALF";
        } else if (item.multiplier == Resistance::ONE) {
            os << "Resistance::ONE";
        } else if (item.multiplier == Resistance::TWO) {
            os << "Resistance::TWO";
        } else if (item.multiplier == Resistance::FOUR) {
            os << "Resistance::FOUR";
        }
        os << "}";
        index++;
    }
    os << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream&os, const PokemonLinks& links) {
    os << "LOOKUP ARRAY" << std::endl;
    for (const auto& header : links.itemTable_) {
        os << "{\""
           << header.name << "\","
           << header.left << ","
           << header.right << "},"
           << std::endl;
    }
    os << "DLX ARRAY" << std::endl;
    int index = 0;
    for (const auto& item : links.pokeLinks_) {
        if (index >= links.itemTable_.size() && item.topOrLen < 0) {
            os << std::endl;
        }
        os << "{" << item.topOrLen << "," << item.up << "," << item.down << ",";
        if (item.multiplier == Resistance::ZERO) {
            os << "Resistance::ZERO";
        } else if (item.multiplier == Resistance::ONE_QUARTER) {
            os << "Resistance::ONE_QUARTER";
        } else if (item.multiplier == Resistance::ONE_HALF) {
            os << "Resistance::ONE_HALF";
        } else if (item.multiplier == Resistance::ONE) {
            os << "Resistance::ONE";
        } else if (item.multiplier == Resistance::TWO) {
            os << "Resistance::TWO";
        } else if (item.multiplier == Resistance::FOUR) {
            os << "Resistance::FOUR";
        }
        os << "},";
        index++;
    }
    os << std::endl;
    os << "Number of items: " << links.numItems_ << std::endl;
    os << "Number of options: " << links.numOptions_ << std::endl;
    return os;
}


/* * * * * * * * * * * * * * * * * *   Defense Links Tests  * * * * * * * * * * * * * * * * * * * */


STUDENT_TEST("Initialize small defensive links") {
    /*
     *
     *         Normal   Fire    Water   <-Attack
     *  Ghost   x0.0                    <-Defense
     *  Water           x0.5    x0.5
     *
     */
    const std::map<std::string,std::set<Resistance>> types {
        {"Ghost", {{"Fire",Resistance::ONE},{"Normal",Resistance::ZERO}, {"Water",Resistance::ONE}}},
        {"Water", {{"Fire",Resistance::ONE_HALF},{"Normal",Resistance::ONE}, {"Water",Resistance::ONE_HALF}}},
    };

    std::vector<std::string> optionTable = {"","Ghost","Water"};
    std::vector<PokemonLinks::typeName> itemTable = {
        {"",3,1},
        {"Fire",0,2},
        {"Normal",1,3},
        {"Water",2,0},
    };
    std::vector<PokemonLinks::pokeLink> dlx = {
        //     0                          1Fire                       2Normal                   3Water
        {0,0,0,Resistance::ZERO}, {1,7,7,Resistance::ZERO},    {1,5,5,Resistance::ZERO},{1,8,8,Resistance::ZERO},
        //     4Ghost                                                 5Zero
        {-1,0,7,Resistance::ZERO},                             {2,2,2,Resistance::ZERO},
        //     6Water                     7Half                                                 8Half
        {-2,3,9,Resistance::ZERO},{1,1,1,Resistance::ONE_HALF},                         {3,3,3,Resistance::ONE_HALF},
        //     9
        {INT_MIN,7,INT_MIN,Resistance::ZERO},
    };
    PokemonLinks links(types, PokemonLinks::DEFENSE);

    EXPECT_EQUAL(optionTable, links.optionTable_);
    EXPECT_EQUAL(itemTable, links.itemTable_);
    EXPECT_EQUAL(dlx, links.pokeLinks_);
}
