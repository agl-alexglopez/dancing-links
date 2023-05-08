#include "Src/DisasterTags.h"
#include "Src/DisasterUtilities.h"
#include "GenericOverloads.h"

namespace DancingLinks {

/* * * * * * * * * * * * * * * * *        Debugging Operators           * * * * * * * * * * * * * */


bool operator==(const DisasterTags::city& lhs, const DisasterTags::city& rhs) {
    return lhs.topOrLen == rhs.topOrLen && lhs.up == rhs.up && lhs.down == rhs.down;
}

bool operator!=(const DisasterTags::city& lhs, const DisasterTags::city& rhs) {
    return !(lhs == rhs);
}

bool operator==(const DisasterTags::cityName& lhs, const DisasterTags::cityName& rhs) {
    return lhs.name == rhs.name && lhs.left == rhs.left && lhs.right == rhs.right;
}

bool operator!=(const DisasterTags::cityName& lhs, const DisasterTags::cityName& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, const DisasterTags::city& person) {
    os << "{ topOrLen: " << person.topOrLen
       << ", up: " << person.up << ", down: " << person.down << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const DisasterTags::cityName& name) {
    os << "{ name: " << name.name << ", left: " << name.left << ", right: " << name.right << " }";
    return os;
}

std::ostream& operator<<(std::ostream&os, const std::vector<DisasterTags::cityName>& links) {
    os << "LOOKUP TABLE" << std::endl;
    for (const auto& item : links) {
        os << "{\"" << item.name << "\"," << item.left << "," << item.right << "}" << std::endl;
    }
    os << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream&os, const std::vector<DisasterTags::city>& links) {
    os << "DLX ARRAY" << std::endl;
    int index = 0;
    for (const auto& item : links) {
        if (item.topOrLen < 0) {
            os << std::endl;
        }
        os << "{"
           << item.topOrLen << ","
           << item.up << ","
           << item.down << ","
           << item.supplyTag << "}";
        index++;
    }
    os << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream&os, const DisasterTags& links) {
    os << "LOOKUP ARRAY" << std::endl;
    for (const auto& header : links.table_) {
        os << "{\""
           << header.name << "\","
           << header.left << ","
           << header.right << "},"
           << std::endl;
    }
    os << "DLX ARRAY" << std::endl;
    int index = 0;
    for (const auto& item : links.grid_) {
        if (index >= links.table_.size() && item.topOrLen < 0) {
            os << std::endl;
        }
        os << "{"
           << item.topOrLen << ","
           << item.up << ","
           << item.down << ","
           << item.supplyTag << "},";
        index++;
    }
    os << std::endl;
    os << "Number of Cities: " << links.numItemsAndOptions_ << std::endl;
    return os;
}

} // namespace DancingLinks

namespace Dx = DancingLinks;

/* * * * * * * * * * * * * * * * *     Test Cases Below This Point      * * * * * * * * * * * * * */


/* * * * * * * * * * * * * * * * *          Initialization Tests        * * * * * * * * * * * * * */


STUDENT_TEST("Initialize a small dancing links.") {
    /*
     *
     *             B
     *             |
     *        A -- C
     *
     *
     *
     */
    const std::map<std::string, std::set<std::string>> cities = {
        {"A", {"C"}},
        {"B", {"C"}},
        {"C", {"A", "B"}},
    };
    std::vector<Dx::DisasterTags::cityName> networkHeaders = {
        {"",  3, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 0}
    };
    std::vector<Dx::DisasterTags::city> dlxItems = {
        // 0           1A        2B        3C
        {0,0,0,0},  {2,9,5,0},{2,12,6,0},{3,13,7,0},
        // 4           5A        6B        7C
        {-3,0,7,0}, {1,1,9,0},{2,2,12,0},{3,3,10,0},
        // 8        9A                     10C
        {-1,5,10,0},{1,5,1,0},           {3,7,13,0},
        // 11                    12A       13C
        {-2,9,13,0},          {2,6,2,0}, {3,10,3,0},
        {INT_MIN,12,INT_MIN,0},
    };
    Dx::DisasterTags network(cities);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}

STUDENT_TEST("Initialize larger dancing links.") {
    /*
     *
     *        F -- D -- B -- E -- C -- A
     *
     */
    const std::map<std::string, std::set<std::string>> cities = {
        {"A", {"C"}},
        {"B", {"D", "E"}},
        {"C", {"A", "E"}},
        {"D", {"B", "F"}},
        {"E", {"B", "C"}},
        {"F", {"D"}},
    };
    std::vector<Dx::DisasterTags::cityName> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    std::vector<Dx::DisasterTags::city> dlxItems = {
        //  0            1A          2B          3C         4D           5E          6F
        {0,0,0,0},   {2,24,12,0},{3,20,8,0}, {3,25,13,0},{3,27,9,0}, {3,22,10,0},{2,28,18,0},
        //  7                       8B                      9D          10E
        {-2,0,10,0},             {2,2,16,0},             {4,4,17,0}, {5,5,14,0},
        //  11          12A                     13C                     14E
        {-3,8,14,0}, {1,1,24,0},             {3,3,21,0},             {5,10,22,0},
        //  15                     16B                      17D                     18F
        {-4,12,18,0},            {2,8,20,0},             {4,9,27,0},             {6,6,28,0},
        //  19                     20B          21C                     22E
        {-5,16,22,0},            {2,16,2,0}, {3,13,25,0},            {5,14,5,0},
        //  23         24A                      25C
        {-1,20,25,0},{1,12,1,0},             {3,21,3,0},
        //  26                                              27D         28E
        {-6,24,28,0},                                    {4,17,4,0}, {6,18,6,0},
        {INT_MIN,27,INT_MIN,0},
    };
    Dx::DisasterTags network(cities);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}

STUDENT_TEST("Simple Ethene Network initialization.") {
    /*
     *
     *             C
     *             |
     *        A -- D -- B -- F
     *                  |
     *                  E
     *
     */
    const std::map<std::string, std::set<std::string>> cities = {
        {"A", {"D"}},
        {"B", {"D", "E", "F"}},
        {"C", {"D"}},
        {"D", {"A", "C", "B"}},
        {"E", {"B"}},
        {"F", {"B"}},
    };
    std::vector<Dx::DisasterTags::cityName> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    std::vector<Dx::DisasterTags::city> dlxItems = {
        //  0            1A          2B         3C          4D         5E          6F
        {0,0,0,0},   {2,18,13,0},{4,27,8,0},{2,21,15,0},{4,22,9,0},{2,25,10,0},{2,28,11,0},
        //  7                        8B                     9D         10E         11F
        {-2,0,11,0},             {2,2,14,0},            {4,4,16,0},{5,5,25,0}, {6,6,28,0},
        //  12          13A          14B        15C        16D
        {-4,8,16,0}, {1,1,18,0}, {2,8,24,0},{3,3,21,0}, {4,9,19,0},
        //  17          18A                                19D
        {-1,13,19,0},{1,13,1,0},                        {4,16,22,0},
        //  20                                  21C        22D
        {-3,18,22,0},                       {3,15,3,0}, {4,19,4,0},
        //  23                       24B                              25E
        {-5,21,25,0},            {2,14,27,0},                      {5,10,5,0},
        //  26                       27B                                           28F
        {-6,24,28,0},            {2,24,2,0},                                    {6,11,6,0},
        {INT_MIN,27,INT_MIN,0},
    };
    Dx::DisasterTags network (cities);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}


/* * * * * * * * * * * * * * * * *      Cover/Uncover Tests             * * * * * * * * * * * * * */


STUDENT_TEST("Cover uncover A should only cover A and C from header list.") {
    /*
     *
     *             B
     *             |
     *        A -- C
     *
     *
     *
     */
    const std::map<std::string, std::set<std::string>> cities = {
        {"A", {"C"}},
        {"B", {"C"}},
        {"C", {"A", "B"}},
    };
    std::vector<Dx::DisasterTags::cityName> networkHeaders = {
        {"",  3, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 0}
    };
    std::vector<Dx::DisasterTags::city> dlxItems = {
        // 0           1A        2B        3C
        {0,0,0,0},  {2,9,5,0},{2,12,6,0},{3,13,7,0},
        // 4           5A        6B        7C
        {-3,0,7,0}, {1,1,9,0},{2,2,12,0},{3,3,10,0},
        // 8        9A                     10C
        {-1,5,10,0},{1,5,1,0},           {3,7,13,0},
        // 11                    12A       13C
        {-2,9,13,0},          {2,6,2,0}, {3,10,3,0},
        {INT_MIN,12,INT_MIN,0},
    };
    Dx::DisasterTags network(cities);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);

    std::string location = network.coverCity(9,1);
    EXPECT_EQUAL(location, "A");
    std::vector<Dx::DisasterTags::cityName> headersCoverA {
        {"",  2, 2},
        {"A", 0, 2},
        {"B", 0, 0},
        {"C", 2, 0}
    };
    std::vector<Dx::DisasterTags::city> dlxCoverA = {
        // 0           1A        2B        3C
        {0,0,0,0},  {2,9,5,1},{2,12,6,0},{3,13,7,1},
        // 4           5A        6B        7C
        {-3,0,7,0}, {1,1,9,0},{2,2,12,0},{3,3,10,0},
        // 8           9A                  10C
        {-1,5,10,0},{1,5,1,1},           {3,7,13,1},
        // 11                    12A       13C
        {-2,9,13,0},          {2,6,2,0}, {3,10,3,0},
        {INT_MIN,12,INT_MIN,0},
    };
    EXPECT_EQUAL(network.table_, headersCoverA);
    EXPECT_EQUAL(network.grid_, dlxCoverA);

    network.uncoverCity(9);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}

STUDENT_TEST("Recursive depth field prevents uncovering cities that should remain covered.") {
    /*
     *
     *             C
     *             |
     *        A -- D -- B -- F
     *                  |
     *                  E
     *
     */
    const std::map<std::string, std::set<std::string>> cities = {
        {"A", {"D"}},
        {"B", {"D", "E", "F"}},
        {"C", {"D"}},
        {"D", {"A", "C", "B"}},
        {"E", {"B"}},
        {"F", {"B"}},
    };
    std::vector<Dx::DisasterTags::cityName> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    std::vector<Dx::DisasterTags::city> dlxItems = {
        //  0            1A          2B         3C          4D         5E          6F
        {0,0,0,0},   {2,18,13,0},{4,27,8,0},{2,21,15,0},{4,22,9,0},{2,25,10,0},{2,28,11,0},
        //  7                        8B                     9D         10E         11F
        {-2,0,11,0},             {2,2,14,0},            {4,4,16,0},{5,5,25,0}, {6,6,28,0},
        //  12          13A          14B        15C        16D
        {-4,8,16,0}, {1,1,18,0}, {2,8,24,0},{3,3,21,0}, {4,9,19,0},
        //  17          18A                                19D
        {-1,13,19,0},{1,13,1,0},                        {4,16,22,0},
        //  20                                  21C        22D
        {-3,18,22,0},                       {3,15,3,0}, {4,19,4,0},
        //  23                       24B                              25E
        {-5,21,25,0},            {2,14,27,0},                      {5,10,5,0},
        //  26                       27B                                           28F
        {-6,24,28,0},            {2,24,2,0},                                    {6,11,6,0},
        {INT_MIN,27,INT_MIN,0},
    };
    Dx::DisasterTags network (cities);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);

    std::string location = network.coverCity(13,1);
    EXPECT_EQUAL(location, "D");
    std::vector<Dx::DisasterTags::cityName> headersCoverD = {
        {"",  6, 5},
        {"A", 0, 2},
        {"B", 0, 3},
        {"C", 0, 4},
        {"D", 0, 5},
        {"E", 0, 6},
        {"F", 5, 0}
    };
    std::vector<Dx::DisasterTags::city> dlxCoverD = {
        //  0            1A          2B         3C          4D         5E          6F
        {0,0,0,0},   {2,18,13,1},{4,27,8,1},{2,21,15,1},{4,22,9,1},{2,25,10,0},{2,28,11,0},
        //  7                        8B                     9D         10E         11F
        {-2,0,11,0},             {2,2,14,0},            {4,4,16,0},{5,5,25,0}, {6,6,28,0},
        //  12          13A          14B        15C        16D
        {-4,8,16,0}, {1,1,18,1}, {2,8,24,1},{3,3,21,1}, {4,9,19,1},
        //  17          18A                                19D
        {-1,13,19,0},{1,13,1,0},                        {4,16,22,0},
        //  20                                  21C        22D
        {-3,18,22,0},                       {3,15,3,0}, {4,19,4,0},
        //  23                       24B                              25E
        {-5,21,25,0},            {2,14,27,0},                      {5,10,5,0},
        //  26                       27B                                           28F
        {-6,24,28,0},            {2,24,2,0},                                    {6,11,6,0},
        {INT_MIN,27,INT_MIN,0},
    };
    EXPECT_EQUAL(network.table_, headersCoverD);
    EXPECT_EQUAL(network.grid_, dlxCoverD);

    location = network.coverCity(10,2);
    EXPECT_EQUAL(location, "B");
    std::vector<Dx::DisasterTags::cityName> headersCoverB = {
        {"",  0, 0},
        {"A", 0, 2},
        {"B", 0, 3},
        {"C", 0, 4},
        {"D", 0, 5},
        {"E", 0, 6},
        {"F", 0, 0}
    };
    std::vector<Dx::DisasterTags::city> dlxCoverB = {
        //  0            1A          2B         3C          4D         5E          6F
        {0,0,0,0},   {2,18,13,1},{4,27,8,1},{2,21,15,1},{4,22,9,1},{2,25,10,2},{2,28,11,2},
        //  7                        8B                     9D         10E         11F
        {-2,0,11,0},             {2,2,14,2},            {4,4,16,2},{5,5,25,2}, {6,6,28,2},
        //  12          13A          14B        15C        16D
        {-4,8,16,0}, {1,1,18,1}, {2,8,24,1},{3,3,21,1}, {4,9,19,1},
        //  17          18A                                19D
        {-1,13,19,0},{1,13,1,0},                        {4,16,22,0},
        //  20                                  21C        22D
        {-3,18,22,0},                       {3,15,3,0}, {4,19,4,0},
        //  23                       24B                              25E
        {-5,21,25,0},            {2,14,27,0},                      {5,10,5,0},
        //  26                       27B                                           28F
        {-6,24,28,0},            {2,24,2,0},                                    {6,11,6,0},
        {INT_MIN,27,INT_MIN,0},
    };
    EXPECT_EQUAL(network.table_, headersCoverB);
    EXPECT_EQUAL(network.grid_, dlxCoverB);

    network.uncoverCity(10);
    EXPECT_EQUAL(network.table_, headersCoverD);
    EXPECT_EQUAL(network.grid_, dlxCoverD);
    network.uncoverCity(13);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}


/* * * * * * * * * * * * * * * * *      Full Coverage Tests             * * * * * * * * * * * * * */


STUDENT_TEST("Can cover this map with one supply.") {
    /*
     *
     *             B
     *             |
     *        A -- C
     *
     *
     *
     */
    const std::map<std::string, std::set<std::string>> cities = {
        {"A", {"C"}},
        {"B", {"C"}},
        {"C", {"A", "B"}},
    };
    std::vector<Dx::DisasterTags::cityName> networkHeaders = {
        {"",  3, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 0}
    };
    std::vector<Dx::DisasterTags::city> dlxItems = {
        // 0           1A        2B        3C
        {0,0,0,0},  {2,9,5,0},{2,12,6,0},{3,13,7,0},
        // 4           5A        6B        7C
        {-3,0,7,0}, {1,1,9,0},{2,2,12,0},{3,3,10,0},
        // 8           9A                 10C
        {-1,5,10,0},{1,5,1,0},           {3,7,13,0},
        // 11                    12A       13C
        {-2,9,13,0},          {2,6,2,0}, {3,10,3,0},
        {INT_MIN,12,INT_MIN,0},
    };
    std::set<std::string> locations = {};

    Dx::DisasterTags network(cities);

    EXPECT(network.hasDisasterCoverage(1,locations));
    EXPECT_EQUAL(locations, {"C"});
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}

STUDENT_TEST("Can cover Ethene with two supplies.") {
    /*
     *
     *             C
     *             |
     *        A -- D -- B -- F
     *                  |
     *                  E
     *
     */
    const std::map<std::string, std::set<std::string>> cities = {
        {"A", {"D"}},
        {"B", {"D", "E", "F"}},
        {"C", {"D"}},
        {"D", {"A", "C", "B"}},
        {"E", {"B"}},
        {"F", {"B"}},
    };
    std::vector<Dx::DisasterTags::cityName> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    std::vector<Dx::DisasterTags::city> dlxItems = {
        //  0            1A          2B         3C          4D         5E          6F
        {0,0,0,0},   {2,18,13,0},{4,27,8,0},{2,21,15,0},{4,22,9,0},{2,25,10,0},{2,28,11,0},
        //  7                        8B                     9D         10E         11F
        {-2,0,11,0},             {2,2,14,0},            {4,4,16,0},{5,5,25,0}, {6,6,28,0},
        //  12          13A          14B        15C        16D
        {-4,8,16,0}, {1,1,18,0}, {2,8,24,0},{3,3,21,0}, {4,9,19,0},
        //  17          18A                                19D
        {-1,13,19,0},{1,13,1,0},                        {4,16,22,0},
        //  20                                  21C        22D
        {-3,18,22,0},                       {3,15,3,0}, {4,19,4,0},
        //  23                       24B                              25E
        {-5,21,25,0},            {2,14,27,0},                      {5,10,5,0},
        //  26                       27B                                           28F
        {-6,24,28,0},            {2,24,2,0},                                    {6,11,6,0},
        {INT_MIN,27,INT_MIN,0},
    };
    std::set<std::string> locations = {};
    Dx::DisasterTags network (cities);
    EXPECT(network.hasDisasterCoverage(2,locations));
    EXPECT_EQUAL(locations, {"D","B"});
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}

STUDENT_TEST("Passes Straight Line test with two supplies.") {
    /*
     *
     *        F -- D -- B -- E -- C -- A
     *
     */
    const std::map<std::string, std::set<std::string>> cities = {
        {"A", {"C"}},
        {"B", {"D", "E"}},
        {"C", {"A", "E"}},
        {"D", {"B", "F"}},
        {"E", {"B", "C"}},
        {"F", {"D"}},
    };
    std::vector<Dx::DisasterTags::cityName> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    std::vector<Dx::DisasterTags::city> dlxItems = {
        //  0            1A          2B          3C         4D           5E          6F
        {0,0,0,0},   {2,24,12,0},{3,20,8,0}, {3,25,13,0},{3,27,9,0}, {3,22,10,0},{2,28,18,0},
        //  7                       8B                      9D          10E
        {-2,0,10,0},             {2,2,16,0},             {4,4,17,0}, {5,5,14,0},
        //  11          12A                     13C                     14E
        {-3,8,14,0}, {1,1,24,0},             {3,3,21,0},             {5,10,22,0},
        //  15                     16B                      17D                     18F
        {-4,12,18,0},            {2,8,20,0},             {4,9,27,0},             {6,6,28,0},
        //  19                     20B          21C                     22E
        {-5,16,22,0},            {2,16,2,0}, {3,13,25,0},            {5,14,5,0},
        //  23         24A                      25C
        {-1,20,25,0},{1,12,1,0},             {3,21,3,0},
        //  26                                              27D         28E
        {-6,24,28,0},                                    {4,17,4,0}, {6,18,6,0},
        {INT_MIN,27,INT_MIN,0},
    };
    std::set<std::string> locations = {};
    Dx::DisasterTags network (cities);
    EXPECT(network.hasDisasterCoverage(2,locations));
    EXPECT_EQUAL(locations, {"D","C"});
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}

STUDENT_TEST("Do not be greedy with our algorithm in terms of most connected cities.") {
    /*
     *
     *     A       E
     *     |       |
     *     B - D - F
     *      \ / \ /
     *       C   G
     *
     */

    const std::map<std::string, std::set<std::string>> cities = {
        {"A", {"B"}},
        {"B", {"A", "C", "D"}},
        {"C", {"B", "D"}},
        {"D", {"B", "C", "F", "G"}},
        {"E", {"F"}},
        {"F", {"D", "E", "G"}},
        {"G", {"D", "F"}},
    };
    Dx::DisasterTags network(cities);
    std::set<std::string> chosen = {};
    EXPECT(!network.hasDisasterCoverage(1, chosen));
    chosen.clear();
    EXPECT(network.hasDisasterCoverage(2, chosen));
    EXPECT_EQUAL(chosen, {"B","F"});
}

STUDENT_TEST("Supply an island city when we must.") {
    /*
     *
     *     A       E
     *     |
     *     B
     *      \
     *       C-D
     *
     */

    const std::map<std::string, std::set<std::string>> cities = {
        {"A", {"B"}},
        {"B", {"A", "C"}},
        {"C", {"B", "D"}},
        {"D", {"C"}},
        {"E", {}}
    };
    Dx::DisasterTags network(cities);
    std::set<std::string> chosen = {};
    EXPECT(!network.hasDisasterCoverage(1, chosen));
    EXPECT(!network.hasDisasterCoverage(2, chosen));
    EXPECT(network.hasDisasterCoverage(3, chosen));
}

STUDENT_TEST("Supply 5 island cities when we must.") {
    /*
     *
     *     A            E
     *
     *            B
     *
     *      C              D
     *
     */

    const std::map<std::string, std::set<std::string>> cities = {
        {"A", {}},
        {"B", {}},
        {"C", {}},
        {"D", {}},
        {"E", {}}
    };
    std::vector<Dx::DisasterTags::cityName> networkHeaders = {
        {"",5,1},
        {"A",0,2},
        {"B",1,3},
        {"C",2,4},
        {"D",3,5},
        {"E",4,0},
    };
    std::vector<Dx::DisasterTags::city> dlxItems = {
        {0,0,0,0},   {1,7,7,0},{1,9,9,0},{1,11,11,0},{1,13,13,0},{1,15,15,0},
        {-1,0,7,0},  {1,1,1,0},
        {-2,7,9,0},            {2,2,2,0},
        {-3,9,11,0},                     {3,3,3,0},
        {-4,11,13,0},                                 {4,4,4,0},
        {-5,13,15,0},                                             {5,5,5,0},
        {INT_MIN,15,INT_MIN,0},
    };
    Dx::DisasterTags network(cities);
    std::set<std::string> chosen = {};
    EXPECT(!network.hasDisasterCoverage(1, chosen));
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
    EXPECT(!network.hasDisasterCoverage(2, chosen));
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
    EXPECT(!network.hasDisasterCoverage(3, chosen));
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
    EXPECT(!network.hasDisasterCoverage(4, chosen));
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
    EXPECT(network.hasDisasterCoverage(5, chosen));
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}

PROVIDED_TEST("Can solve ethene example, regardless of ordering, with output.") {
    /* Because std::map and std::set internally store items in sorted order, the order
     * in which you iterate over the cities when making decisions is sensitive
     * to the order of those cities' names. This test looks at a map like
     * this one, trying out all possible orderings of the city names:
     *
     *             *
     *             |
     *        * -- * -- * -- *
     *                  |
     *                  *
     *
     * There are 6! = 720 possible permutations of the ordering of these six
     * cities. If your code is able to solve the problem correctly for all of
     * those orderings, there's a good chance that you're correctly
     * covering and uncovering cities at each step. On the other hand, if
     * your code runs into issues here, it may indicate that the way in which
     * you're covering and uncovering cities accidentally uncovers a city that
     * you have previously covered.
     */
    Vector<std::string> cities = { "A", "B", "C", "D", "E", "F" };
    do {
        /* Ethene arrangement:
         *
         *        0
         *       1234
         *         5
         */
        std::map<std::string, std::set<std::string>> map = makeMap({
            { cities[2], { cities[0], cities[1], cities[3] } },
            { cities[3], { cities[4], cities[5] } }
        });

        Dx::DisasterTags network(map);
        std::set<std::string> chosen = {};
        EXPECT(network.hasDisasterCoverage(2, chosen));

        EXPECT_EQUAL(chosen.size(), 2);
        EXPECT(chosen.count(cities[2]));
        EXPECT(chosen.count(cities[3]));

        chosen.clear();

        EXPECT(!network.hasDisasterCoverage(1, chosen));
    } while (next_permutation(cities.begin(), cities.end()));
}

PROVIDED_TEST("Works for six cities in a line, regardless of order with output.") {
    /* Because std::map and std::set internally store items in sorted order, the order
     * in which you iterate over the cities when making decisions is sensitive
     * to the order of those cities' names. This test looks at a map like
     * this one, trying out all possible orderings of the city names:
     *
     *        * -- * -- * -- * -- * -- *
     *
     * There are 6! = 720 possible permutations of the ordering of these six
     * cities in a line. If your code is able to solve the problem correctly
     * for all of those orderings, there's a good chance that you're correctly
     * covering and uncovering cities at each step. On the other hand, if
     * your code runs into issues here, it may indicate that the way in which
     * you're covering and uncovering cities accidentally uncovers a city that
     * you have previously covered.
     */
    Vector<std::string> cities = { "A", "B", "C", "D", "E", "F" };
    do {
        /* Linear arrangement. */
        std::map<std::string, std::set<std::string>> map;
        for (int i = 1; i + 1 < cities.size(); i++) {
            map[cities[i]] = { cities[i - 1], cities[i + 1] };
        }

        std::set<std::string> chosen = {};
        map = makeMap(map);

        Dx::DisasterTags network(map);
        EXPECT(network.hasDisasterCoverage(2, chosen));

        EXPECT_EQUAL(chosen.size(), 2);
        EXPECT(chosen.count(cities[1]));
        EXPECT(chosen.count(cities[4]));

        chosen.clear();

        EXPECT(!network.hasDisasterCoverage(1, chosen));
    } while (next_permutation(cities.begin(), cities.end()));
}

PROVIDED_TEST("Solves \"Don't be Greedy,\" regardless of ordering with output.") {
    /* Because std::map and std::set internally store items in sorted order, the order
     * in which you iterate over the cities when making decisions is sensitive
     * to the order of those cities' names. This test looks at a map like
     * this one, trying out all possible orderings of the city names:
     *
     *     0       4
     *     |       |
     *     1 - 2 - 3
     *      \ / \ /
     *       5   6
     *
     * There are 7! = 5,040 possible permutations of the ordering of these seven
     * cities. If your code is able to solve the problem correctly for all of
     * those orderings, there's a good chance that you're correctly
     * covering and uncovering cities at each step. On the other hand, if
     * your code runs into issues here, it may indicate that the way in which
     * you're covering and uncovering cities accidentally uncovers a city that
     * you have previously covered.
     */
    Vector<std::string> cities = { "A", "B", "C", "D", "E", "F", "G" };
    do {
        std::map<std::string, std::set<std::string>> map = makeMap({
            { cities[1], { cities[0], cities[2], cities[5] } },
            { cities[2], { cities[3], cities[5], cities[6] } },
            { cities[3], { cities[4], cities[6] } },
        });

        std::set<std::string> chosen = {};

        Dx::DisasterTags network(map);
        /* We should be able to cover everything with two cities. */
        EXPECT(network.hasDisasterCoverage(2, chosen));

        /* Those cities should be 1 and 3. */
        EXPECT_EQUAL(chosen, { cities[1], cities[3] });

        chosen.clear();

        EXPECT(!network.hasDisasterCoverage(1, chosen));
    } while (next_permutation(cities.begin(), cities.end()));
}

STUDENT_TEST("We should still be quick if we start by focussing on most isolated so far.") {
    /*
     *
     * 0 is extremely well connected but will play no role in final solution to supply all cities.
     * Every surrounding city in the inner ring must receive supplies.
     *
     *             32   17   18
     *          31   \  |  /   19
     *            \  16 1 2   /
     *             15 | | | 3
     *     30 --14--| | | | |--4--- 20
     *     29 --13--|-|-0-|-|--5--- 21
     *     28 --12--| | | | |--6--- 22
     *             11 | | |-7
     *            /  10 9 8  \
     *          27   /  |  \  23
     *              26  25  24
     *
     */
    Vector<std::string> cities = {
        "A", "B", "C", "D", "E", "F", "G", "H",
        "I", "J", "K", "L", "M", "N", "O", "P",
        "Q", "R", "S", "T", "U", "V", "W", "X",
        "Y", "Z", "?", "@", "#", "$", "%", ")","*",
    };
    std::map<std::string, std::set<std::string>> map = makeMap({
        { cities[0], { cities[1], cities[2], cities[3], cities[4], cities[5], cities[6],
                       cities[7],cities[8],cities[9],cities[10],cities[11],cities[12],
                       cities[13],cities[14],cities[15],cities[16],} },
        { cities[17], { cities[1]} },
        { cities[18], { cities[2]} },
        { cities[19], { cities[3]} },
        { cities[20], { cities[4]} },
        { cities[21], { cities[5]} },
        { cities[22], { cities[6]} },
        { cities[23], { cities[7]} },
        { cities[24], { cities[8]} },
        { cities[25], { cities[9]} },
        { cities[26], { cities[10]} },
        { cities[27], { cities[11]} },
        { cities[28], { cities[12]} },
        { cities[29], { cities[13]} },
        { cities[30], { cities[14]} },
        { cities[31], { cities[15]} },
        { cities[32], { cities[16]} },
    });

    std::set<std::string> chosen = {};

    Dx::DisasterTags network(map);
    /* We should be able to cover everything with two cities. */
    EXPECT(network.hasDisasterCoverage(16, chosen));

    /* Those cities should be 1 and 3. */
    EXPECT_EQUAL(chosen, { cities[1], cities[2], cities[3], cities[4], cities[5], cities[6],
                           cities[7],cities[8],cities[9],cities[10],cities[11],cities[12],
                           cities[13],cities[14],cities[15],cities[16], });

    chosen.clear();

    EXPECT(!network.hasDisasterCoverage(15, chosen));
}

PROVIDED_TEST("Stress test: 6 x 6 grid. (This should take at most a few seconds.)") {
    std::map<std::string, std::set<std::string>> grid;

    /* Build the grid. */
    char maxRow = 'F';
    int  maxCol = 6;
    for (char row = 'A'; row <= maxRow; row++) {
        for (int col = 1; col <= maxCol; col++) {
            if (row != maxRow) {
                grid[row + std::to_string(col)].insert((char(row + 1) + std::to_string(col)));
            }
            if (col != maxCol) {
                grid[row + std::to_string(col)].insert((char(row) + std::to_string(col + 1)));
            }
        }
    }
    grid = makeMap(grid);

    Dx::DisasterTags network(grid);
    std::set<std::string> locations;
    EXPECT(network.hasDisasterCoverage(10, locations));
}

PROVIDED_TEST("Stress test: 6 x 6 grid, with output. (This should take at most a few seconds.)") {
    std::map<std::string, std::set<std::string>> grid;

    /* Build the grid. */
    char maxRow = 'F';
    int  maxCol = 6;
    for (char row = 'A'; row <= maxRow; row++) {
        for (int col = 1; col <= maxCol; col++) {
            if (row != maxRow) {
                grid[row + std::to_string(col)].insert((char(row + 1) + std::to_string(col)));
            }
            if (col != maxCol) {
                grid[row + std::to_string(col)].insert((char(row) + std::to_string(col + 1)));
            }
        }
    }
    grid = makeMap(grid);

    Dx::DisasterTags network(grid);
    std::set<std::string> locations;
    EXPECT(network.hasDisasterCoverage(10, locations));

    for (char row = 'A'; row <= maxRow; row++) {
        for (int col = 1; col <= maxCol; col++) {
            EXPECT(checkCovered(row + std::to_string(col), grid, locations));
        }
    }
}

PROVIDED_TEST("Stress test: 8 x 8 grid, with output. (This should take at most a few seconds.)") {
    std::map<std::string, std::set<std::string>> grid;

    /* Build the grid. */
    char maxRow = 'G';
    int  maxCol = 8;
    for (char row = 'A'; row <= maxRow; row++) {
        for (int col = 1; col <= maxCol; col++) {
            if (row != maxRow) {
                grid[row + std::to_string(col)].insert((char(row + 1) + std::to_string(col)));
            }
            if (col != maxCol) {
                grid[row + std::to_string(col)].insert((char(row) + std::to_string(col + 1)));
            }
        }
    }
    grid = makeMap(grid);

    Dx::DisasterTags network(grid);
    std::set<std::string> locations;
    EXPECT(network.hasDisasterCoverage(14, locations));

    for (char row = 'A'; row <= maxRow; row++) {
        for (int col = 1; col <= maxCol; col++) {
            EXPECT(checkCovered(row + std::to_string(col), grid, locations));
        }
    }
}

STUDENT_TEST("All possible configurations of a square.") {
    /*
     *
     *        A----B
     *        |    |
     *        C----D
     *
     *
     *
     */
    const std::map<std::string, std::set<std::string>> cities = {
        {"A", {"B","C"}},
        {"B", {"A","D"}},
        {"C", {"A","D"}},
        {"D", {"B","C"}},
    };
    std::set<std::set<std::string>> allConfigs = {
        {"A","C"},
        {"A","B"},
        {"A","D"},
        {"B","C"},
        {"B","D"},
        {"C","D"},
    };
    Dx::DisasterTags grid(cities);
    std::set<std::set<std::string>> allFound = grid.getAllDisasterConfigurations(2);
    EXPECT_EQUAL(allFound,allConfigs);
}

STUDENT_TEST("All possible configurations with 4 supplies are many.") {
    /*
     *
     *   H--A     E--I
     *      |     |
     *      B--D--F
     *      |     |
     *      C     G
     *
     */
    const std::map<std::string, std::set<std::string>> cities = {
        {"A", {"H","B"}},
        {"B", {"A","C","D"}},
        {"C", {"B"}},
        {"D", {"B","F"}},
        {"E", {"F","I"}},
        {"F", {"D","E","G"}},
        {"G", {"F"}},
        {"H", {"A"}},
        {"I", {"E"}},
    };
    Dx::DisasterTags grid(cities);
    std::set<std::set<std::string>> allFound = grid.getAllDisasterConfigurations(4);
    std::set<std::set<std::string>> allConfigs = {
        {"A", "B", "E", "F"},
        {"A", "B", "E", "G"},
        {"A", "B", "F", "I"},
        {"A", "B", "G", "I"},
        {"A", "C", "E", "F"},
        {"A", "C", "F", "I"},
        {"B", "E", "F", "H"},
        {"B", "E", "G", "H"},
        {"B", "F", "H", "I"},
        {"B", "G", "H", "I"},
        {"C", "E", "F", "H"},
        {"C", "F", "H", "I"}
    };
    EXPECT_EQUAL(allFound,allConfigs);
}

STUDENT_TEST("Larger maps are more difficult to filter out duplicates.") {
    /*
     * This is a good test for when I am trying to avoid generating duplicates. Put a number
     * counter in the getAllDisasterConfigurations function and have it count every time a
     * set is generated. You will see that we generate extra sets and the containing set filters
     * them for us. I cannot yet figure out how to only generate unique sets.
     *
     *                                    /--S-----------
     *                         T --------   /            \
     *                          \     ----R               \
     *                           \   /    |                \
     *                            -U      Q       L-- K--- J
     *                            /      /      /  \   |   |
     *                 A         /      P      O   N   M---I
     *                  \       /       |     |    |   |  /
     *                   -----B----C -- E-----F----G---H--
     *                             \
     *                              D
     */
    const std::map<std::string, std::set<std::string>> cities = {
        {"A", {"B"}},
        {"B", {"A","C","U"}},
        {"C", {"B","D","E"}},
        {"D", {"C"}},
        {"E", {"C","P","F"}},
        {"F", {"E","O","G"}},
        {"G", {"F","H","N"}},
        {"H", {"G","I","M"}},
        {"I", {"H","J","M"}},
        {"J", {"I","K","M"}},
        {"K", {"J","L","M"}},
        {"L", {"K","N","O"}},
        {"M", {"H","I","K"}},
        {"N", {"G","L"}},
        {"O", {"F","L"}},
        {"P", {"E","Q"}},
        {"Q", {"P","R"}},
        {"R", {"S","Q","U"}},
        {"S", {"J","R","T"}},
        {"T", {"S","U"}},
        {"U", {"B","R","T"}},
    };
    Dx::DisasterTags grid(cities);
    std::set<std::set<std::string>> allFound = grid.getAllDisasterConfigurations(7);
    std::set<std::set<std::string>> allConfigs = {
        {"A", "C", "E", "H", "L", "R", "S"}, {"A", "C", "F", "I", "L", "Q", "T"},
        {"A", "C", "G", "I", "L", "Q", "T"}, {"A", "C", "G", "J", "L", "Q", "T"},
        {"A", "C", "G", "J", "O", "Q", "T"}, {"A", "D", "E", "H", "L", "R", "S"},
        {"A", "D", "F", "I", "L", "Q", "T"}, {"B", "C", "E", "H", "L", "P", "S"},
        {"B", "C", "E", "H", "L", "Q", "S"}, {"B", "C", "E", "H", "L", "R", "S"},
        {"B", "C", "F", "H", "L", "P", "S"}, {"B", "C", "F", "H", "L", "Q", "S"},
        {"B", "C", "F", "I", "L", "P", "S"}, {"B", "C", "F", "I", "L", "Q", "S"},
        {"B", "C", "F", "I", "L", "Q", "T"}, {"B", "C", "F", "L", "M", "P", "S"},
        {"B", "C", "F", "L", "M", "Q", "S"}, {"B", "C", "F", "M", "N", "P", "S"},
        {"B", "C", "F", "M", "N", "Q", "S"}, {"B", "C", "G", "H", "L", "P", "S"},
        {"B", "C", "G", "H", "L", "Q", "S"}, {"B", "C", "G", "I", "L", "P", "S"},
        {"B", "C", "G", "I", "L", "Q", "S"}, {"B", "C", "G", "I", "L", "Q", "T"},
        {"B", "C", "G", "J", "L", "P", "S"}, {"B", "C", "G", "J", "L", "Q", "S"},
        {"B", "C", "G", "J", "L", "Q", "T"}, {"B", "C", "G", "J", "O", "P", "S"},
        {"B", "C", "G", "J", "O", "Q", "S"}, {"B", "C", "G", "J", "O", "Q", "T"},
        {"B", "C", "G", "L", "M", "P", "S"}, {"B", "C", "G", "L", "M", "Q", "S"},
        {"B", "C", "G", "M", "O", "P", "S"}, {"B", "C", "G", "M", "O", "Q", "S"},
        {"B", "C", "H", "L", "O", "P", "S"}, {"B", "C", "H", "L", "O", "Q", "S"},
        {"B", "C", "M", "N", "O", "P", "S"}, {"B", "C", "M", "N", "O", "Q", "S"},
        {"B", "D", "E", "H", "L", "P", "S"}, {"B", "D", "E", "H", "L", "Q", "S"},
        {"B", "D", "E", "H", "L", "R", "S"}, {"B", "D", "F", "H", "L", "P", "S"},
        {"B", "D", "F", "H", "L", "Q", "S"}, {"B", "D", "F", "I", "L", "P", "S"},
        {"B", "D", "F", "I", "L", "Q", "S"}, {"B", "D", "F", "I", "L", "Q", "T"},
        {"B", "D", "F", "L", "M", "P", "S"}, {"B", "D", "F", "L", "M", "Q", "S"},
        {"B", "D", "F", "M", "N", "P", "S"}, {"B", "D", "F", "M", "N", "Q", "S"},
        {"B", "D", "G", "H", "L", "P", "S"}, {"B", "D", "G", "I", "L", "P", "S"},
        {"B", "D", "G", "J", "L", "P", "S"}, {"B", "D", "G", "J", "O", "P", "S"},
        {"B", "D", "G", "L", "M", "P", "S"}, {"B", "D", "G", "M", "O", "P", "S"},
        {"B", "D", "H", "L", "O", "P", "S"}, {"B", "D", "M", "N", "O", "P", "S"}
    };
    EXPECT_EQUAL(allFound,allConfigs);
}
