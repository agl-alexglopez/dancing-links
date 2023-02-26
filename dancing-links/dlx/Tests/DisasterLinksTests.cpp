#include "Src/DisasterLinks.h"
#include "Src/DisasterUtilities.h"

namespace DancingLinks {

/* * * * * * * * * * * * * * * *   Overloaded Operators for Debugging   * * * * * * * * * * * * * */


bool operator==(const DisasterLinks::cityItem& lhs, const DisasterLinks::cityItem& rhs) {
    return lhs.topOrLen == rhs.topOrLen
            && lhs.up == rhs.up && lhs.down == rhs.down
               && lhs.left == rhs.left && lhs.right == rhs.right;
}

bool operator!=(const DisasterLinks::cityItem& lhs, const DisasterLinks::cityItem& rhs) {
    return !(lhs == rhs);
}

bool operator==(const DisasterLinks::cityHeader& lhs, const DisasterLinks::cityHeader& rhs) {
    return lhs.name == rhs.name && lhs.left == rhs.left && lhs.right == rhs.right;
}

bool operator!=(const DisasterLinks::cityHeader& lhs, const DisasterLinks::cityHeader& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, const DisasterLinks::cityItem& city) {
    os << "{ topOrLen: " << city.topOrLen << ", up: " << city.up << ", down: " << city.down
       << ", left: " << city.left << ", right: " << city.right << " }";
    return os;
}

std::ostream& operator<<(std::ostream& os, const DisasterLinks::cityHeader& city) {
    os << "{ name: " << city.name << ", left: " << city.left << ", right: " << city.right << " }";
    return os;
}

std::ostream& operator<<(std::ostream&os, const std::vector<DisasterLinks::cityItem>& grid) {
    os << "DLX ARRAY" << std::endl;
    for (const auto& item : grid) {
        if (item.topOrLen < 0) {
            os << std::endl;
        }
        os << "{" << item.topOrLen << "," << item.up << "," << item.down << ","
           << item.left << "," << item.right << "}, ";
    }
    os << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream&os, const std::vector<DisasterLinks::cityHeader>& grid) {
    os << "LOOKUP TABLE" << std::endl;
    for (const auto& item : grid) {
        os << "{\"" << item.name << "\"," << item.left << "," << item.right << "}," << std::endl;
    }
    os << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream&os, const DisasterLinks& network) {
    os << "LOOKUP TABLE:" << std::endl;
    for (const auto& header : network.table_) {
        os << "{\"" << header.name << "\"," << header.left << "," << header.right << "},";
    }
    os << std::endl;
    os << "DLX ARRAY:" << std::endl;
    for (const auto& item : network.grid_) {
        if (item.topOrLen < 0) {
            os << std::endl;
        }
        os << "{" << item.topOrLen << "," << item.up << "," << item.down << ","
           << item.left << "," << item.right << "}, ";
    }
    os << std::endl;
    os << "Number of Cities: " << network.numItemsAndOptions_ << std::endl;
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
    std::vector<Dx::DisasterLinks::cityHeader> networkHeaders = {
        {"",  3, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 0}
    };
    std::vector<Dx::DisasterLinks::cityItem> dlxItems = {
        //        0               1A             2B            3C
              {0,0,0,3,1},    {2,9,5,0,2}, {2,12,6,1,3},  {3,13,7,2,0},
        //       4A               5A             6B            7C
        /*C*/ {-3,0,7,7,5},   {1,1,9,4,6}, {2,2,12,5,7},  {3,3,10,6,4},
        //       8B               9A                           10C
        /*A*/ {-1,5,10,10,9}, {1,5,1,8,10},               {3,7,13,9,8},
        //       11C                             12B           13C
        /*B*/ {-2,9,13,13,12},             {2,6,2,11,13}, {3,10,3,12,11},
              {INT_MIN,12,0,13,INT_MIN},
    };
    Dx::DisasterLinks network(cities);
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
    std::vector<Dx::DisasterLinks::cityHeader> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    std::vector<Dx::DisasterLinks::cityItem> dlxItems = {
        //        0                 1A             2B               3C               4D              5E             6F
              {0,0,0,6,1},     {2,24,12,0,2},   {3,20,8,1,3},   {3,25,13,2,4},   {3,27,9,3,5},   {3,22,10,4,6},   {2,28,18,5,0},
        /*B*/ {-2,0,10,10,8},                   {2,2,16,7,9},                    {4,4,17,8,10},  {5,5,14,9,7},
        /*C*/ {-3,8,14,14,12},  {1,1,24,11,13},                 {3,3,21,12,14},                  {5,10,22,13,11},
        /*D*/ {-4,12,18,18,16},                 {2,8,20,15,17},                  {4,9,27,16,18},                  {6,6,28,17,15},
        /*E*/ {-5,16,22,22,20},                 {2,16,2,19,21}, {3,13,25,20,22},                 {5,14,5,21,19},
        /*A*/ {-1,20,25,25,24}, {1,12,1,23,25},                 {3,21,3,24,23},
        /*F*/ {-6,24,28,28,27},                                                  {4,17,4,26,28},                  {6,18,6,27,26},
        {INT_MIN,27,0,28,INT_MIN},
    };
    Dx::DisasterLinks network(cities);
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
    std::vector<Dx::DisasterLinks::cityHeader> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    std::vector<Dx::DisasterLinks::cityItem> dlxItems = {
        /* Network
         *
         *      A  B  C  D  E  F
         *   B     1     1  1  1
         *   D  1  1  1  1
         *   A  1        1
         *   C        1  1
         *   E     1        1
         *   F     1           1
         */


        //        0                    1A             2B                3C                4D                5E               6F
              {0,0,0,6,1},       {2,18,13,0,2},   {4,27,8,1,3},     {2,21,15,2,4},   {4,22,9,3,5},     {2,25,10,4,6},   {2,28,11,5,0},
        //        7                                   8B                                  9D                10E              11F
        /*B*/ {-2,0,11,11,8},                     {2,2,14,7,9},                      {4,4,16,8,10},    {5,5,25,9,11},   {6,6,28,10,7},
        //        12                   13A            14B               15C               16D
        /*D*/ {-4,8,16,16,13},   {1,1,18,12,14},  {2,8,24,13,15},   {3,3,21,14,16},  {4,9,19,15,12},
        //        17                   18A                                                19D
        /*A*/ {-1,13,19,19,18},  {1,13,1,17,19},                                     {4,16,22,18,17},
        //        20                                                    21C               22D
        /*C*/ {-3,18,22,22,21},                                     {3,15,3,20,22},  {4,19,4,21,20},
        //        23                                  24B                                                   25E
        /*E*/ {-5,21,25,25,24},                   {2,14,27,23,25},                                     {5,10,5,24,23},
        //        26                                  27B                                                                    28F
        /*F*/ {-6,24,28,28,27},                   {2,24,2,26,28},                                                       {6,11,6,27,26},
        //        29



              {INT_MIN,27,0,28,INT_MIN},

    };
    Dx::DisasterLinks network (cities);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}


/* * * * * * * * * * * * * * * * *         Cover/Supplying Logic        * * * * * * * * * * * * * */


STUDENT_TEST("Supplying A will only cover A and C. C remains available supply location.") {
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
    std::vector<Dx::DisasterLinks::cityHeader> networkHeaders = {
        {"",  3, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 0}
    };
    std::vector<Dx::DisasterLinks::cityItem> dlxItems = {
        //        0                 1A           2B            3C
              {0,0,0,3,1},     {2,9,5,0,2},  {2,12,6,1,3},  {3,13,7,2,0},
        /*C*/ {-3,0,7,7,5},    {1,1,9,4,6},  {2,2,12,5,7},  {3,3,10,6,4},
        /*A*/ {-1,5,10,10,9},  {1,5,1,8,10},                {3,7,13,9,8},
        /*B*/ {-2,9,13,13,12},               {2,6,2,11,13}, {3,10,3,12,11},
              {INT_MIN,12,0,13,INT_MIN},
    };
    Dx::DisasterLinks network (cities);

    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);

    std::string supplyLocation = network.coverCity(9);

    EXPECT_EQUAL(supplyLocation, "A");

    std::vector<Dx::DisasterLinks::cityHeader> headersCoverA = {
        {"",  2, 2},
        {"A", 0, 2},
        {"B", 0, 0},
        {"C", 2, 0}
    };
    std::vector<Dx::DisasterLinks::cityItem> dlxCoverA = {
        /* Smaller network now.
         *       B
         *    B  1
         *    C  1
         */
        //        0                 1A           2B            3C
              {0,0,0,2,2},     {2,9,5,0,2},  {2,12,6,0,0},  {3,13,7,2,0},
        //        4                 5A           6B            7C
        /*C*/ {-3,0,7,6,6},    {1,1,9,4,6},  {2,2,12,4,4},  {3,3,10,6,4},
        //        8                 9A                         10C
        /*A*/ {-1,5,10,10,9},  {1,5,1,8,10},                {3,7,13,9,8},
        //        11                             12A           13C
        /*B*/ {-2,9,13,12,12},               {2,6,2,11,11}, {3,10,3,12,11},
              {INT_MIN,12,0,13,INT_MIN},
    };
    EXPECT_EQUAL(network.table_, headersCoverA);
    EXPECT_EQUAL(network.grid_, dlxCoverA);
}

STUDENT_TEST("Supplying B will only cover B and C. Make sure splicing from lookupTable works.") {
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
    std::vector<Dx::DisasterLinks::cityHeader> networkHeaders = {
        {"",  3, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 0}
    };
    std::vector<Dx::DisasterLinks::cityItem> dlxItems = {
        //        0                1A             2B            3C
              {0,0,0,3,1},     {2,9,5,0,2},  {2,12,6,1,3},  {3,13,7,2,0},
        /*C*/ {-3,0,7,7,5},    {1,1,9,4,6},  {2,2,12,5,7},  {3,3,10,6,4},
        /*A*/ {-1,5,10,10,9},  {1,5,1,8,10},                {3,7,13,9,8},
        /*B*/ {-2,9,13,13,12},               {2,6,2,11,13}, {3,10,3,12,11},
        {INT_MIN,12,0,13,INT_MIN},
    };
    Dx::DisasterLinks network (cities);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);

    std::string supplyLocation = network.coverCity(12);

    EXPECT_EQUAL(supplyLocation, "B");

    std::vector<Dx::DisasterLinks::cityHeader> headersCoverB = {
        {"",  1, 1},
        {"A", 0, 0},
        {"B", 1, 3},
        {"C", 1, 0}
    };
    std::vector<Dx::DisasterLinks::cityItem> dlxCoverB = {
        /* Smaller network now.
         *       A
         *    A  1
         *    C  1
         */

        //        0                 1A             2B            3C
              {0,0,0,1,1},     {2,9,5,0,0},  {2,12,6,1,3},  {3,13,7,1,0},
        //        4                 5A             6B            7C
        /*C*/ {-3,0,7,5,5},    {1,1,9,4,4},  {2,2,12,5,7},  {3,3,10,5,4},
        //        8                 9A                           10C
        /*A*/ {-1,5,10,9,9},   {1,5,1,8,8},                 {3,7,13,9,8},
        //        11                               12B           13C
        /*B*/ {-2,9,13,13,12},               {2,6,2,11,13}, {3,10,3,12,11},
              {INT_MIN,12,0,13,INT_MIN},
    };
    EXPECT_EQUAL(network.table_, headersCoverB);
    EXPECT_EQUAL(network.grid_, dlxCoverB);
}


STUDENT_TEST("Supplying C will cover all. Make sure splicing from lookupTable works.") {
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
    std::vector<Dx::DisasterLinks::cityHeader> networkHeaders = {
        {"",  3, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 0}
    };
    std::vector<Dx::DisasterLinks::cityItem> dlxItems = {
        //        0                1A             2B            3C
              {0,0,0,3,1},     {2,9,5,0,2},  {2,12,6,1,3},  {3,13,7,2,0},
        /*C*/ {-3,0,7,7,5},    {1,1,9,4,6},  {2,2,12,5,7},  {3,3,10,6,4},
        /*A*/ {-1,5,10,10,9},  {1,5,1,8,10},                {3,7,13,9,8},
        /*B*/ {-2,9,13,13,12},               {2,6,2,11,13}, {3,10,3,12,11},
              {INT_MIN,12,0,13,INT_MIN},
    };
    Dx::DisasterLinks network (cities);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);

    std::string supplyLocation = network.coverCity(5);

    EXPECT_EQUAL(supplyLocation, "C");

    std::vector<Dx::DisasterLinks::cityHeader> headersOptionC = {
        {"",  0, 0},
        {"A", 0, 2},
        {"B", 0, 3},
        {"C", 0, 0}
    };
    std::vector<Dx::DisasterLinks::cityItem> dlxOptionC = {
        /* Successful empty network.
         *
         *       0
         *    A
         *    B
         */

        //        0                 1A             2B           3C
              {0,0,0,0,0},     {2,9,5,0,2},  {2,12,6,0,3},  {3,13,7,0,0},
        //        4                 5A             6B            7C
        /*C*/ {-3,0,7,7,5},    {1,1,9,4,6},  {2,2,12,5,7},  {3,3,10,6,4},
        //        8                 9A                           10C
        /*A*/ {-1,5,10,8,8},   {1,5,1,8,10},                {3,7,13,8,8},
        //        11                               12B           13C
        /*B*/ {-2,9,13,11,11},               {2,6,2,11,13}, {3,10,3,11,11},
        {INT_MIN,12,0,13,INT_MIN},
    };
    EXPECT_EQUAL(network.table_, headersOptionC);
    EXPECT_EQUAL(network.grid_, dlxOptionC);
}


/* * * * * * * * * * * * * * * * *       Cover then Uncover Logic       * * * * * * * * * * * * * */


STUDENT_TEST("Supplying A will only cover A and C with one supply. Uncover to try again.") {
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
    std::vector<Dx::DisasterLinks::cityHeader> networkHeaders = {
        {"",  3, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 0}
    };
    std::vector<Dx::DisasterLinks::cityItem> dlxItems = {
        //        0                1A             2B            3C
              {0,0,0,3,1},     {2,9,5,0,2},  {2,12,6,1,3},  {3,13,7,2,0},
        /*C*/ {-3,0,7,7,5},    {1,1,9,4,6},  {2,2,12,5,7},  {3,3,10,6,4},
        /*A*/ {-1,5,10,10,9},  {1,5,1,8,10},                {3,7,13,9,8},
        /*B*/ {-2,9,13,13,12},               {2,6,2,11,13}, {3,10,3,12,11},
              {INT_MIN,12,0,13,INT_MIN},
    };
    Dx::DisasterLinks network (cities);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);

    std::string supplyLocation = network.coverCity(9);

    EXPECT_EQUAL(supplyLocation, "A");


    std::vector<Dx::DisasterLinks::cityHeader> headersCoverA = {
        {"",  2, 2},
        {"A", 0, 2},
        {"B", 0, 0},
        {"C", 2, 0}
    };
    std::vector<Dx::DisasterLinks::cityItem> dlxCoverA = {
        /* Smaller network now.
         *       B
         *    B  1
         *    C  1
         */
        //        0                 1A           2B            3C
              {0,0,0,2,2},     {2,9,5,0,2},  {2,12,6,0,0},  {3,13,7,2,0},
        //        4                 5A           6B            7C
        /*C*/ {-3,0,7,6,6},    {1,1,9,4,6},  {2,2,12,4,4},  {3,3,10,6,4},
        //        8                 9A                         10C
        /*A*/ {-1,5,10,10,9},  {1,5,1,8,10},                {3,7,13,9,8},
        //        11                             12A           13C
        /*B*/ {-2,9,13,12,12},               {2,6,2,11,11}, {3,10,3,12,11},
              {INT_MIN,12,0,13,INT_MIN},
    };
    EXPECT_EQUAL(network.table_, headersCoverA);
    EXPECT_EQUAL(network.grid_, dlxCoverA);

    // We can just check it against our original array. All state should be returned to normal.
    network.uncoverCity(9);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}

STUDENT_TEST("Simple Ethene cover B, large item wipe out with D remaining an option.") {
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
    std::vector<Dx::DisasterLinks::cityHeader> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    std::vector<Dx::DisasterLinks::cityItem> dlxItems = {
        /* Network
         *
         *      A  B  C  D  E  F
         *   B     1     1  1  1
         *   D  1  1  1  1
         *   A  1        1
         *   C        1  1
         *   E     1        1
         *   F     1           1
         */

        //        0                  1A             2B               3C               4D            5E             6F
              {0,0,0,6,1},      {2,18,13,0,2},  {4,27,8,1,3},   {2,21,15,2,4},  {4,22,9,3,5},    {2,25,10,4,6},  {2,28,11,5,0},
        /*B*/ {-2,0,11,11,8},                   {2,2,14,7,9},                   {4,4,16,8,10},   {5,5,25,9,11},  {6,6,28,10,7},
        /*D*/ {-4,8,16,16,13},  {1,1,18,12,14}, {2,8,24,13,15}, {3,3,21,14,16}, {4,9,19,15,12},
        /*A*/ {-1,13,19,19,18}, {1,13,1,17,19},                                 {4,16,22,18,17},
        /*C*/ {-3,18,22,22,21},                                 {3,15,3,20,22}, {4,19,4,21,20},
        /*E*/ {-5,21,25,25,24},                 {2,14,27,23,25},                                 {5,10,5,24,23},
        /*F*/ {-6,24,28,28,27},                 {2,24,2,26,28},                                                  {6,11,6,27,26},
              {INT_MIN,27,0,28,INT_MIN},
    };

    Dx::DisasterLinks network (cities);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);

    std::string supplyLocation = network.coverCity(8);

    EXPECT_EQUAL(supplyLocation, "B");

    std::vector<Dx::DisasterLinks::cityHeader> headersCoverB = {
        {"",  3, 1},
        {"A", 0, 3},
        {"B", 1, 3},
        {"C", 1, 0},
        {"D", 3, 5},
        {"E", 3, 6},
        {"F", 3, 0}
    };

    std::vector<Dx::DisasterLinks::cityItem> dlxCoverB = {
        /* Network is now smaller but we can supply D option still even though D item is gone.
         *
         *      A  C
         *   A  1
         *   C     1
         *   D  1  1
         *   E
         *   F
         */

        //        0                  1A             2B               3C               4D                 5E             6F
              {0,0,0,3,1},      {2,18,13,0,3},  {4,27,8,1,3},    {2,21,15,1,0},  {4,22,9,3,5},    {2,25,10,3,6},  {2,28,11,3,0},
        /*B*/ {-2,0,11,11,8},                   {2,2,14,7,9},                    {4,4,16,8,10},   {5,5,25,9,11},  {6,6,28,10,7},
        /*D*/ {-4,8,16,15,13},  {1,1,18,12,15}, {2,8,24,13,15},  {3,3,21,13,12}, {4,9,19,15,12},
        /*A*/ {-1,13,19,18,18}, {1,13,1,17,17},                                  {4,16,22,18,17},
        /*C*/ {-3,18,22,21,21},                                  {3,15,3,20,20}, {4,19,4,21,20},
        /*E*/ {-5,21,25,23,23},                 {2,14,27,23,25},                                 {5,10,5,23,23},
        /*F*/ {-6,24,28,26,26},                 {2,24,2,26,28},                                                  {6,11,6,26,26},
              {INT_MIN,27,0,28,INT_MIN},
    };
    EXPECT_EQUAL(network.table_, headersCoverB);
    EXPECT_EQUAL(network.grid_, dlxCoverB);
    network.uncoverCity(8);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}


STUDENT_TEST("Simple Ethene cover A with option D.") {
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
    std::vector<Dx::DisasterLinks::cityHeader> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    std::vector<Dx::DisasterLinks::cityItem> dlxItems = {
        /* Network
         *
         *      A  B  C  D  E  F
         *   B     1     1  1  1
         *   D  1  1  1  1
         *   A  1        1
         *   C        1  1
         *   E     1        1
         *   F     1           1
         */

        //        0                  1A             2B               3C               4D            5E             6F
              {0,0,0,6,1},      {2,18,13,0,2},  {4,27,8,1,3},   {2,21,15,2,4},  {4,22,9,3,5},    {2,25,10,4,6},  {2,28,11,5,0},
        /*B*/ {-2,0,11,11,8},                   {2,2,14,7,9},                   {4,4,16,8,10},   {5,5,25,9,11},  {6,6,28,10,7},
        /*D*/ {-4,8,16,16,13},  {1,1,18,12,14}, {2,8,24,13,15}, {3,3,21,14,16}, {4,9,19,15,12},
        /*A*/ {-1,13,19,19,18}, {1,13,1,17,19},                                 {4,16,22,18,17},
        /*C*/ {-3,18,22,22,21},                                 {3,15,3,20,22}, {4,19,4,21,20},
        /*E*/ {-5,21,25,25,24},                 {2,14,27,23,25},                                 {5,10,5,24,23},
        /*F*/ {-6,24,28,28,27},                 {2,24,2,26,28},                                                  {6,11,6,27,26},
              {INT_MIN,27,0,28,INT_MIN},
    };

    Dx::DisasterLinks network (cities);

    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);

    std::string supplyLocation = network.coverCity(13);

    EXPECT_EQUAL(supplyLocation, "D");

    std::vector<Dx::DisasterLinks::cityHeader> headersCoverD = {
        {"",  6, 5},
        {"A", 0, 2},
        {"B", 0, 3},
        {"C", 0, 4},
        {"D", 0, 5},
        {"E", 0, 6},
        {"F", 5, 0}
    };

    std::vector<Dx::DisasterLinks::cityItem> dlxCoverD = {
        /* Network is now smaller but we can supply D option still even though D item is gone.
         *
         *      A  C
         *   D  1  1
         *   A  1
         *   C     1
         *   E
         *   F
         */

        //        0                  1A               2B             3C                4D             5E               6F
              {0,0,0,6,5},      {2,18,13,0,2},   {4,27,8,0,3},    {2,21,15,0,4},  {4,22,9,0,5},    {2,25,10,0,6},  {2,28,11,5,0},
        /*B*/ {-2,0,11,11,10},  {2,2,14,7,9},                                     {4,4,16,7,10},   {5,5,25,7,11},  {6,6,28,10,7},
        /*D*/ {-4,8,16,16,13},  {1,1,18,12,14},  {2,8,24,13,15},  {3,3,21,14,16}, {4,9,19,15,12},
        /*A*/ {-1,13,19,17,17}, {1,13,1,17,19},                                   {4,16,22,17,17},
        /*C*/ {-3,18,22,20,20},                                   {3,15,3,20,22}, {4,19,4,20,20},
        /*E*/ {-5,21,25,25,25},                  {2,14,27,23,25},                                  {5,10,5,23,23},
        /*F*/ {-6,24,28,28,28},                  {2,24,2,26,28},                                                   {6,11,6,26,26},
              {INT_MIN,27,0,28,INT_MIN},
    };
    EXPECT_EQUAL(network.table_, headersCoverD);
    EXPECT_EQUAL(network.grid_, dlxCoverD);

    network.uncoverCity(13);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}

STUDENT_TEST("Test for a depth 2 cover and uncover. Two covers then two uncovers.") {
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
    std::vector<Dx::DisasterLinks::cityHeader> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    std::vector<Dx::DisasterLinks::cityItem> dlxItems = {
        /* Network
         *
         *      A  B  C  D  E  F
         *   B     1     1  1  1
         *   D  1  1  1  1
         *   A  1        1
         *   C        1  1
         *   E     1        1
         *   F     1           1
         */

        //        0                  1A             2B               3C               4D            5E             6F
              {0,0,0,6,1},      {2,18,13,0,2},  {4,27,8,1,3},   {2,21,15,2,4},  {4,22,9,3,5},    {2,25,10,4,6},  {2,28,11,5,0},
        /*B*/ {-2,0,11,11,8},                   {2,2,14,7,9},                   {4,4,16,8,10},   {5,5,25,9,11},  {6,6,28,10,7},
        /*D*/ {-4,8,16,16,13},  {1,1,18,12,14}, {2,8,24,13,15}, {3,3,21,14,16}, {4,9,19,15,12},
        /*A*/ {-1,13,19,19,18}, {1,13,1,17,19},                                 {4,16,22,18,17},
        /*C*/ {-3,18,22,22,21},                                 {3,15,3,20,22}, {4,19,4,21,20},
        /*E*/ {-5,21,25,25,24},                 {2,14,27,23,25},                                 {5,10,5,24,23},
        /*F*/ {-6,24,28,28,27},                 {2,24,2,26,28},                                                  {6,11,6,27,26},
              {INT_MIN,27,0,28,INT_MIN},
    };
    Dx::DisasterLinks network (cities);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);

    std::string supplyLocation = network.coverCity(18);

    EXPECT_EQUAL(supplyLocation, "A");

    std::vector<Dx::DisasterLinks::cityHeader> headersCoverA = {
        {"",  6, 2},
        {"A", 0, 2},
        {"B", 0, 3},
        {"C", 2, 5},
        {"D", 3, 5},
        {"E", 3, 6},
        {"F", 5, 0}
    };

    std::vector<Dx::DisasterLinks::cityItem> dlxCoverA = {
        /* Network is now smaller.
         *
         *      B  C  E  F
         *   B  1     1  1
         *   D  1  1
         *   C     1
         *   E  1     1
         *   F  1        1
         */

        //         0                1A             2B               3C               4D                 5E             6F
              {0,0,0,6,2},      {2,18,13,0,2},  {4,27,8,0,3},   {2,21,15,2,5},  {4,22,9,3,5},    {2,25,10,3,6},  {2,28,11,5,0},
        /*B*/ {-2,0,11,11,8},                   {2,2,14,7,10},                  {4,4,16,8,10},   {5,5,25,8,11},  {6,6,28,10,7},
        /*D*/ {-4,8,16,15,14},  {1,1,18,12,14}, {2,8,24,12,15}, {3,3,21,14,12}, {4,9,19,15,12},
        /*A*/ {-1,13,19,19,18}, {1,13,1,17,19},                                 {4,16,22,18,17},
        /*C*/ {-3,18,22,21,21},                                 {3,15,3,20,20}, {4,19,4,21,20},
        /*E*/ {-5,21,25,25,24},                 {2,14,27,23,25},                                 {5,10,5,24,23},
        /*F*/ {-6,24,28,28,27},                 {2,24,2,26,28},                                                  {6,11,6,27,26},
        {INT_MIN,27,0,28,INT_MIN},
    };
    EXPECT_EQUAL(network.table_, headersCoverA);
    EXPECT_EQUAL(network.grid_, dlxCoverA);

    supplyLocation = network.coverCity(21);

    EXPECT_EQUAL(supplyLocation, "C");

    std::vector<Dx::DisasterLinks::cityHeader> headersCoverC = {
        {"",  6, 2},
        {"A", 0, 2},
        {"B", 0, 5},
        {"C", 2, 5},
        {"D", 3, 5},
        {"E", 2, 6},
        {"F", 5, 0}
    };

    std::vector<Dx::DisasterLinks::cityItem> dlxCoverC = {
        /* Second cover operation.
         *
         *      B  E  F
         *   B  1  1  1
         *   D  1
         *   E  1  1
         *   F  1     1
         */

        //        0                  1A             2B               3C               4D              5E          6F
              {0,0,0,6,2},      {2,18,13,0,2},  {4,27,8,0,5},    {2,21,15,2,5},  {4,22,9,3,5},   {2,25,10,2,6},  {2,28,11,5,0},
        /*B*/ {-2,0,11,11,8},                   {2,2,14,7,10},                   {4,4,16,8,10},  {5,5,25,8,11},  {6,6,28,10,7},
        /*D*/ {-4,8,16,14,14},  {1,1,18,12,14}, {2,8,24,12,12},  {3,3,21,14,12}, {4,9,19,15,12},
        /*A*/ {-1,13,19,19,18}, {1,13,1,17,19}, {4,16,22,18,17},
        /*C*/ {-3,18,22,21,21},                                  {3,15,3,20,20}, {4,19,4,21,20},
        /*E*/ {-5,21,25,25,24},                 {2,14,27,23,25},                                 {5,10,5,24,23},
        /*F*/ {-6,24,28,28,27},                 {2,24,2,26,28},                                                  {6,11,6,27,26},
        {INT_MIN,27,0,28,INT_MIN},
    };

    EXPECT_EQUAL(network.table_, headersCoverC);
    EXPECT_EQUAL(network.grid_, dlxCoverC);
    network.uncoverCity(21);
    EXPECT_EQUAL(network.table_, headersCoverA);
    EXPECT_EQUAL(network.grid_, dlxCoverA);
    network.uncoverCity(18);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);

}


/* * * * * * * * * * * * * * * * *       Test Disaster Supplies         * * * * * * * * * * * * * */


STUDENT_TEST("Supplying C will cover all.") {
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

    Dx::DisasterLinks network(cities);
    std::set<std::string> chosen = {};
    EXPECT(network.isDisasterReady(1, chosen));
}

STUDENT_TEST("Simple Ethene cover D and B to succeed.") {
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
    std::vector<Dx::DisasterLinks::cityHeader> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    std::vector<Dx::DisasterLinks::cityItem> dlxItems = {
        /* Network
         *
         *      A  B  C  D  E  F
         *   B     1     1  1  1
         *   D  1  1  1  1
         *   A  1        1
         *   C        1  1
         *   E     1        1
         *   F     1           1
         */

        //        0                  1A             2B               3C               4D            5E             6F
              {0,0,0,6,1},      {2,18,13,0,2},  {4,27,8,1,3},   {2,21,15,2,4},  {4,22,9,3,5},    {2,25,10,4,6},  {2,28,11,5,0},
        /*B*/ {-2,0,11,11,8},                   {2,2,14,7,9},                   {4,4,16,8,10},   {5,5,25,9,11},  {6,6,28,10,7},
        /*D*/ {-4,8,16,16,13},  {1,1,18,12,14}, {2,8,24,13,15}, {3,3,21,14,16}, {4,9,19,15,12},
        /*A*/ {-1,13,19,19,18}, {1,13,1,17,19},                                 {4,16,22,18,17},
        /*C*/ {-3,18,22,22,21},                                 {3,15,3,20,22}, {4,19,4,21,20},
        /*E*/ {-5,21,25,25,24},                 {2,14,27,23,25},                                 {5,10,5,24,23},
        /*F*/ {-6,24,28,28,27},                 {2,24,2,26,28},                                                  {6,11,6,27,26},
              {INT_MIN,27,0,28,INT_MIN},
    };
    Dx::DisasterLinks network(cities);
    std::set<std::string> chosen = {};
    EXPECT(!network.isDisasterReady(1, chosen));
    // Make sure that we cleanup our data structure between calls in case it is tested before destr.
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
    EXPECT(network.isDisasterReady(2, chosen));
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT_EQUAL(network.grid_, dlxItems);
}

STUDENT_TEST("Make sure data structure returns to original state after many calls in a row.") {
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
    std::vector<Dx::DisasterLinks::cityHeader> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    std::vector<Dx::DisasterLinks::cityItem> dlxItems = {
        /* Network
         *
         *      A  B  C  D  E  F
         *   B     1     1  1  1
         *   D  1  1  1  1
         *   A  1        1
         *   C        1  1
         *   E     1        1
         *   F     1           1
         */

        //        0                  1A             2B               3C               4D            5E             6F
              {0,0,0,6,1},      {2,18,13,0,2},  {4,27,8,1,3},   {2,21,15,2,4},  {4,22,9,3,5},    {2,25,10,4,6},  {2,28,11,5,0},
        /*B*/ {-2,0,11,11,8},                   {2,2,14,7,9},                   {4,4,16,8,10},   {5,5,25,9,11},  {6,6,28,10,7},
        /*D*/ {-4,8,16,16,13},  {1,1,18,12,14}, {2,8,24,13,15}, {3,3,21,14,16}, {4,9,19,15,12},
        /*A*/ {-1,13,19,19,18}, {1,13,1,17,19},                                 {4,16,22,18,17},
        /*C*/ {-3,18,22,22,21},                                 {3,15,3,20,22}, {4,19,4,21,20},
        /*E*/ {-5,21,25,25,24},                 {2,14,27,23,25},                                 {5,10,5,24,23},
        /*F*/ {-6,24,28,28,27},                 {2,24,2,26,28},                                                  {6,11,6,27,26},
              {INT_MIN,27,0,28,INT_MIN},
    };
    Dx::DisasterLinks network(cities);
    std::set<std::string> chosen = {};
    for (int i = 50; i >= 0; i--) {
        if (i < 2) {
            EXPECT(!network.isCovered(i, chosen));
        } else {
            EXPECT(network.isCovered(i, chosen));
        }
        chosen.clear();
        // No matter how many tests we do, the data structure should always restore itself.
        EXPECT_EQUAL(network.grid_, dlxItems);
        EXPECT_EQUAL(network.table_, networkHeaders);
    }
}

STUDENT_TEST("The straight line test. This will help us make sure we manage options correctly.") {
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
    std::vector<Dx::DisasterLinks::cityHeader> networkHeaders = {
        {"",  6, 1},
        {"A", 0, 2},
        {"B", 1, 3},
        {"C", 2, 4},
        {"D", 3, 5},
        {"E", 4, 6},
        {"F", 5, 0}
    };
    std::vector<Dx::DisasterLinks::cityItem> dlxItems = {
        //        0                 1A             2B               3C               4D              5E             6F
              {0,0,0,6,1},     {2,24,12,0,2},   {3,20,8,1,3},   {3,25,13,2,4},   {3,27,9,3,5},   {3,22,10,4,6},   {2,28,18,5,0},
        /*B*/ {-2,0,10,10,8},                   {2,2,16,7,9},                    {4,4,17,8,10},  {5,5,14,9,7},
        /*C*/ {-3,8,14,14,12},  {1,1,24,11,13},                 {3,3,21,12,14},                  {5,10,22,13,11},
        /*D*/ {-4,12,18,18,16},                 {2,8,20,15,17},                  {4,9,27,16,18},                  {6,6,28,17,15},
        /*E*/ {-5,16,22,22,20},                 {2,16,2,19,21}, {3,13,25,20,22},                 {5,14,5,21,19},
        /*A*/ {-1,20,25,25,24}, {1,12,1,23,25},                 {3,21,3,24,23},
        /*F*/ {-6,24,28,28,27},                                                  {4,17,4,26,28},                  {6,18,6,27,26},
        {INT_MIN,27,0,28,INT_MIN},
    };
    Dx::DisasterLinks network(cities);
    std::set<std::string> chosen = {};
    EXPECT(!network.isDisasterReady(1, chosen));
    EXPECT_EQUAL(network.grid_, dlxItems);
    EXPECT_EQUAL(network.table_, networkHeaders);
    EXPECT(network.isDisasterReady(2, chosen));
    EXPECT_EQUAL(network.grid_, dlxItems);
    EXPECT_EQUAL(network.table_, networkHeaders);
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
    Dx::DisasterLinks network(cities);
    std::set<std::string> chosen = {};
    EXPECT(!network.isDisasterReady(1, chosen));
    EXPECT(network.isDisasterReady(2, chosen));
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
    Dx::DisasterLinks network(cities);
    std::set<std::string> chosen = {};
    EXPECT(!network.isDisasterReady(1, chosen));
    EXPECT(!network.isDisasterReady(2, chosen));
    EXPECT(network.isDisasterReady(3, chosen));
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
    Dx::DisasterLinks network(cities);
    std::set<std::string> chosen = {};
    EXPECT(!network.isDisasterReady(1, chosen));
    EXPECT(!network.isDisasterReady(2, chosen));
    EXPECT(!network.isDisasterReady(3, chosen));
    EXPECT(!network.isDisasterReady(4, chosen));
    EXPECT(network.isDisasterReady(5, chosen));
}


PROVIDED_TEST("Can solve ethene example, regardless of ordering.") {
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

        Dx::DisasterLinks network(map);
        std::set<std::string> chosen = {};
        EXPECT(network.isDisasterReady(2, chosen));

        chosen.clear();

        EXPECT(!network.isDisasterReady(1, chosen));
    } while (next_permutation(cities.begin(), cities.end()));
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

        Dx::DisasterLinks network(map);
        std::set<std::string> chosen = {};
        EXPECT(network.isDisasterReady(2, chosen));

        EXPECT_EQUAL(chosen.size(), 2);
        EXPECT(chosen.count(cities[2]));
        EXPECT(chosen.count(cities[3]));

        chosen.clear();

        EXPECT(!network.isDisasterReady(1, chosen));
    } while (next_permutation(cities.begin(), cities.end()));
}

PROVIDED_TEST("Works for six cities in a line, regardless of order.") {
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

        Dx::DisasterLinks network(map);
        EXPECT(network.isDisasterReady(2, chosen));

        chosen.clear();

        EXPECT(!network.isDisasterReady(1, chosen));
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

        Dx::DisasterLinks network(map);
        EXPECT(network.isDisasterReady(2, chosen));

        EXPECT_EQUAL(chosen.size(), 2);
        EXPECT(chosen.count(cities[1]));
        EXPECT(chosen.count(cities[4]));

        chosen.clear();

        EXPECT(!network.isDisasterReady(1, chosen));
    } while (next_permutation(cities.begin(), cities.end()));
}

PROVIDED_TEST("Solves \"Don't be Greedy,\" regardless of ordering.") {
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

        Dx::DisasterLinks network(map);
        /* We should be able to cover everything with two cities. */
        EXPECT(network.isDisasterReady(2, chosen));

        chosen.clear();

        EXPECT(!network.isDisasterReady(1, chosen));
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

        Dx::DisasterLinks network(map);
        /* We should be able to cover everything with two cities. */
        EXPECT(network.isDisasterReady(2, chosen));

        /* Those cities should be 1 and 3. */
        EXPECT_EQUAL(chosen, { cities[1], cities[3] });

        chosen.clear();

        EXPECT(!network.isDisasterReady(1, chosen));
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

    Dx::DisasterLinks network(map);
    /* We should be able to cover everything with two cities. */
    EXPECT(network.isDisasterReady(16, chosen));

    /* Those cities should be 1 and 3. */
    EXPECT_EQUAL(chosen, { cities[1], cities[2], cities[3], cities[4], cities[5], cities[6],
                           cities[7],cities[8],cities[9],cities[10],cities[11],cities[12],
                           cities[13],cities[14],cities[15],cities[16], });

    chosen.clear();

    EXPECT(!network.isDisasterReady(15, chosen));
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

    Dx::DisasterLinks network(grid);
    std::set<std::string> locations;
    EXPECT(network.isDisasterReady(10, locations));
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

    Dx::DisasterLinks network(grid);
    std::set<std::string> locations;
    EXPECT(network.isDisasterReady(10, locations));

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
    Dx::DisasterLinks grid(cities);
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
    Dx::DisasterLinks grid(cities);
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
    Dx::DisasterLinks grid(cities);
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
