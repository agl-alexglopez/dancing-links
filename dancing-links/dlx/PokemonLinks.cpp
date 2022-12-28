#include "PokemonLinks.h"
#include <limits.h>
#include <cmath>



std::set<RankedSet<std::string>> PokemonLinks::getExactTypeCoverage() {
    std::set<RankedSet<std::string>> exactCoverages = {};
    RankedSet<std::string> coverage = {};
    int depthLimit = requestedCoverSolution_ == DEFENSE ? MAX_TEAM_SIZE : MAX_ATTACK_SLOTS;
    fillExactCoverages(exactCoverages, coverage, depthLimit);
    return exactCoverages;
}

std::set<RankedSet<std::string>> PokemonLinks::getOverlappingTypeCoverage() {
    std::set<RankedSet<std::string>> overlappingCoverages = {};
    RankedSet<std::string> coverage = {};
    int depthLimit = requestedCoverSolution_ == DEFENSE ? MAX_TEAM_SIZE : MAX_ATTACK_SLOTS;
    fillOverlappingCoverages(overlappingCoverages, coverage, depthLimit);
    return overlappingCoverages;
}

void PokemonLinks::fillExactCoverages(std::set<RankedSet<std::string>>& exactCoverages,
                                      RankedSet<std::string>& coverage,
                                      int depthLimit) {
    if (itemTable_[0].right == 0 && depthLimit >= 0) {
        exactCoverages.insert(coverage);
        return;
    }
    if (depthLimit <= 0) {
        return;
    }
    int attackType = chooseItem();
    if (attackType == -1) {
        return;
    }
    for (int cur = links_[attackType].down; cur != attackType; cur = links_[cur].down) {
        std::pair<int,std::string> typeStrength = coverType(cur);
        coverage.insert(typeStrength.first, typeStrength.second);

        fillExactCoverages(exactCoverages, coverage, depthLimit - 1);

        coverage.remove(typeStrength.first, typeStrength.second);
        uncoverType(cur);
    }
}

void PokemonLinks::fillOverlappingCoverages(std::set<RankedSet<std::string>>& overlappingCoverages,
                                            RankedSet<std::string>& coverage,
                                            int depthTag) {
    if (itemTable_[0].right == 0 && depthTag >= 0) {
        overlappingCoverages.insert(coverage);
        return;
    }
    if (depthTag <= 0) {
        return;
    }
    int attackType = chooseItem();
    if (attackType == -1) {
        return;
    }

    for (int cur = links_[attackType].down; cur != attackType; cur = links_[cur].down) {
        std::pair<int,std::string> typeStrength = looseCoverType(cur, depthTag);
        coverage.insert(typeStrength.first, typeStrength.second);


        fillOverlappingCoverages(overlappingCoverages, coverage, depthTag - 1);
        if (overlappingCoverages.size() == MAX_OUTPUT_SIZE) {
            coverage.remove(typeStrength.first, typeStrength.second);
            looseUncoverType(cur);
            return;
        }

        coverage.remove(typeStrength.first, typeStrength.second);
        looseUncoverType(cur);
    }

}


int PokemonLinks::chooseItem() const {
    int min = INT_MAX;
    int chosenIndex = 0;
    int head = 0;
    for (int cur = itemTable_[0].right; cur != head; cur = itemTable_[cur].right) {
        if (links_[cur].topOrLen <= 0) {
            return -1;
        }
        if (links_[cur].topOrLen < min) {
            chosenIndex = cur;
            min = links_[cur].topOrLen;
        }
    }
    return chosenIndex;
}

std::pair<int,std::string> PokemonLinks::coverType(int indexInOption) {
    std::pair<int,std::string> result = {};
    int i = indexInOption;
    do {
        int top = links_[i].topOrLen;
        // Pickup the current type defense option we have chosen.
        if (top <= 0) {
            i = links_[i].up;
            result.second = optionTable_[std::abs(links_[i - 1].topOrLen)];
        } else {
            typeName cur = itemTable_[top];
            itemTable_[cur.left].right = cur.right;
            itemTable_[cur.right].left = cur.left;
            hideOptions(i);
            result.first += links_[i++].multiplier;
        }
    } while (i != indexInOption);
    return result;
}

void PokemonLinks::uncoverType(int indexInOption) {
    int i = --indexInOption;
    do {
        int top = links_[i].topOrLen;
        // Pickup the current type defense option we have chosen.
        if (top <= 0) {
            i = links_[i].down;
        } else {
            typeName cur = itemTable_[top];
            itemTable_[cur.left].right = top;
            itemTable_[cur.right].left = top;
            unhideOptions(i--);
        }
    } while (i != indexInOption);
}

std::pair<int,std::string> PokemonLinks::looseCoverType(int indexInOption, int depthTag) {
    int i = indexInOption;
    std::pair<int, std::string> result = {};
    do {
        int top = links_[i].topOrLen;
        if (top <= 0) {
            i = links_[i].up;
            result.second = optionTable_[std::abs(links_[i - 1].topOrLen)];
        } else {
            if (!links_[top].depthTag) {
                links_[top].depthTag = depthTag;
                itemTable_[itemTable_[top].left].right = itemTable_[top].right;
                itemTable_[itemTable_[top].right].left = itemTable_[top].left;
                result.first += links_[i].multiplier;
            }
            links_[i++].depthTag = depthTag;
        }
    } while (i != indexInOption);

    return result;
}

void PokemonLinks::looseUncoverType(int indexInOption) {
    int i = --indexInOption;
    do {
        int top = links_[i].topOrLen;
        if (top < 0) {
            i = links_[i].down;
        } else {
            /* This is the key optimization of this algorithm. Only reset a city to uncovered if
             * we know we are taking away the same supply we gave when covering this city. A simple
             * O(1) check beats an up,down,left,right pointer implementation that needs to splice
             * from a left right doubly linked list for an entire column.
             */
            if (links_[top].depthTag == links_[i].depthTag) {
                links_[top].depthTag = 0;
                itemTable_[itemTable_[top].left].right = top;
                itemTable_[itemTable_[top].right].left = top;
            }
            links_[i--].depthTag = 0;
        }
    } while (i != indexInOption);
}


void PokemonLinks::hideOptions(int indexInOption) {
    for (int i = links_[indexInOption].down; i != indexInOption; i = links_[i].down) {
        if (i == links_[indexInOption].topOrLen) {
            continue;
        }
        for (int j = i + 1; j != i;) {
            int top = links_[j].topOrLen;
            if (top <= 0) {
                j = links_[j].up;
            } else {
                pokeLink cur = links_[j++];
                links_[cur.up].down = cur.down;
                links_[cur.down].up = cur.up;
                links_[top].topOrLen--;
            }
        }
    }
}

void PokemonLinks::unhideOptions(int indexInOption) {
    for (int i = links_[indexInOption].up; i != indexInOption; i = links_[i].up) {
        if (i == links_[indexInOption].topOrLen) {
            continue;
        }
        for (int j = i - 1; j != i;) {
            int top = links_[j].topOrLen;
            if (top <= 0) {
                j = links_[j].down;
            } else {
                pokeLink cur = links_[j];
                links_[cur.up].down = j;
                links_[cur.down].up = j;
                links_[top].topOrLen++;
                j--;
            }
        }
    }
}


/* * * * * * * * * * * * * * * * *        Debugging Operators           * * * * * * * * * * * * * */


PokemonLinks::PokemonLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions,
                           const CoverageType requestedCoverSolution) :
                           optionTable_({}),
                           itemTable_({}),
                           links_({}),
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
    // We always must gather all attack types available in this generation before we begin.
    requestedCoverSolution_ = DEFENSE;
    std::set<std::string> generationTypes = {};
    for (const Resistance& res : typeInteractions.begin()->second) {
        generationTypes.insert(res.type());
    }

    std::unordered_map<std::string,int> columnBuilder = {};
    optionTable_.push_back("");
    itemTable_.push_back({"", 0, 1});
    links_.push_back({0, 0, 0, Resistance::EMPTY_, 0});
    int index = 1;
    for (const std::string& type : generationTypes) {

        columnBuilder[type] = index;

        itemTable_.push_back({type, index - 1, index + 1});
        itemTable_[0].left++;

        links_.push_back({0, index, index, Resistance::EMPTY_,0});

        numItems_++;
        index++;
    }
    itemTable_[itemTable_.size() - 1].right = 0;

    initializeColumns(typeInteractions, columnBuilder, requestedCoverSolution_);
}

void PokemonLinks::initializeColumns(const std::map<std::string,std::set<Resistance>>&
                                     typeInteractions,
                                     std::unordered_map<std::string,int>& columnBuilder,
                                     CoverageType requestedCoverage) {
    int previousSetSize = links_.size();
    int currentLinksIndex = links_.size();
    int typeLookupIndex = 1;
    for (const auto& type : typeInteractions) {

        int typeTitle = currentLinksIndex;
        int setSize = 0;
        // We will lookup our defense options in a seperate array with an O(1) index.
        links_.push_back({-typeLookupIndex,
                          currentLinksIndex - previousSetSize,
                          currentLinksIndex,
                          Resistance::EMPTY_,
                          0});

        for (const Resistance& singleType : type.second) {

            /* Important consideration for this algorithm. I am only interested in damage
             * resistances better than normal. So "covered" for a pokemon team means you found at
             * most 6 Pokemon that give you some level of resistance to all types in the game and
             * no pokemon on your team overlap by resisting the same types. You could have Pokemon
             * with x0.0, x0.25, or x0.5 resistances, but no higher. Maybe we could lessen criteria?
             * Also, just flip this condition for the ATTACK version. We want damage better than
             * Normal, meaining x2 or x4.
             */

            if ((requestedCoverage == DEFENSE ? singleType.multiplier() < Resistance::NORMAL :
                                                Resistance::NORMAL < singleType.multiplier())) {
                currentLinksIndex++;
                links_[typeTitle].down++;
                setSize++;

                std::string sType = singleType.type();
                links_[links_[columnBuilder[sType]].down].topOrLen++;

                // A single item in a circular doubly linked list points to itself.
                links_.push_back({links_[columnBuilder[sType]].down,
                                  currentLinksIndex,
                                  currentLinksIndex,
                                  singleType.multiplier(),
                                  0});

                // This is the necessary adjustment to the column header's up field for a given item.
                links_[links_[columnBuilder[sType]].down].up = currentLinksIndex;
                // The current node is now the new tail in a vertical circular linked list for an item.
                links_[currentLinksIndex].up = columnBuilder[sType];
                links_[currentLinksIndex].down = links_[columnBuilder[sType]].down;
                // Update the old tail to reflect the new addition of an item in its option.
                links_[columnBuilder[sType]].down = currentLinksIndex;
                // Similar to a previous/current coding pattern but in an above/below column.
                columnBuilder[sType] = currentLinksIndex;
            }
        }

        optionTable_.push_back(type.first);
        typeLookupIndex++;
        currentLinksIndex++;
        numOptions_++;
        previousSetSize = setSize;
    }
    links_.push_back({INT_MIN,
                      currentLinksIndex - previousSetSize,
                      INT_MIN,
                      Resistance::EMPTY_,
                      0});
}

void PokemonLinks::buildAttackLinks(const std::map<std::string,std::set<Resistance>>&
                                    typeInteractions) {
    // We need two passes. The first pass will gather all types as options in the itemTable/headers
    requestedCoverSolution_ = ATTACK;
    optionTable_.push_back("");
    itemTable_.push_back({"", 0, 1});
    links_.push_back({0, 0, 0, Resistance::EMPTY_,0});
    int index = 1;
    std::map<std::string,std::set<Resistance>> invertedMap = {};
    std::unordered_map<std::string,int> columnBuilder = {};
    for (const auto& interaction : typeInteractions) {
        std::string defenseType = interaction.first;

        columnBuilder[defenseType] = index;
        itemTable_.push_back({defenseType, index - 1, index + 1});
        itemTable_[0].left++;

        links_.push_back({0, index, index, Resistance::EMPTY_,0});

        numItems_++;
        index++;

        for (const Resistance& attack : interaction.second) {
            invertedMap[attack.type()].insert({defenseType, attack.multiplier()});
        }
    }
    itemTable_[itemTable_.size() - 1].right = 0;
    // Second Pass iterate through this inverted map like you normally would. Use same helpers!
    initializeColumns(invertedMap, columnBuilder, requestedCoverSolution_);

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
       << ", multiplier: " << type.multiplier;
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
        os << "{" << item.topOrLen << ","
           << item.up << "," << item.down << "," << item.multiplier << "}";
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
    for (const auto& item : links.links_) {
        if (index >= links.itemTable_.size() && item.topOrLen < 0) {
            os << std::endl;
        }
        os << "{" << item.topOrLen << ","
           << item.up << "," << item.down << "," << item.multiplier << "},";
        index++;
    }
    os << std::endl;
    os << "Number of items: " << links.numItems_ << std::endl;
    os << "Number of options: " << links.numOptions_ << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::set<RankedSet<std::string>>& solution) {
    for (const auto& s : solution) {
        os << s;
    }
    os << std::endl;
    return os;
}


/* * * * * * * * * * * * * * * * * *   Defense Links Init   * * * * * * * * * * * * * * * * * * * */


STUDENT_TEST("Initialize small defensive links") {
    /*
     *
     *          Fire   Normal    Water   <-Attack
     *  Ghost          x0.0              <-Defense
     *  Water   x0.5             x0.5
     *
     */
    const std::map<std::string,std::set<Resistance>> types {
        {"Ghost", {{"Fire",Resistance::NORMAL},{"Normal",Resistance::IMMUNE},{"Water",Resistance::NORMAL}}},
        {"Water", {{"Fire",Resistance::FRAC12},{"Normal",Resistance::NORMAL},{"Water",Resistance::FRAC12}}},
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
        {0,0,0,Resistance::EMPTY_,0}, {1,7,7,Resistance::EMPTY_,0},{1,5,5,Resistance::EMPTY_,0},{1,8,8,Resistance::EMPTY_,0},
        //     4Ghost                                                 5Zero
        {-1,0,5,Resistance::EMPTY_,0},                             {2,2,2,Resistance::IMMUNE,0},
        //     6Water                     7Half                                                 8Half
        {-2,5,8,Resistance::EMPTY_,0},{1,1,1,Resistance::FRAC12,0},                            {3,3,3,Resistance::FRAC12,0},
        //     9
        {INT_MIN,7,INT_MIN,Resistance::EMPTY_,0} ,
    };
    PokemonLinks links(types, PokemonLinks::DEFENSE);
    EXPECT_EQUAL(optionTable, links.optionTable_);
    EXPECT_EQUAL(itemTable, links.itemTable_);
    EXPECT_EQUAL(dlx, links.links_);
}

STUDENT_TEST("Initialize a world where there are only single types.") {
    /*
     *
     *            Electric  Fire  Grass  Ice   Normal  Water
     *  Dragon     x0.5     x0.5  x0.5                 x0.5
     *  Electric   x0.5
     *  Ghost                                  x0.0
     *  Ice                              x0.5
     *
     */
    const std::map<std::string,std::set<Resistance>> types {
        {"Dragon", {{"Normal",Resistance::NORMAL},{"Fire",Resistance::FRAC12},{"Water",Resistance::FRAC12},{"Electric",Resistance::FRAC12},{"Grass",Resistance::FRAC12},{"Ice",Resistance::DOUBLE}}},
        {"Electric", {{"Normal",Resistance::NORMAL},{"Fire",Resistance::NORMAL},{"Water",Resistance::NORMAL},{"Electric",Resistance::FRAC12},{"Grass",Resistance::NORMAL},{"Ice",Resistance::NORMAL}}},
        {"Ghost", {{"Normal",Resistance::IMMUNE},{"Fire",Resistance::NORMAL},{"Water",Resistance::NORMAL},{"Electric",Resistance::NORMAL},{"Grass",Resistance::NORMAL},{"Ice",Resistance::NORMAL}}},
        {"Ice", {{"Normal",Resistance::NORMAL},{"Fire",Resistance::NORMAL},{"Water",Resistance::NORMAL},{"Electric",Resistance::NORMAL},{"Grass",Resistance::NORMAL},{"Ice",Resistance::FRAC12}}},
    };

    std::vector<std::string> optionTable = {"","Dragon","Electric","Ghost","Ice"};
    std::vector<PokemonLinks::typeName> itemTable = {
        {"",6,1},
        {"Electric",0,2},
        {"Fire",1,3},
        {"Grass",2,4},
        {"Ice",3,5},
        {"Normal",4,6},
        {"Water",5,0},
    };
    std::vector<PokemonLinks::pokeLink> dlx = {
        //       0                             1Electric                  2Fire                     3Grass                        4Ice                          5Normal                      6Water
        {0,0,0,Resistance::EMPTY_,0},   {2,13,8,Resistance::EMPTY_,0},{1,9,9,Resistance::EMPTY_,0},{1,10,10,Resistance::EMPTY_,0},{1,17,17,Resistance::EMPTY_,0},{1,15,15,Resistance::EMPTY_,0},{1,11,11,Resistance::EMPTY_,0},
        //       7Dragon                       8half                      9half                     10half                                                                                   11half
        {-1,0,11,Resistance::EMPTY_,0}, {1,1,13,Resistance::FRAC12,0},{2,2,2,Resistance::FRAC12,0},{3,3,3,Resistance::FRAC12,0},                                                            {6,6,6,Resistance::FRAC12,0},
        //       12Electric                    13half
        {-2,8,13,Resistance::EMPTY_,0}, {1,8,1,Resistance::FRAC12,0},
        //       14Ghost                                                                                                                                        15immune
        {-3,13,15,Resistance::EMPTY_,0},                                                                                                                  {5,5,5,Resistance::IMMUNE,0},
        //       16Ice                                                                                                            17half
        {-4,15,17,Resistance::EMPTY_,0},                                                                                    {4,4,4,Resistance::FRAC12,0},
        //       18
        {INT_MIN,17,INT_MIN,Resistance::EMPTY_,0},
    };
    PokemonLinks links(types, PokemonLinks::DEFENSE);
    EXPECT_EQUAL(optionTable, links.optionTable_);
    EXPECT_EQUAL(itemTable, links.itemTable_);
    EXPECT_EQUAL(dlx, links.links_);
}


/* * * * * * * * * * * * * * * *   Defense Links Cover/Uncover      * * * * * * * * * * * * * * * */


STUDENT_TEST("Cover Electric with Dragon eliminates Electric Option. Uncover resets.") {
    /*
     *
     *            Electric  Fire  Grass  Ice   Normal  Water
     *  Dragon     x0.5     x0.5  x0.5                 x0.5
     *  Electric   x0.5
     *  Ghost                                  x0.0
     *  Ice                              x0.5
     *
     */
    const std::map<std::string,std::set<Resistance>> types {
        {"Dragon", {{"Normal",Resistance::NORMAL},{"Fire",Resistance::FRAC12},{"Water",Resistance::FRAC12},{"Electric",Resistance::FRAC12},{"Grass",Resistance::FRAC12},{"Ice",Resistance::DOUBLE}}},
        {"Electric", {{"Normal",Resistance::NORMAL},{"Fire",Resistance::NORMAL},{"Water",Resistance::NORMAL},{"Electric",Resistance::FRAC12},{"Grass",Resistance::NORMAL},{"Ice",Resistance::NORMAL}}},
        {"Ghost", {{"Normal",Resistance::IMMUNE},{"Fire",Resistance::NORMAL},{"Water",Resistance::NORMAL},{"Electric",Resistance::NORMAL},{"Grass",Resistance::NORMAL},{"Ice",Resistance::NORMAL}}},
        {"Ice", {{"Normal",Resistance::NORMAL},{"Fire",Resistance::NORMAL},{"Water",Resistance::NORMAL},{"Electric",Resistance::NORMAL},{"Grass",Resistance::NORMAL},{"Ice",Resistance::FRAC12}}},
    };

    std::vector<std::string> optionTable = {"","Dragon","Electric","Ghost","Ice"};
    std::vector<PokemonLinks::typeName> itemTable = {
        {"",6,1},
        {"Electric",0,2},
        {"Fire",1,3},
        {"Grass",2,4},
        {"Ice",3,5},
        {"Normal",4,6},
        {"Water",5,0},
    };
    std::vector<PokemonLinks::pokeLink> dlx = {
        //       0                             1Electric                  2Fire                     3Grass                        4Ice                          5Normal                      6Water
        {0,0,0,Resistance::EMPTY_,0},   {2,13,8,Resistance::EMPTY_,0},{1,9,9,Resistance::EMPTY_,0},{1,10,10,Resistance::EMPTY_,0},{1,17,17,Resistance::EMPTY_,0},{1,15,15,Resistance::EMPTY_,0},{1,11,11,Resistance::EMPTY_,0},
        //       7Dragon                       8half                      9half                     10half                                                                                   11half
        {-1,0,11,Resistance::EMPTY_,0}, {1,1,13,Resistance::FRAC12,0},{2,2,2,Resistance::FRAC12,0},{3,3,3,Resistance::FRAC12,0},                                                            {6,6,6,Resistance::FRAC12,0},
        //       12Electric                    13half
        {-2,8,13,Resistance::EMPTY_,0}, {1,8,1,Resistance::FRAC12,0},
        //       14Ghost                                                                                                                                        15immune
        {-3,13,15,Resistance::EMPTY_,0},                                                                                                                  {5,5,5,Resistance::IMMUNE,0},
        //       16Ice                                                                                                            17half
        {-4,15,17,Resistance::EMPTY_,0},                                                                                    {4,4,4,Resistance::FRAC12,0},
        //       18
        {INT_MIN,17,INT_MIN,Resistance::EMPTY_,0},
    };
    PokemonLinks links(types, PokemonLinks::DEFENSE);
    EXPECT_EQUAL(optionTable, links.optionTable_);
    EXPECT_EQUAL(itemTable, links.itemTable_);
    EXPECT_EQUAL(dlx, links.links_);

    std::vector<PokemonLinks::typeName> itemCoverElectric = {
        {"",5,4},
        {"Electric",0,2},
        {"Fire",0,3},
        {"Grass",0,4},
        {"Ice",0,5},
        {"Normal",4,0},
        {"Water",5,0},
    };
    /*
     *             Ice   Normal
     *  Ghost             x0.0
     *  Ice        x0.5
     *
     */
    std::vector<PokemonLinks::pokeLink> dlxCoverElectric = {
        //       0                             1Electric                  2Fire                     3Grass                        4Ice                          5Normal                      6Water
        {0,0,0,Resistance::EMPTY_,0},   {2,13,8,Resistance::EMPTY_,0},{1,9,9,Resistance::EMPTY_,0},{1,10,10,Resistance::EMPTY_,0},{1,17,17,Resistance::EMPTY_,0},{1,15,15,Resistance::EMPTY_,0},{1,11,11,Resistance::EMPTY_,0},
        //       7Dragon                       8half                      9half                     10half                                                                                   11half
        {-1,0,11,Resistance::EMPTY_,0}, {1,1,13,Resistance::FRAC12,0},{2,2,2,Resistance::FRAC12,0},{3,3,3,Resistance::FRAC12,0},                                                            {6,6,6,Resistance::FRAC12,0},
        //       12Electric                    13half
        {-2,8,13,Resistance::EMPTY_,0}, {1,8,1,Resistance::FRAC12,0},
        //       14Ghost                                                                                                                                        15immune
        {-3,13,15,Resistance::EMPTY_,0},                                                                                                                  {5,5,5,Resistance::IMMUNE,0},
        //       16Ice                                                                                                            17half
        {-4,15,17,Resistance::EMPTY_,0},                                                                                    {4,4,4,Resistance::FRAC12,0},
        //       18
        {INT_MIN,17,INT_MIN,Resistance::EMPTY_,0},
    };

    std::pair<int,std::string> pick = links.coverType(8);
    EXPECT_EQUAL(pick.first,12);
    EXPECT_EQUAL(pick.second,"Dragon");
    EXPECT_EQUAL(itemCoverElectric, links.itemTable_);
    EXPECT_EQUAL(dlxCoverElectric, links.links_);

    links.uncoverType(8);
    EXPECT_EQUAL(itemTable, links.itemTable_);
    EXPECT_EQUAL(dlx, links.links_);
}

STUDENT_TEST("Cover Electric with Electric to cause hiding of many options.") {
    /*
     * This is just nonsense type weakness information in pairs to I can test the cover logic.
     *
     *            Electric  Fire  Grass  Ice   Normal  Water
     *  Electric   x0.5     x0.5
     *  Fire       x0.5           x0.5                 x0.5
     *  Grass               x0.5                       x0.5
     *  Ice                              x0.5          x0.5
     *  Normal     x0.5                        x0.5
     *  Water              x0.5                        x0.5
     *
     */
    const std::map<std::string,std::set<Resistance>> types {
        {"Electric", {{"Electric",Resistance::FRAC12},{"Fire",Resistance::FRAC12},{"Grass",Resistance::NORMAL},{"Ice",Resistance::NORMAL},{"Normal",Resistance::NORMAL},{"Water",Resistance::NORMAL}}},
        {"Fire", {{"Electric",Resistance::FRAC12},{"Fire",Resistance::NORMAL},{"Grass",Resistance::FRAC12},{"Ice",Resistance::NORMAL},{"Normal",Resistance::NORMAL},{"Water",Resistance::FRAC12}}},
        {"Grass", {{"Electric",Resistance::NORMAL},{"Fire",Resistance::FRAC12},{"Grass",Resistance::NORMAL},{"Ice",Resistance::NORMAL},{"Normal",Resistance::NORMAL},{"Water",Resistance::FRAC12}}},
        {"Ice", {{"Electric",Resistance::NORMAL},{"Fire",Resistance::NORMAL},{"Grass",Resistance::NORMAL},{"Ice",Resistance::FRAC12},{"Normal",Resistance::NORMAL},{"Water",Resistance::FRAC12}}},
        {"Normal", {{"Electric",Resistance::FRAC12},{"Fire",Resistance::NORMAL},{"Grass",Resistance::NORMAL},{"Ice",Resistance::NORMAL},{"Normal",Resistance::FRAC12},{"Water",Resistance::NORMAL}}},
        {"Water", {{"Electric",Resistance::NORMAL},{"Fire",Resistance::FRAC12},{"Grass",Resistance::NORMAL},{"Ice",Resistance::NORMAL},{"Normal",Resistance::NORMAL},{"Water",Resistance::FRAC12}}},
    };
    PokemonLinks links (types, PokemonLinks::DEFENSE);
    std::vector<PokemonLinks::typeName> headers = {
        {"",6,1},
        {"Electric",0,2},
        {"Fire",1,3},
        {"Grass",2,4},
        {"Ice",3,5},
        {"Normal",4,6},
        {"Water",5,0},
    };
    std::vector<PokemonLinks::pokeLink> dlx = {
        //         0                           1Electric                       2Fire                        3Grass                      4Ice                          5Normal                      6Water
        {0,0,0,Resistance::EMPTY_,0},   {3,21,8,Resistance::EMPTY_,0},{3,24,9,Resistance::EMPTY_,0}, {1,12,12,Resistance::EMPTY_,0},{1,18,18,Resistance::EMPTY_,0},{1,22,22,Resistance::EMPTY_,0},{4,25,13,Resistance::EMPTY_,0},
        //         7Electric                   8                          9
        {-1,0,9,Resistance::EMPTY_,0},  {1,1,11,Resistance::FRAC12,0},{2,2,15,Resistance::FRAC12,0},
        //         10Fire                      11                                                       12                                                                                           13
        {-2,8,13,Resistance::EMPTY_,0}, {1,8,21,Resistance::FRAC12,0},                               {3,3,3,Resistance::FRAC12,0},                                                              {6,6,16,Resistance::FRAC12,0},
        //         14Grass                                                15                                                                                                                         16
        {-3,11,16,Resistance::EMPTY_,0},                            {2,9,24,Resistance::FRAC12,0},                                                                                              {6,13,19,Resistance::FRAC12,0},
        //         17Ice                                                                                                                  18                                                         19
        {-4,15,19,Resistance::EMPTY_,0},                                                                                             {4,4,4,Resistance::FRAC12,0},                              {6,16,25,Resistance::FRAC12,0},
        //         20Normal                    21                                                                                                                       22
        {-5,18,22,Resistance::EMPTY_,0},{1,11,1,Resistance::FRAC12,0},                                                                                             {5,5,5,Resistance::FRAC12,0},
        //         23Water                                                24                                                                                                                         25
        {-6,21,25,Resistance::EMPTY_,0},                            {2,15,2,Resistance::FRAC12,0},                                                                                              {6,19,6,Resistance::FRAC12,0},
        {INT_MIN,24,INT_MIN,Resistance::EMPTY_,0},
    };
    EXPECT_EQUAL(headers, links.itemTable_);
    EXPECT_EQUAL(dlx, links.links_);

    std::vector<PokemonLinks::typeName> headersCoverElectric = {
        {"",6,3},
        {"Electric",0,2},
        {"Fire",0,3},
        {"Grass",0,4},
        {"Ice",3,5},
        {"Normal",4,6},
        {"Water",5,0},
    };
    std::vector<PokemonLinks::pokeLink> dlxCoverElectric = {
        /*
         *
         *        Grass   Ice    Normal  Water
         *  Ice           x0.5           x0.5
         *
         *
         */
        //         0                           1Electric                      2Fire                        3Grass                      4Ice                          5Normal                          6Water
        {0,0,0,Resistance::EMPTY_,0},   {3,21,8,Resistance::EMPTY_,0},{3,24,9,Resistance::EMPTY_,0}, {0,3,3,Resistance::EMPTY_,0},{1,18,18,Resistance::EMPTY_,0},{0,5,5,Resistance::EMPTY_,0},  {1,19,19,Resistance::EMPTY_,0},
        //         7Electric                   8                          9
        {-1,0,9,Resistance::EMPTY_,0},  {1,1,11,Resistance::FRAC12,0},{2,2,15,Resistance::FRAC12,0},
        //         10Fire                      11                                                       12                                                                                             13
        {-2,8,13,Resistance::EMPTY_,0}, {1,8,21,Resistance::FRAC12,0},                             {3,3,3,Resistance::FRAC12,0},                                                                {6,6,16,Resistance::FRAC12,0},
        //         14Grass                                                15                                                                                                                           16
        {-3,11,16,Resistance::EMPTY_,0},                              {2,9,24,Resistance::FRAC12,0},                                                                                            {6,6,19,Resistance::FRAC12,0},
        //         17Ice                                                                                                           18                                                                  19
        {-4,15,19,Resistance::EMPTY_,0},                                                                                      {4,4,4,Resistance::FRAC12,0},                                     {6,6,6,Resistance::FRAC12,0},
        //         20Normal                    21                                                                                                                22
        {-5,18,22,Resistance::EMPTY_,0},{1,11,1,Resistance::FRAC12,0},                                                                                       {5,5,5,Resistance::FRAC12,0},
        //         23Water                                                24                                                                                                                           25
        {-6,21,25,Resistance::EMPTY_,0},                              {2,15,2,Resistance::FRAC12,0},                                                                                            {6,19,6,Resistance::FRAC12,0},
        {INT_MIN,24,INT_MIN,Resistance::EMPTY_,0},
    };

    std::pair<int,std::string> pick = links.coverType(8);
    EXPECT_EQUAL(pick.first,6);
    EXPECT_EQUAL(pick.second,"Electric");
    EXPECT_EQUAL(headersCoverElectric, links.itemTable_);
    EXPECT_EQUAL(dlxCoverElectric, links.links_);

    links.uncoverType(8);
    EXPECT_EQUAL(headers, links.itemTable_);
    EXPECT_EQUAL(dlx, links.links_);
}


/* * * * * * * * * * * * *      Solve the Defensive Cover Problem       * * * * * * * * * * * * * */


STUDENT_TEST("There are two exact covers for this typing combo.") {
    /*
     *              Electric   Grass   Ice   Normal   Water
     *   Electric    x0.5
     *   Ghost                               x0.0
     *   Ground      x0.0
     *   Ice                           x0.5
     *   Poison                x0.5
     *   Water                         x0.5           x0.5
     *
     *   Exact Defensive Type Covers. 1 is better because Ground is immune to electric.
     *      1. Ghost, Ground, Poison, Water
     *      2. Electric, Ghost, Poison, Water
     *
     */
    const std::map<std::string,std::set<Resistance>> types {
        {"Electric", {{"Electric",Resistance::FRAC12},{"Grass",Resistance::NORMAL},{"Ice",Resistance::NORMAL},{"Normal",Resistance::NORMAL},{"Water",Resistance::NORMAL}}},
        {"Ghost", {{"Electric",Resistance::NORMAL},{"Grass",Resistance::NORMAL},{"Ice",Resistance::NORMAL},{"Normal",Resistance::IMMUNE},{"Water",Resistance::NORMAL}}},
        {"Ground", {{"Electric",Resistance::IMMUNE},{"Grass",Resistance::NORMAL},{"Ice",Resistance::NORMAL},{"Normal",Resistance::NORMAL},{"Water",Resistance::NORMAL}}},
        {"Ice", {{"Electric",Resistance::NORMAL},{"Grass",Resistance::NORMAL},{"Ice",Resistance::FRAC12},{"Normal",Resistance::NORMAL},{"Water",Resistance::NORMAL}}},
        {"Poison", {{"Electric",Resistance::NORMAL},{"Grass",Resistance::FRAC12},{"Ice",Resistance::NORMAL},{"Normal",Resistance::NORMAL},{"Water",Resistance::NORMAL}}},
        {"Water", {{"Electric",Resistance::NORMAL},{"Grass",Resistance::DOUBLE},{"Ice",Resistance::FRAC12},{"Normal",Resistance::NORMAL},{"Water",Resistance::FRAC12}}},
    };
    PokemonLinks links(types, PokemonLinks::DEFENSE);
    std::set<RankedSet<std::string>> correct = {{11,{"Ghost","Ground","Poison","Water"}}, {13,{"Electric","Ghost","Poison","Water"}}};
    EXPECT_EQUAL(links.getExactTypeCoverage(), correct);
}


/* * * * * * * * * * * * * * * * * *   Attack Links Init    * * * * * * * * * * * * * * * * * * * */

/* The good news about this section is that we only have to test that we can correctly initialize
 * the network by inverting the attack types and defense types. Then, the algorithm runs
 * identically and we can use the same functions for this problem.
 */

STUDENT_TEST("Initialization of ATTACK dancing links.") {
    /*
     *
     *                    Fire-Flying   Ground-Grass   Ground-Rock   <-Defense
     *         Electric       2X
     *         Fire                         2x
     *         Water          2x                         4x          <-Attack
     *
     */
    const std::map<std::string,std::set<Resistance>> types {
        {"Ground-Rock", {{"Electric",Resistance::IMMUNE},{"Fire",Resistance::NORMAL},{"Water",Resistance::QUADRU}}},
        {"Ground-Grass", {{"Electric",Resistance::IMMUNE},{"Fire",Resistance::DOUBLE},{"Water",Resistance::NORMAL}}},
        {"Fire-Flying", {{"Electric",Resistance::DOUBLE},{"Fire",Resistance::FRAC12},{"Water",Resistance::DOUBLE}}},
    };
    const std::vector<std::string> optionTable = {"","Electric","Fire","Water"};
    const std::vector<PokemonLinks::typeName> itemTable = {
        {"",3,1},
        {"Fire-Flying",0,2},
        {"Ground-Grass",1,3},
        {"Ground-Rock",2,0},
    };
    const std::vector<PokemonLinks::pokeLink> dlx {
        //       0                       1Fire-Flying                 2Ground-Grass              3Ground-Rock
        {0,0,0,Resistance::EMPTY_,0},  {2,9,5,Resistance::EMPTY_,0},{1,7,7,Resistance::EMPTY_,0},{1,10,10,Resistance::EMPTY_,0},
        //       4Electric               5Double
        {-1,0,5,Resistance::EMPTY_,0}, {1,1,9,Resistance::DOUBLE,0},
        //       6Fire                                                7Double
        {-2,5,7,Resistance::EMPTY_,0},                                {2,2,2,Resistance::DOUBLE,0},
        //       8Water                  9Double                                                 10Quadru
        {-3,7,10,Resistance::EMPTY_,0},{1,5,1,Resistance::DOUBLE,0},                           {3,3,3,Resistance::QUADRU,0},
        {INT_MIN,9,INT_MIN,Resistance::EMPTY_,0},
    };
    PokemonLinks links(types, PokemonLinks::ATTACK);
    EXPECT_EQUAL(links.optionTable_, optionTable);
    EXPECT_EQUAL(links.itemTable_, itemTable);
    EXPECT_EQUAL(links.links_, dlx);
}

STUDENT_TEST("At least test that we can recognize a successful attack coverage") {
    /*
     *
     *               Normal   Fire   Water   Electric   Grass   Ice     <- Defensive Types
     *    Fighting     x2                                        x2
     *    Grass                       x2                                <- Attack Types
     *    Ground               x2               x2
     *    Ice                                            x2
     *    Poison                                         x2
     *
     * There are two attack coverage schemes:
     *      Fighting, Grass, Ground, Ice
     *      Fighting, Grass, Ground, Poison
     */
    const std::map<std::string,std::set<Resistance>> types = {
        {"Electric", {{"Ground",Resistance::DOUBLE}}},
        {"Fire", {{"Ground",Resistance::DOUBLE}}},
        {"Grass", {{"Ice",Resistance::DOUBLE},{"Poison",Resistance::DOUBLE}}},
        {"Ice", {{"Fighting",Resistance::DOUBLE}}},
        {"Normal", {{"Fighting",Resistance::DOUBLE}}},
        {"Water", {{"Grass",Resistance::DOUBLE}}},
    };
    std::set<RankedSet<std::string>> solutions = {{30, {"Fighting","Grass","Ground","Ice"}},
                                                  {30,{"Fighting","Grass","Ground","Poison"}}};
    PokemonLinks links(types, PokemonLinks::ATTACK);
    EXPECT_EQUAL(links.getExactTypeCoverage(), solutions);
}


/* * * * * * * * * * *    Finding a Weak Coverage that Allows Overlap     * * * * * * * * * * * * */


STUDENT_TEST("Cover Electric with Electric to cause hiding of many options.") {
    /*
     * This is just nonsense type weakness information in pairs to I can test the cover logic.
     *
     *            Electric  Fire  Grass  Ice   Normal  Water
     *  Electric   x0.5     x0.5
     *  Fire       x0.5           x0.5                 x0.5
     *  Grass               x0.5                       x0.5
     *  Ice                              x0.5          x0.5
     *  Normal     x0.5                        x0.5
     *  Water              x0.5                        x0.5
     *
     */
    const std::map<std::string,std::set<Resistance>> types {
        {"Electric", {{"Electric",Resistance::FRAC12},{"Fire",Resistance::FRAC12},{"Grass",Resistance::NORMAL},{"Ice",Resistance::NORMAL},{"Normal",Resistance::NORMAL},{"Water",Resistance::NORMAL}}},
        {"Fire", {{"Electric",Resistance::FRAC12},{"Fire",Resistance::NORMAL},{"Grass",Resistance::FRAC12},{"Ice",Resistance::NORMAL},{"Normal",Resistance::NORMAL},{"Water",Resistance::FRAC12}}},
        {"Grass", {{"Electric",Resistance::NORMAL},{"Fire",Resistance::FRAC12},{"Grass",Resistance::NORMAL},{"Ice",Resistance::NORMAL},{"Normal",Resistance::NORMAL},{"Water",Resistance::FRAC12}}},
        {"Ice", {{"Electric",Resistance::NORMAL},{"Fire",Resistance::NORMAL},{"Grass",Resistance::NORMAL},{"Ice",Resistance::FRAC12},{"Normal",Resistance::NORMAL},{"Water",Resistance::FRAC12}}},
        {"Normal", {{"Electric",Resistance::FRAC12},{"Fire",Resistance::NORMAL},{"Grass",Resistance::NORMAL},{"Ice",Resistance::NORMAL},{"Normal",Resistance::FRAC12},{"Water",Resistance::NORMAL}}},
        {"Water", {{"Electric",Resistance::NORMAL},{"Fire",Resistance::FRAC12},{"Grass",Resistance::NORMAL},{"Ice",Resistance::NORMAL},{"Normal",Resistance::NORMAL},{"Water",Resistance::FRAC12}}},
    };
    PokemonLinks links (types, PokemonLinks::DEFENSE);
    std::set<RankedSet<std::string>> result = links.getOverlappingTypeCoverage();
    std::cout << result << std::endl;
}
