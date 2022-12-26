#include "PokemonLinks.h"
#include <limits.h>
#include <cmath>



std::priority_queue<RankedCover> PokemonLinks::getAllCoveredTeams() {
    if (requestedCoverSolution_ == ATTACK) {
        error("Requested ATTACK solution on DEFENSE links. Instantiate ATTACK links to proceed.");
    }
    std::priority_queue<RankedCover> exactCoverages = {};
    RankedCover coverage = {};
    fillCoverages(exactCoverages, coverage, teamSize_);
    return exactCoverages;
}

void PokemonLinks::fillCoverages(std::priority_queue<RankedCover>& exactCoverages,
                                 RankedCover& coverage,
                                 int teamPicks) {
    if (itemTable_[0].right == 0 && teamPicks >= 0) {
        exactCoverages.push(coverage);
        return;
    }
    if (teamPicks <= 0) {
        return;
    }
    int attackType = chooseItem();
    if (attackType == -1) {
        return;
    }
    for (int cur = links_[attackType].down; cur != attackType; cur = links_[cur].down) {
        std::pair<int,std::string> typeStrength = coverAttackType(cur);
        coverage.add(typeStrength.first);
        coverage.insert(typeStrength.second);

        fillCoverages(exactCoverages, coverage, teamPicks - 1);

        coverage.subtract(typeStrength.first);
        coverage.remove(typeStrength.second);
        uncoverAttackType(cur);
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

std::pair<int,std::string> PokemonLinks::coverAttackType(int indexInOption) {
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
            result.first -= links_[i++].multiplier;
        }
    } while (i != indexInOption);
    return result;
}

void PokemonLinks::uncoverAttackType(int indexInOption) {
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
                           teamSize_(MAX_TEAM_SIZE),
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
    std::set<std::string> generationTypes = {};
    for (const Resistance& res : typeInteractions.begin()->second) {
        generationTypes.insert(res.type());
    }

    std::unordered_map<std::string,int> columnBuilder = {};
    optionTable_.push_back("");
    itemTable_.push_back({"", 0, 1});
    links_.push_back({0, 0, 0, Resistance::EMPTY_});
    int index = 1;
    for (const std::string& type : generationTypes) {

        columnBuilder[type] = index;

        itemTable_.push_back({type, index - 1, index + 1});
        itemTable_[0].left++;

        links_.push_back({0, index, index, Resistance::EMPTY_});

        numItems_++;
        index++;
    }
    itemTable_[itemTable_.size() - 1].right = 0;

    initializeColumns(typeInteractions, columnBuilder);
}

void PokemonLinks::initializeColumns(const std::map<std::string,std::set<Resistance>>&
                                     typeInteractions,
                                     std::unordered_map<std::string,int>& columnBuilder) {
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
                          Resistance::EMPTY_});

        for (const Resistance& singleType : type.second) {

            /* Important consideration for this algorithm. I am only interested in damage
             * resistances better than normal. So "covered" for a pokemon team means you found at
             * most 6 Pokemon that give you some level of resistance to all types in the game and
             * no pokemon on your team overlap by resisting the same types. You could have Pokemon
             * with x0.0, x0.25, or x0.5 resistances, but no higher. Maybe we could lessen criteria?
             */

            if (singleType.multiplier() < Resistance::NORMAL) {
                currentLinksIndex++;
                links_[typeTitle].down++;
                setSize++;

                std::string sType = singleType.type();
                links_[links_[columnBuilder[sType]].down].topOrLen++;

                // A single item in a circular doubly linked list points to itself.
                links_.push_back({links_[columnBuilder[sType]].down,
                                  currentLinksIndex,
                                  currentLinksIndex,
                                  singleType.multiplier()});

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
                      Resistance::EMPTY_});
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
        {0,0,0,Resistance::EMPTY_}, {1,7,7,Resistance::EMPTY_},{1,5,5,Resistance::EMPTY_},{1,8,8,Resistance::EMPTY_},
        //     4Ghost                                                 5Zero
        {-1,0,5,Resistance::EMPTY_},                           {2,2,2,Resistance::IMMUNE},
        //     6Water                     7Half                                                 8Half
        {-2,5,8,Resistance::EMPTY_},{1,1,1,Resistance::FRAC12},                           {3,3,3,Resistance::FRAC12},
        //     9
        {INT_MIN,7,INT_MIN,Resistance::EMPTY_} ,
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
        {0,0,0,Resistance::EMPTY_},   {2,13,8,Resistance::EMPTY_},{1,9,9,Resistance::EMPTY_},{1,10,10,Resistance::EMPTY_},{1,17,17,Resistance::EMPTY_},{1,15,15,Resistance::EMPTY_},{1,11,11,Resistance::EMPTY_},
        //       7Dragon                       8half                      9half                     10half                                                                                   11half
        {-1,0,11,Resistance::EMPTY_}, {1,1,13,Resistance::FRAC12},{2,2,2,Resistance::FRAC12},{3,3,3,Resistance::FRAC12},                                                            {6,6,6,Resistance::FRAC12},
        //       12Electric                    13half
        {-2,8,13,Resistance::EMPTY_}, {1,8,1,Resistance::FRAC12},
        //       14Ghost                                                                                                                                        15immune
        {-3,13,15,Resistance::EMPTY_},                                                                                                                  {5,5,5,Resistance::IMMUNE},
        //       16Ice                                                                                                            17half
        {-4,15,17,Resistance::EMPTY_},                                                                                    {4,4,4,Resistance::FRAC12},
        //       18
        {INT_MIN,17,INT_MIN,Resistance::EMPTY_},
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
        {0,0,0,Resistance::EMPTY_},   {2,13,8,Resistance::EMPTY_},{1,9,9,Resistance::EMPTY_},{1,10,10,Resistance::EMPTY_},{1,17,17,Resistance::EMPTY_},{1,15,15,Resistance::EMPTY_},{1,11,11,Resistance::EMPTY_},
        //       7Dragon                       8half                      9half                     10half                                                                                   11half
        {-1,0,11,Resistance::EMPTY_}, {1,1,13,Resistance::FRAC12},{2,2,2,Resistance::FRAC12},{3,3,3,Resistance::FRAC12},                                                            {6,6,6,Resistance::FRAC12},
        //       12Electric                    13half
        {-2,8,13,Resistance::EMPTY_}, {1,8,1,Resistance::FRAC12},
        //       14Ghost                                                                                                                                        15immune
        {-3,13,15,Resistance::EMPTY_},                                                                                                                  {5,5,5,Resistance::IMMUNE},
        //       16Ice                                                                                                            17half
        {-4,15,17,Resistance::EMPTY_},                                                                                    {4,4,4,Resistance::FRAC12},
        //       18
        {INT_MIN,17,INT_MIN,Resistance::EMPTY_},
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
        {0,0,0,Resistance::EMPTY_},   {2,13,8,Resistance::EMPTY_},{1,9,9,Resistance::EMPTY_},{1,10,10,Resistance::EMPTY_},{1,17,17,Resistance::EMPTY_},{1,15,15,Resistance::EMPTY_},{1,11,11,Resistance::EMPTY_},
        //       7Dragon                       8half                      9half                     10half                                                                                   11half
        {-1,0,11,Resistance::EMPTY_}, {1,1,13,Resistance::FRAC12},{2,2,2,Resistance::FRAC12},{3,3,3,Resistance::FRAC12},                                                            {6,6,6,Resistance::FRAC12},
        //       12Electric                    13half
        {-2,8,13,Resistance::EMPTY_}, {1,8,1,Resistance::FRAC12},
        //       14Ghost                                                                                                                                        15immune
        {-3,13,15,Resistance::EMPTY_},                                                                                                                  {5,5,5,Resistance::IMMUNE},
        //       16Ice                                                                                                            17half
        {-4,15,17,Resistance::EMPTY_},                                                                                    {4,4,4,Resistance::FRAC12},
        //       18
        {INT_MIN,17,INT_MIN,Resistance::EMPTY_},
    };

    std::pair<int,std::string> pick = links.coverAttackType(8);
    EXPECT_EQUAL(pick.first,-12);
    EXPECT_EQUAL(pick.second,"Dragon");
    EXPECT_EQUAL(itemCoverElectric, links.itemTable_);
    EXPECT_EQUAL(dlxCoverElectric, links.links_);

    links.uncoverAttackType(8);
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
        //         0                           1Electric                  2Fire                        3Grass                      4Ice                          5Normal                      6Water
        {0,0,0,Resistance::EMPTY_},   {3,21,8,Resistance::EMPTY_},{3,24,9,Resistance::EMPTY_}, {1,12,12,Resistance::EMPTY_},{1,18,18,Resistance::EMPTY_},{1,22,22,Resistance::EMPTY_},{4,25,13,Resistance::EMPTY_},
        //         7Electric                   8                          9
        {-1,0,9,Resistance::EMPTY_},  {1,1,11,Resistance::FRAC12},{2,2,15,Resistance::FRAC12},
        //         10Fire                      11                                                       12                                                                                    13
        {-2,8,13,Resistance::EMPTY_}, {1,8,21,Resistance::FRAC12},                             {3,3,3,Resistance::FRAC12},                                                            {6,6,16,Resistance::FRAC12},
        //         14Grass                                                15                                                                                                                  16
        {-3,11,16,Resistance::EMPTY_},                            {2,9,24,Resistance::FRAC12},                                                                                        {6,13,19,Resistance::FRAC12},
        //         17Ice                                                                                                           18                                                         19
        {-4,15,19,Resistance::EMPTY_},                                                                                      {4,4,4,Resistance::FRAC12},                               {6,16,25,Resistance::FRAC12},
        //         20Normal                    21                                                                                                                22
        {-5,18,22,Resistance::EMPTY_},{1,11,1,Resistance::FRAC12},                                                                                       {5,5,5,Resistance::FRAC12},
        //         23Water                                                24                                                                                                                  25
        {-6,21,25,Resistance::EMPTY_},                            {2,15,2,Resistance::FRAC12},                                                                                        {6,19,6,Resistance::FRAC12},
        {INT_MIN,24,INT_MIN,Resistance::EMPTY_},
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
        //         0                           1Electric                  2Fire                        3Grass                      4Ice                          5Normal                      6Water
        {0,0,0,Resistance::EMPTY_},   {3,21,8,Resistance::EMPTY_},{3,24,9,Resistance::EMPTY_}, {0,3,3,Resistance::EMPTY_},{1,18,18,Resistance::EMPTY_},{0,5,5,Resistance::EMPTY_},  {1,19,19,Resistance::EMPTY_},
        //         7Electric                   8                          9
        {-1,0,9,Resistance::EMPTY_},  {1,1,11,Resistance::FRAC12},{2,2,15,Resistance::FRAC12},
        //         10Fire                      11                                                       12                                                                                    13
        {-2,8,13,Resistance::EMPTY_}, {1,8,21,Resistance::FRAC12},                             {3,3,3,Resistance::FRAC12},                                                            {6,6,16,Resistance::FRAC12},
        //         14Grass                                                15                                                                                                                  16
        {-3,11,16,Resistance::EMPTY_},                            {2,9,24,Resistance::FRAC12},                                                                                        {6,6,19,Resistance::FRAC12},
        //         17Ice                                                                                                           18                                                         19
        {-4,15,19,Resistance::EMPTY_},                                                                                      {4,4,4,Resistance::FRAC12},                               {6,6,6,Resistance::FRAC12},
        //         20Normal                    21                                                                                                                22
        {-5,18,22,Resistance::EMPTY_},{1,11,1,Resistance::FRAC12},                                                                                       {5,5,5,Resistance::FRAC12},
        //         23Water                                                24                                                                                                                  25
        {-6,21,25,Resistance::EMPTY_},                            {2,15,2,Resistance::FRAC12},                                                                                        {6,19,6,Resistance::FRAC12},
        {INT_MIN,24,INT_MIN,Resistance::EMPTY_},
    };

    std::pair<int,std::string> pick = links.coverAttackType(8);
    EXPECT_EQUAL(pick.first,-6);
    EXPECT_EQUAL(pick.second,"Electric");
    EXPECT_EQUAL(headersCoverElectric, links.itemTable_);
    EXPECT_EQUAL(dlxCoverElectric, links.links_);

    links.uncoverAttackType(8);
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
    std::priority_queue<RankedCover> cover = links.getAllCoveredTeams();
    RankedCover firstPlace = {-11,{"Ghost","Ground","Poison","Water"}};
    EXPECT_EQUAL(cover.top(), firstPlace);
    cover.pop();
    // Higher numbers are worst for defense. More damage is possible.
    RankedCover secondPlace = {-13,{"Electric","Ghost","Poison","Water"}};
    EXPECT_EQUAL(cover.top(), secondPlace);
    cover.pop();
    EXPECT(cover.empty());
}
