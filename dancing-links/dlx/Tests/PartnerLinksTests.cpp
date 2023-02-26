#include "Src/PartnerLinks.h"

namespace DancingLinks {

/* * * * * * * * * * * * * * * *   Overloaded Operators for Debugging   * * * * * * * * * * * * * */


bool operator==(const PartnerLinks::personLink& lhs, const PartnerLinks::personLink& rhs) {
    return lhs.topOrLen == rhs.topOrLen && lhs.up == rhs.up && lhs.down == rhs.down;
}

bool operator!=(const PartnerLinks::personLink& lhs, const PartnerLinks::personLink& rhs) {
    return !(lhs == rhs);
}

bool operator==(const PartnerLinks::personName& lhs, const PartnerLinks::personName& rhs) {
    return lhs.name == rhs.name && lhs.left == rhs.left && lhs.right == rhs.right;
}

bool operator!=(const PartnerLinks::personName& lhs, const PartnerLinks::personName& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, const PartnerLinks::personLink& person) {
    os << "{ topOrLen: " << person.topOrLen
       << ", up: " << person.up << ", down: " << person.down << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const PartnerLinks::personName& name) {
    os << "{ name: " << name.name << ", left: " << name.left << ", right: " << name.right << " }";
    return os;
}

std::ostream& operator<<(std::ostream&os, const std::vector<PartnerLinks::personLink>& links) {
    os << "DLX ARRAY" << std::endl;
    for (const auto& item : links) {
        if (item.topOrLen < 0) {
            os << std::endl;
        }
        os << "{" << item.topOrLen << "," << item.up << "," << item.down << "}";
    }
    os << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream&os, const std::vector<PartnerLinks::personName>& links) {
    os << "LOOKUP TABLE" << std::endl;
    for (const auto& item : links) {
        os << "{\"" << item.name << "\"," << item.left << "," << item.right << "}" << std::endl;
    }
    return os;
}

std::ostream& operator<<(std::ostream&os, const PartnerLinks& links) {
    os << "LOOKUP ARRAY" << std::endl;
    for (const auto& header : links.table_) {
        os << "{\"" << header.name << "\"," << header.left << "," << header.right << "},";
    }
    os << std::endl;
    os << "DLX ARRAY" << std::endl;
    for (const auto& item : links.links_) {
        if (item.topOrLen < 0) {
            os << std::endl;
        }
        os << "{" << item.topOrLen << "," << item.up << "," << item.down << "},";
    }
    os << std::endl;
    os << "Number of People: " << links.numPeople_ << std::endl;
    os << "Number of Pairs: " << links.numPairings_ << std::endl;
    os << "Has Singleton: " << links.hasSingleton_ << std::endl;
    os << "Is Weighted: " << links.isWeighted_ << std::endl;
    return os;
}

} // namespace DancingLinks

namespace Dx = DancingLinks;

/* * * * * * * * * * * * * * * *      Test Cases Below this Point       * * * * * * * * * * * * * */


/* * * * * * * * * * * * * * * *            WEIGHTED MATCHING           * * * * * * * * * * * * * */


/* * * * * * * * * * * * * * * *             Initialization             * * * * * * * * * * * * * */


STUDENT_TEST("Empty is empty.") {
    const std::map<std::string, std::map<std::string,int>> provided = {};
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",0,0}
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        {0,0,0},
        {INT_MIN,-1,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(dlxItems, matches.links_);
}

STUDENT_TEST("Weighted matching initializes straight line correctly.") {
    /*
     *          1    2     3     4      5
     *       C----D-----A-----F------B-----E
     *
     *
     */
    const std::map<std::string, std::map<std::string,int>> provided = {
        {"A", {{"D", 2}, {"F",3}}},
        {"B", {{"E", 5}, {"F",4}}},
        {"C", {{"D", 1}}},
        {"D", {{"A", 2}, {"C",1}}},
        {"E", {{"B", 5}}},
        {"F", {{"A", 3}, {"B",4}}},
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0}
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /* Standard Line. Two people on ends should have one connection.
         *      A  B  C  D  E  F
         *   1  1        1
         *   2  1              1
         *   3     1        1
         *   4     1           1
         *   5        1  1
         */

        //       0          1A       2B       3C         4D       5E        6F
        /*0*/ {0,0,0},   {2,11,8},{2,17,14},{1,20,20},{2,21,9},{1,15,15},{2,18,12},
        //       7          8A                           9D
        /*1*/ {-2,5,9},  {1,1,11},                    {4,4,21},
        //       10         11A                                             12F
        /*2*/ {-3,8,12}, {1,8,1},                                        {6,6,18},
        //       13                  14B                          15E
        /*3*/ {-5,11,15},         {2,2,17},                     {5,5,5},
        //       16                  17B                                    18F
        /*4*/ {-4,14,18},         {2,14,2},                              {6,12,6},
        //       19                           20C        21D
        /*5*/ {-1,17,21},                   {3,3,3},  {4,9,4},
        //       22
        /*6*/ {INT_MIN,20,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(dlxItems, matches.links_);
}

STUDENT_TEST("Weighted matching does not care about leaving others out.") {
    /* Here's the world:
     *                  10
     *                A --- B
     *                 \   /
     *                2 \ / 2
     *                   C
     *
     * There is no perfect matching here, unfortunately.
     */
    const std::map<std::string, std::map<std::string,int>> provided = {
        {"A", {{"B",10}, {"C",2}}},
        {"B", {{"A",10}, {"C",2}}},
        {"C", {{"A",2},{"B", 2}}},
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",3,1},{"A",0,2},{"B",1,3},{"C",2,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C
         *   1  1  1
         *   3  1     1
         *   2     1  1
         */


        //       0         1A       2B      3C
        /*0*/ {0,0,0},   {2,8,5}, {2,11,6},{2,12,9},
        //       4         5A       6B
        /*1*/ {-10,2,6}, {1,1,8}, {2,2,11},
        //       7         8A               9C
        /*2*/ {-2,5,9},  {1,5,1},          {3,3,12},
        //       10                 11A     12C
        /*3*/ {-2,8,12},          {2,6,2}, {3,9,3},
        //       13
        /*4*/ {INT_MIN,11,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);
}


/* * * * * * * * * * * * * * * *              Cover Logic               * * * * * * * * * * * * * */


STUDENT_TEST("Covering a person in weighted will only take that person's pairs out.") {
    /* Here's the world:
     *                  10
     *                A --- B
     *                 \   /
     *                2 \ / 2
     *                   C
     *
     * There is no perfect matching here, unfortunately.
     */
    const std::map<std::string, std::map<std::string,int>> provided = {
        {"A", {{"B",10}, {"C",2}}},
        {"B", {{"A",10}, {"C",2}}},
        {"C", {{"A",2},{"B", 2}}},
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",3,1},{"A",0,2},{"B",1,3},{"C",2,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C
         *   1  1  1
         *   3  1     1
         *   2     1  1
         */


        //       0         1A       2B      3C
        /*0*/ {0,0,0},   {2,8,5}, {2,11,6},{2,12,9},
        //       4         5A       6B
        /*1*/ {-10,2,6}, {1,1,8}, {2,2,11},
        //       7         8A               9C
        /*2*/ {-2,5,9},  {1,5,1},          {3,3,12},
        //       10                 11B     12C
        /*3*/ {-2,8,12},          {2,6,2}, {3,9,3},
        //       13
        /*4*/ {INT_MIN,11,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    matches.hidePerson(5);

    std::vector<Dx::PartnerLinks::personName> lookupCoverA {
        {"",3,2},{"A",0,2},{"B",0,3},{"C",2,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxCoverA {
        /*
         *       B C
         *    3  1 1
         */


        //       0         1A       2B      3C
        /*0*/ {0,0,0},   {2,8,5}, {1,11,11},{1,12,12},
        //       4         5A       6B
        /*1*/ {-10,2,6}, {1,1,8}, {2,2,11},
        //       7         8A               9C
        /*2*/ {-2,5,9},  {1,5,1},          {3,3,12},
        //       10                 11B     12C
        /*3*/ {-2,8,12},          {2,2,2}, {3,3,3},
        //       13
        /*4*/ {INT_MIN,11,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxCoverA);

    matches.unhidePerson(5);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

}

STUDENT_TEST("All weights are unique so we can know that we report the right weight and pair.") {
    /* Here's the world:
     *                  3
     *               A-----B
     *            4  |     |  6
     *               |     |
     *               C-----D
     *                  5
     * There is no perfect matching here, unfortunately.
     */
    const std::map<std::string, std::map<std::string,int>> provided = {
        {"A", {{"B", 3}, {"C",4}}},
        {"B", {{"A", 3}, {"D",6}}},
        {"C", {{"A", 4}, {"D",5}}},
        {"D", {{"B", 6}, {"C",5}}},
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D
         *   1  1  1
         *   2  1     1
         *   3     1     1
         *   4        1  1
         */
        //       0         1A      2B        3C        4D
        /*0*/ {0,0,0},   {2,9,6}, {2,12,7}, {2,15,10},{2,16,13},
        //       5         6A      7B
        /*1*/ {-3,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                10C
        /*2*/ {-4,6,10}, {1,6,1},          {3,3,15},
        //       11                12B                 13D
        /*3*/ {-6,9,13},          {2,7,2},            {4,4,16},
        //       14                          15C       16D
        /*4*/ {-5,12,16},                  {3,10,3}, {4,13,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    // Cover A, this will select option 2, partners are AB.
    std::pair<int,Pair> match = matches.coverWeightedPair(6);
    EXPECT_EQUAL(match.first, 3);
    EXPECT_EQUAL(match.second, {"A","B"});

    std::vector<Dx::PartnerLinks::personName> lookupCoverA {
        {"",4,3},{"A",0,2},{"B",0,3},{"C",0,4},{"D",3,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxCoverA {
        /* Smaller links with A covered via option 1.
         *      C  D
         *   4  1  1
         */

        //       0         1A       2B         3C        4D
        /*1*/ {0,0,0},   {2,9,6}, {2,12,7}, {1,15,15},{1,16,16},
        //       5         6A       7B
        /*2*/ {-3,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                 10C
        /*3*/ {-4,6,10}, {1,6,1},           {3,3,15},
        //       11                 12B                  13D
        /*4*/ {-6,9,13},          {2,7,2},            {4,4,16},
        //       14                           15C        16D
        /*5*/ {-5,12,16},                   {3,3,3},  {4,4,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);
}

/* * * * * * * * * * * * * * * *             Solve Problem              * * * * * * * * * * * * * */


PROVIDED_TEST("maximumWeightMatching: Works on a square.") {
    /* This world:
     *
     *         1
     *      A --- B
     *      |     |
     *    8 |     | 2
     *      |     |
     *      D --- C
     *         4
     *
     * Best option is to pick BC/AD.
     */
    const std::map<std::string, std::map<std::string,int>> provided = {
        {"A", {{"B",1}, {"D",8}}},
        {"B", {{"A",1}, {"C",2}}},
        {"C", {{"B",2},{"D", 4}}},
        {"D", {{"A",8},{"C", 4}}},
    };
    Dx::PartnerLinks weights(provided);

    EXPECT_EQUAL(weights.getMaxWeightMatching(), { {"A", "D"}, {"B", "C"} });
}

PROVIDED_TEST("maximumWeightMatching: Works on a line of four people.") {
    /* This world:
     *
     *  A --- B --- C --- D
     *     1     3     1
     *
     * Best option is to pick B -- C, even though this is not a perfect
     * matching.
     */
    const std::map<std::string, std::map<std::string,int>> links = {
        {"A", {{"B",1}}},
        {"B", {{"A",1}, {"C",3}}},
        {"C", {{"B",3},{"D",1}}},
        {"D", {{"C",1}}},
    };
    Dx::PartnerLinks weights(links);

    /* Should pick B--C. */
    EXPECT_EQUAL(weights.getMaxWeightMatching(), { {"B", "C"} });
}

PROVIDED_TEST("maximumWeightMatching: Works on a line of three people.") {
    /* This world:
     *
     *  A --- B --- C
     *     1     2
     *
     * Best option is to pick B -- C.
     */
    auto links = fromWeightedLinks({
        { "A", "B", 1 },
        { "B", "C", 2 },
    });

    Dx::PartnerLinks weights(links);
    /* Should pick B--C. */
    EXPECT_EQUAL(weights.getMaxWeightMatching(), { {"B", "C"} });
}

PROVIDED_TEST("maximumWeightMatching: Odd shap that requires us to pick opposite edges.") {
    /* Because std::map and std::set internally store items in sorted order, the order
     * in which you iterate over people when making decisions is sensitive
     * to the order of those peoples' names. This test looks at a group like
     * this one, trying out all possible orderings of peoples' names:
     *
     *               E
     *               | 1
     *               C
     *            1 / \ 5
     *             A---B
     *          1 /  1  \ 1
     *           F       D
     *
     * (Best option is to pick the 5-cost edge and the opposite-side 1-cost
     * edge.)
     *
     */
    const std::map<std::string, std::map<std::string,int>> links = {
        {"A", {{"B",1},{"C",1},{"F",1}}},
        {"B", {{"A",1},{"C",5},{"D",1}}},
        {"C", {{"A",1},{"B",5},{"E",1}}},
        {"D", {{"B",1}}},
        {"E", {{"C",1}}},
        {"F", {{"A",1}}},
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *     A  B  C  D  E  F
         *  1  1  1
         *  2  1     1
         *  3  1              1
         *  4     1  1
         *  5     1     1
         *  6        1     1
         *
         */

        //            1A       2B        3C       4D         5E       6F
        {0,0,0},   {3,14,8},{3,20,9},{3,23,12},{1,21,21},{1,24,24},{1,15,15},
        {-1,5,9},  {1,1,11},{2,2,17},
        {-1,8,12}, {1,8,14},         {3,3,18},
        {-1,11,15},{1,11,1},                                       {6,6,6},
        {-5,14,18},         {2,9,20},{3,12,23},
        {-1,17,21},         {2,17,2},          {4,4,4},
        {-1,20,24},                  {3,18,3},            {5,5,5},
        {-2147483648,23,-2147483648},

    };
    Dx::PartnerLinks weights(links);

    EXPECT_EQUAL(weights.getMaxWeightMatching(), {{"A","F"},{"B","C"}});
    EXPECT_EQUAL(weights.links_, dlxItems);
    EXPECT_EQUAL(weights.table_, lookup);
}

PROVIDED_TEST("maximumWeightMatching: Another permutation of the same shape is failing.") {
    /* Because std::map and std::set internally store items in sorted order, the order
     * in which you iterate over people when making decisions is sensitive
     * to the order of those peoples' names. This test looks at a group like
     * this one, trying out all possible orderings of peoples' names:
     *
     *               E
     *               | 1
     *               D
     *            1 / \ 5
     *             A---C
     *          1 /  1  \ 1
     *           F       B
     *
     * (Best option is to pick the 5-cost edge and the opposite-side 1-cost
     * edge.)
     *
     */
    const std::map<std::string, std::map<std::string,int>> links = {
        {"A", {{"C",1},{"D",1},{"F",1}}},
        {"B", {{"C",1}}},
        {"C", {{"A",1},{"B",1},{"D",5}}},
        {"D", {{"A",1},{"C",5},{"E",1}}},
        {"E", {{"D",1}}},
        {"F", {{"A",1}}},
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *     A  B  C  D  E  F
         *  1  1     1
         *  2  1        1
         *  3  1              1
         *  4     1  1
         *  5        1  1
         *  6           1  1
         *
         */

        //            1A       2B        3C       4D         5E       6F
        {0,0,0},   {3,14,8},{1,17,17},{3,20,9},{3,23,12},{1,24,24},{1,15,15},
        {-1,5,9},  {1,1,11},          {3,3,18},
        {-1,8,12}, {1,8,14},                   {4,4,21},
        {-1,11,15},{1,11,1},                                       {6,6,6},
        {-1,14,18},         {2,2,2},  {3,9,20},
        {-5,17,21},                   {3,18,3},{4,12,23},
        {-1,20,24},                            {4,21,4}, {5,5,5},
        {INT_MIN,23,INT_MIN},


    };
    Dx::PartnerLinks weights(links);

    EXPECT_EQUAL(weights.getMaxWeightMatching(), {{"A","F"},{"C","D"}});
    EXPECT_EQUAL(weights.links_, dlxItems);
    EXPECT_EQUAL(weights.table_, lookup);
}

PROVIDED_TEST("maximumWeightMatching: The network resets after every run.") {
    /* Because std::map and std::set internally store items in sorted order, the order
     * in which you iterate over people when making decisions is sensitive
     * to the order of those peoples' names. This test looks at a group like
     * this one, trying out all possible orderings of peoples' names:
     *
     *               E
     *               | 1
     *               C
     *            1 / \ 5
     *             A---B
     *          1 /  1  \ 1
     *           F       D
     *
     * (Best option is to pick the 5-cost edge and the opposite-side 1-cost
     * edge.)
     *
     */
    const std::map<std::string, std::map<std::string,int>> links = {
        {"A", {{"B",1},{"C",1},{"F",1}}},
        {"B", {{"A",1},{"C",5},{"D",1}}},
        {"C", {{"A",1},{"B",5},{"E",1}}},
        {"D", {{"B",1}}},
        {"E", {{"C",1}}},
        {"F", {{"A",1}}},
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *     A  B  C  D  E  F
         *  1  1  1
         *  1  1     1
         *  1  1              1
         *  5     1  1
         *  1     1     1
         *  1        1     1
         *
         */

        // 0          1A       2B        3C       4D         5E       6F
        {0,0,0},   {3,14,8},{3,20,9},{3,23,12},{1,21,21},{1,24,24},{1,15,15},
        // 7          8A       9B
        {-1,5,9},  {1,1,11},{2,2,17},
        // 10         11A                12C
        {-1,8,12}, {1,8,14},         {3,3,18},
        // 13         14A                                             15F
        {-1,11,15},{1,11,1},                                       {6,6,6},
        // 16                  17B       18C
        {-5,14,18},         {2,9,20},{3,12,23},
        // 19                  20B                21D
        {-1,17,21},         {2,17,2},          {4,4,4},
        // 22                            23C                24E
        {-1,20,24},                  {3,18,3},            {5,5,5},
        {INT_MIN,23,INT_MIN},
    };
    Dx::PartnerLinks weights(links);

    for (int i = 0; i < 11; i++) {
        EXPECT_EQUAL(weights.getMaxWeightMatching(), {{"A","F"},{"B","C"}});
        EXPECT_EQUAL(weights.links_, dlxItems);
        EXPECT_EQUAL(weights.table_, lookup);
    }

}

PROVIDED_TEST("maximumWeightMatching: Small stress test (should take at most a second or two).") {
    /* Because std::map and std::set internally store items in sorted order, the order
     * in which you iterate over people when making decisions is sensitive
     * to the order of those peoples' names. This test looks at a group like
     * this one, trying out all possible orderings of peoples' names:
     *
     *               *
     *               | 1
     *               *
     *            1 / \ 5
     *             *---*
     *          1 /  1  \ 1
     *           *       *
     *
     * (Best option is to pick the 5-cost edge and the opposite-side 1-cost
     * edge.)
     *
     * There are 6! = 720 possible permutations of the ordering of these
     * people. If your code is able to solve the problem correctly for all of
     * those orderings, there's a good chance that you're correctly tracking
     * who is matched at each step. On the other hand, if your code runs into
     * issues here, it may indicate that there's a bug in how you mark who's
     * paired and who isn't.
     */
    std::vector<std::string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        auto links = fromWeightedLinks({
            { people[0], people[1], 5 },
            { people[1], people[2], 1 },
            { people[2], people[0], 1 },
            { people[3], people[0], 1 },
            { people[4], people[1], 1 },
            { people[5], people[2], 1 },
        });

        std::set<Pair> expected = {
            { people[0], people[1] },
            { people[2], people[5] }
        };

        Dx::PartnerLinks weights(links);
        EXPECT_EQUAL(weights.getMaxWeightMatching(), expected);
    } while (next_permutation(people.begin(), people.end()));
}

PROVIDED_TEST("maximumWeightMatching: Large stress test (should take at most a second or two).") {
    /* Here, we're giving a chain of people, like this:
     *
     *    *---*---*---*---*---*---*---* ... *---*
     *      1   1   1   1   1   1   1         1
     *
     * The number of different matchings in a chain of n people is given by the
     * nth Fibonacci number. (Great exercise - can you explain why?) This means
     * that if we have a chain of 21 people, there are F(21) = 10,946 possible
     * matchings to check. If your program tests every single one of them exactly
     * once, then it should be pretty quick to determine what the best matching
     * here is. (It's any matching that uses exactly floor(21 / 2) = 10 edges.
     *
     * On the other hand, if your implementation repeatedly constructs the same
     * matchings over and over again, then the number of options you need to consider
     * will be too large for your computer to handle in any reasonable time.
     *
     * If you're passing the other tests and this test hangs, double-check your
     * code to make sure you aren't repeatedly constructing the same matchings
     * multiple times.
     */
    const int kNumPeople = 21;
    std::vector<WeightedLink> links;
    for (int i = 0; i < kNumPeople - 1; i++) {
        links.push_back({ std::to_string(i), std::to_string(i + 1), 1 });
    }

    Dx::PartnerLinks weighted(fromWeightedLinks(links));

    auto matching = weighted.getMaxWeightMatching();
    EXPECT_EQUAL(matching.size(), kNumPeople / 2);

    /* Confirm it's a matching. */
    std::set<std::string> used;
    for (Pair p: matching) {
        /* No people paired more than once. */
        EXPECT(!used.count(p.first()));
        EXPECT(!used.count(p.second()));
        used.insert(p.first());
        used.insert(p.second());

        /* Must be a possible links. */
        EXPECT_EQUAL(abs(stringToInteger(p.first()) - stringToInteger(p.second())), 1);
    }
}



/* * * * * * * * * * * * * * * *            PERFECT MATCHING            * * * * * * * * * * * * * */


/* * * * * * * * * * * * * * * *             Initialization             * * * * * * * * * * * * * */


STUDENT_TEST("Empty is empty perfect matching.") {
    const std::map<std::string, std::set<std::string>> provided = {};
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",0,0}
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        {0,0,0},
        {INT_MIN,-1,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(dlxItems, matches.links_);
}

STUDENT_TEST("Line of six but tricky due to natural order.") {
    /*
     *
     *       C--D--A--F--B--E
     *
     *
     */
    const std::map<std::string, std::set<std::string>> provided = {
        {"A", {"D", "F"}},
        {"B", {"E", "F"}},
        {"C", {"D"}},
        {"D", {"A", "C"}},
        {"E", {"B"}},
        {"F", {"A", "B"}},
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0}
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /* Standard Line. Two people on ends should have one connection.
         *      A  B  C  D  E  F
         *   1  1        1
         *   2  1              1
         *   3     1        1
         *   4     1           1
         *   5        1  1
         */

        //       0          1A       2B       3C         4D       5E        6F
        /*0*/ {0,0,0},   {2,11,8},{2,17,14},{1,20,20},{2,21,9},{1,15,15},{2,18,12},
        //       7          8A                           9D
        /*1*/ {-1,5,9},  {1,1,11},                    {4,4,21},
        //       10         11A                                             12F
        /*2*/ {-2,8,12}, {1,8,1},                                        {6,6,18},
        //       13                  14B                          15E
        /*3*/ {-3,11,15},         {2,2,17},                     {5,5,5},
        //       16                  17B                                    18F
        /*4*/ {-4,14,18},         {2,14,2},                              {6,12,6},
        //       19                           20C        21D
        /*5*/ {-5,17,21},                   {3,3,3},  {4,9,4},
        //       22
        /*6*/ {INT_MIN,20,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(dlxItems, matches.links_);
}

STUDENT_TEST("We will allow setup of worlds that are impossible to match.") {
    /* Here's the world:
     *
     *               A --- B
     *                \   /
     *                 \ /
     *                  C
     *
     * There is no perfect matching here, unfortunately.
     */
    const std::map<std::string, std::set<std::string>> provided = {
        { "A", {"B"} },
        { "B", {"C"} },
        { "C", {"A"} }
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",3,1},{"A",0,2},{"B",1,3},{"C",2,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C
         *   1  1  1
         *   2     1  1
         *   3  1     1
         */


        //       0         1A       2B      3C
        /*0*/ {0,0,0},   {2,11,5},{2,8,6},{2,12,9},
        //       4         5A       6B
        /*1*/ {-1,2,6},  {1,1,11},{2,2,8},
        //       7                  8B      9C
        /*2*/ {-2,5,9},           {2,6,2},{3,3,12},
        //       10        11A              12C
        /*3*/ {-3,8,12}, {1,5,1},         {3,9,3},
        //       13
        /*4*/ {INT_MIN,11,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);
}

STUDENT_TEST("Initialize a world that will have matching.") {
    /* Here's the world:
     *
     *               C --- B
     *             /        \
     *            E          F
     *             \        /
     *               D --- A
     *
     */
    const std::map<std::string, std::set<std::string>> provided = {
        { "A", {"D", "F"} },
        { "B", {"C", "F"} },
        { "C", {"B", "E"} },
        { "D", {"A", "E"} },
        { "E", {"C", "D"} },
        { "F", {"A", "B"} }
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D  E  F
         *   1  1        1
         *   2  1              1
         *   3     1  1
         *   4     1           1
         *   5        1     1
         *   6           1  1
         */

        //       0          1A       2B       3C         4D       5E        6F
        /*0*/ {0,0,0},   {2,11,8},{2,17,14},{2,20,15},{2,23,9},{2,24,21},{2,18,12},
        //       7          8A                           9D
        /*1*/ {-1,5,9},  {1,1,11},                    {4,4,23},
        //       10         11A                                             12F
        /*2*/ {-2,8,12}, {1,8,1},                                        {6,6,18},
        //       13                  14B      15C
        /*3*/ {-3,11,15},         {2,2,17}, {3,3,20},
        //       16                  17B                                    18F
        /*4*/ {-4,14,18},         {2,14,2},                              {6,12,6},
        //       19                           20C                 21E
        /*5*/ {-5,17,21},                   {3,15,3},          {5,5,24},
        //       22                                      23D      24E
        /*6*/ {-6,20,24},                             {4,9,4}, {5,21,5},
        //       25
        /*7*/ {INT_MIN,23,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(dlxItems, matches.links_);
}

STUDENT_TEST("std::setup works on a disconnected hexagon of people and reportes singleton.") {
    /* Here's the world:
     *
     *               C --- B
     *             /        \
     *            E          F
     *             \
     *               D     A
     *
     *
     *
     */
    const std::map<std::string, std::set<std::string>> provided = {
        { "A", {} },
        { "B", {"C", "F"} },
        { "C", {"B", "E"} },
        { "D", {"E"} },
        { "E", {"C", "D"} },
        { "F", {"B"} }
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         * None-v
         *      A  B  C  D  E  F
         *   1     1  1
         *   2     1           1
         *   3        1     1
         *   4           1  1
         */


        //      NO MATCHES--v
        //       0          1A       2B       3C       4D        5E        6F
        /*0*/ {0,0,0},   {0,1,1}, {2,11,8},{2,14,9},{1,17,17},{2,18,15},{1,12,12},
        /*1*/ {-1,5,9},           {2,2,11},{3,3,14},
        /*2*/ {-2,8,12},          {2,8,2},                              {6,6,6},
        /*3*/ {-3,11,15},                  {3,9,3},           {5,5,18},
        /*4*/ {-4,14,18},                           {4,4,4},  {5,15,5},
        /*6*/ {INT_MIN,17,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT(matches.hasSingleton_);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(dlxItems, matches.links_);
}


/* * * * * * * * * * * * * * * *              Cover Logic               * * * * * * * * * * * * * */


STUDENT_TEST("Simple square any valid partners will work. Cover A.") {
    /* Here's the world:
     *
     *               A-----B
     *               |     |
     *               |     |
     *               C-----D
     *
     * There is no perfect matching here, unfortunately.
     */
    const std::map<std::string, std::set<std::string>> provided = {
        { "A",{"B","C"}},
        { "B",{"A","D"}},
        { "C",{"A","D"}},
        { "D",{"C","B"}},
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D
         *   1  1  1
         *   2  1     1
         *   3     1     1
         *   4        1  1
         */
        //       0         1A      2B        3C        4D
        /*0*/ {0,0,0},   {2,9,6}, {2,12,7}, {2,15,10},{2,16,13},
        //       5         6A      7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                10C
        /*2*/ {-2,6,10}, {1,6,1},          {3,3,15},
        //       11                12B                 13D
        /*3*/ {-3,9,13},          {2,7,2},            {4,4,16},
        //       14                          15C       16D
        /*4*/ {-4,12,16},                  {3,10,3}, {4,13,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    // Cover A, this will select option 2, partners are AB.
    Pair match = matches.coverPairing(6);
    EXPECT_EQUAL(match, {"A","B"});

    std::vector<Dx::PartnerLinks::personName> lookupCoverA {
        {"",4,3},{"A",0,2},{"B",0,3},{"C",0,4},{"D",3,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxCoverA {
        /* Smaller links with A covered via option 1.
         *      C  D
         *   4  1  1
         */

        //       0         1A       2B         3C        4D
        /*1*/ {0,0,0},   {2,9,6}, {2,12,7}, {1,15,15},{1,16,16},
        //       5         6A       7B
        /*2*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                 10C
        /*3*/ {-2,6,10}, {1,6,1},           {3,3,15},
        //       11                 12B                  13D
        /*4*/ {-3,9,13},          {2,7,2},            {4,4,16},
        //       14                           15C        16D
        /*5*/ {-4,12,16},                   {3,3,3},  {4,4,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);
}

STUDENT_TEST("There are no perfect pairings, any matching will fail.") {
    /* Here's the world:
     *
     *               A --- B
     *                \   /
     *                 \ /
     *                  C
     *
     * There is no perfect matching here, unfortunately.
     */
    const std::map<std::string, std::set<std::string>> provided = {
        { "A", {"B"} },
        { "B", {"C"} },
        { "C", {"A"} }
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",3,1},{"A",0,2},{"B",1,3},{"C",2,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C
         *   1  1  1
         *   2     1  1
         *   3  1     1
         */


        //       0         1A       2B      3C
        /*0*/ {0,0,0},   {2,11,5},{2,8,6},{2,12,9},
        //       4         5A       6B
        /*1*/ {-1,2,6},  {1,1,11},{2,2,8},
        //       7                  8B      9C
        /*2*/ {-2,5,9},           {2,6,2},{3,3,12},
        //       10        11A              12C
        /*3*/ {-3,8,12}, {1,5,1},         {3,9,3},
        //       13
        /*4*/ {INT_MIN,11,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    Pair match = matches.coverPairing(5);
    EXPECT_EQUAL(match, {"A", "B"});
    std::vector<Dx::PartnerLinks::personName> lookupA {
        {"",3,3},{"A",0,2},{"B",0,3},{"C",0,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxCoverA {
        /*
         * C is left but there are no available options for C.
         *
         *      C
         *    0
         */


        //       0         1A       2B      3C
        /*0*/ {0,0,0},   {2,11,5},{2,8,6},{0,3,3},
        //       4         5A       6B
        /*1*/ {-1,2,6},  {1,1,11},{2,2,8},
        //       7                  8B      9C
        /*2*/ {-2,5,9},           {2,6,2},{3,3,3},
        //       10        11A              12C
        /*3*/ {-3,8,12}, {1,5,1},         {3,9,3},
        //       13
        /*4*/ {INT_MIN,11,INT_MIN},
    };
    EXPECT_EQUAL(lookupA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);
}

STUDENT_TEST("We will quickly learn that A-B is a bad pairing that leaves C out.") {
    /* Here's the world:
     *
     *               A --- B---C
     *                \   /
     *                 \ /
     *                  D
     *
     * There is no perfect matching here, unfortunately.
     */
    const std::map<std::string, std::set<std::string>> provided = {
        { "A", {"B", "D"} },
        { "B", {"A","C","D"} },
        { "C", {"B"} },
        { "D", {"A","B"}},
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0}
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D
         *   1  1  1
         *   2  1        1
         *   3     1  1
         *   4     1     1
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {3,15,7},{1,13,13},{2,16,10},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                    {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,7,15},{3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},          {4,10,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    Pair match = matches.coverPairing(6);
    EXPECT_EQUAL(match, {"A", "B"});
    std::vector<Dx::PartnerLinks::personName> lookupCoverA {
        {"",4,3},{"A",0,2},{"B",0,3},{"C",0,4},{"D",3,0}
    };
    std::vector<Dx::PartnerLinks::personLink> dlxCoverA {
        /*
         * We cannot reach C and D if we pair A and B.
         *
         *      C  D
         *   0
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {3,15,7},{0,3,3}, {0,4,4},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                   {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,7,15},{3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},         {4,4,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);
}

STUDENT_TEST("A-D is a good pairing.") {
    /* Here's the world:
     *
     *               A --- B---C
     *                \   /
     *                 \ /
     *                  D
     *
     * There is no perfect matching here, unfortunately.
     */
    const std::map<std::string, std::set<std::string>> provided = {
        { "A", {"B", "D"} },
        { "B", {"A","C","D"} },
        { "C", {"B"} },
        { "D", {"A","B"}},
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0}
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D
         *   1  1  1
         *   2  1        1
         *   3     1  1
         *   4     1     1
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {3,15,7},{1,13,13},{2,16,10},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                    {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,7,15},{3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},          {4,10,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    Pair match = matches.coverPairing(9);
    EXPECT_EQUAL(match, {"A", "D"});
    std::vector<Dx::PartnerLinks::personName> lookupCoverA {
        {"",3,2},{"A",0,2},{"B",0,3},{"C",2,0},{"D",3,0}
    };
    std::vector<Dx::PartnerLinks::personLink> dlxCoverA {
        /*
         *      B  C
         *   3  1  1
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {1,12,12},{1,13,13},{2,16,10},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                     {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,2,2},  {3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},           {4,10,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);
}

STUDENT_TEST("Cover A in a world where everyone has two connections.") {
    /* Here's the world:
     *
     *               C --- B
     *             /        \
     *            E          F
     *             \        /
     *               D --- A
     *
     */
    const std::map<std::string, std::set<std::string>> provided = {
        { "A", {"D", "F"} },
        { "B", {"C", "F"} },
        { "C", {"B", "E"} },
        { "D", {"A", "E"} },
        { "E", {"C", "D"} },
        { "F", {"A", "B"} }
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D  E  F
         *   1  1        1
         *   2  1              1
         *   3     1  1
         *   4     1           1
         *   5        1     1
         *   6           1  1
         */

        //       0          1A       2B       3C         4D       5E        6F
        /*0*/ {0,0,0},   {2,11,8},{2,17,14},{2,20,15},{2,23,9},{2,24,21},{2,18,12},
        //       7          8A                           9D
        /*1*/ {-1,5,9},  {1,1,11},                    {4,4,23},
        //       10         11A                                             12F
        /*2*/ {-2,8,12}, {1,8,1},                                        {6,6,18},
        //       13                  14B      15C
        /*3*/ {-3,11,15},         {2,2,17}, {3,3,20},
        //       16                  17B                                    18F
        /*4*/ {-4,14,18},         {2,14,2},                              {6,12,6},
        //       19                           20C                 21E
        /*5*/ {-5,17,21},                   {3,15,3},          {5,5,24},
        //       22                                      23D      24E
        /*6*/ {-6,20,24},                             {4,9,4}, {5,21,5},
        //       25
        /*7*/ {INT_MIN,23,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(dlxItems, matches.links_);

    Pair match = matches.coverPairing(8);
    EXPECT_EQUAL(match, {"A","D"});

    std::vector<Dx::PartnerLinks::personName> lookupCoverA {
        {"",6,2},{"A",0,2},{"B",0,3},{"C",2,5},{"D",3,5},{"E",3,6},{"F",5,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxCoverA {
        /*
         *      B  C  E  F
         *   3  1  1
         *   4  1        1
         *   5     1  1
         */

        //       0          1A       2B       3C         4D       5E        6F
        /*0*/ {0,0,0},   {2,11,8},{2,17,14},{2,20,15},{2,23,9},{1,21,21},{1,18,18},
        //       7          8A                           9D
        /*1*/ {-1,5,9},  {1,1,11},                    {4,4,23},
        //       10         11A                                             12F
        /*2*/ {-2,8,12}, {1,8,1},                                        {6,6,18},
        //       13                  14B      15C
        /*3*/ {-3,11,15},         {2,2,17}, {3,3,20},
        //       16                  17B                                    18F
        /*4*/ {-4,14,18},         {2,14,2},                              {6,6,6},
        //       19                           20C                 21E
        /*5*/ {-5,17,21},                   {3,15,3},          {5,5,5},
        //       22                                      23D      24E
        /*6*/ {-6,20,24},                             {4,9,4}, {5,21,5},
        //       25
        /*7*/ {INT_MIN,23,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);
}

STUDENT_TEST("A-D then B-C solves the world.") {
    /* Here's the world:
     *
     *               A --- B---C
     *                \   /
     *                 \ /
     *                  D
     *
     * There is no perfect matching here, unfortunately.
     */
    const std::map<std::string, std::set<std::string>> provided = {
        { "A", {"B", "D"} },
        { "B", {"A","C","D"} },
        { "C", {"B"} },
        { "D", {"A","B"}},
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0}
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D
         *   1  1  1
         *   2  1        1
         *   3     1  1
         *   4     1     1
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {3,15,7},{1,13,13},{2,16,10},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                    {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,7,15},{3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},          {4,10,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    Pair match = matches.coverPairing(9);
    EXPECT_EQUAL(match, {"A", "D"});
    std::vector<Dx::PartnerLinks::personName> lookupCoverA {
        {"",3,2},{"A",0,2},{"B",0,3},{"C",2,0},{"D",3,0}
    };
    std::vector<Dx::PartnerLinks::personLink> dlxCoverA {
        /*
         *      B  C
         *   3  1  1
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {1,12,12},{1,13,13},{2,16,10},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                     {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,2,2},  {3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},           {4,10,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);

    match = matches.coverPairing(12);
    EXPECT_EQUAL(match, {"B", "C"});
    std::vector<Dx::PartnerLinks::personName> lookupCoverB {
        {"",0,0},{"A",0,2},{"B",0,3},{"C",0,0},{"D",3,0}
    };
    std::vector<Dx::PartnerLinks::personLink> dlxCoverB {
        /*
         * Empty world is solved.
         *
         *     0
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {1,12,12},{1,13,13},{2,16,10},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                     {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,2,2},  {3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},           {4,10,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverB, matches.table_);
    EXPECT_EQUAL(dlxCoverB, matches.links_);
}


/* * * * * * * * * * * * * * * *         Cover/Uncover Logic            * * * * * * * * * * * * * */


STUDENT_TEST("Simple square any valid partners will work. Cover A then uncover.") {
    /* Here's the world:
     *
     *               A-----B
     *               |     |
     *               |     |
     *               C-----D
     *
     * There is no perfect matching here, unfortunately.
     */
    const std::map<std::string, std::set<std::string>> provided = {
        { "A",{"B","C"}},
        { "B",{"A","D"}},
        { "C",{"A","D"}},
        { "D",{"C","B"}},
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D
         *   1  1  1
         *   2  1     1
         *   3     1     1
         *   4        1  1
         */
        //       0         1A      2B        3C        4D
        /*0*/ {0,0,0},   {2,9,6}, {2,12,7}, {2,15,10},{2,16,13},
        //       5         6A      7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                10C
        /*2*/ {-2,6,10}, {1,6,1},          {3,3,15},
        //       11                12B                 13D
        /*3*/ {-3,9,13},          {2,7,2},            {4,4,16},
        //       14                          15C       16D
        /*4*/ {-4,12,16},                  {3,10,3}, {4,13,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    // Cover A, partners are AB.
    Pair match = matches.coverPairing(6);
    EXPECT_EQUAL(match, {"A","B"});

    std::vector<Dx::PartnerLinks::personName> lookupCoverA {
        {"",4,3},{"A",0,2},{"B",0,3},{"C",0,4},{"D",3,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxCoverA {
        /* Smaller links with A covered via option 1.
         *      C  D
         *   4  1  1
         */

        //       0         1A       2B         3C        4D
        /*1*/ {0,0,0},   {2,9,6}, {2,12,7}, {1,15,15},{1,16,16},
        //       5         6A       7B
        /*2*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                 10C
        /*3*/ {-2,6,10}, {1,6,1},           {3,3,15},
        //       11                 12B                  13D
        /*4*/ {-3,9,13},          {2,7,2},            {4,4,16},
        //       14                           15C        16D
        /*5*/ {-4,12,16},                   {3,3,3},  {4,4,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);

    matches.uncoverPairing(6);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

}

STUDENT_TEST("There are no perfect pairings, any matching will fail. Cover uncover.") {
    /* Here's the world:
     *
     *               A --- B
     *                \   /
     *                 \ /
     *                  C
     *
     * There is no perfect matching here, unfortunately.
     */
    const std::map<std::string, std::set<std::string>> provided = {
        { "A", {"B"} },
        { "B", {"C"} },
        { "C", {"A"} }
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",3,1},{"A",0,2},{"B",1,3},{"C",2,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C
         *   1  1  1
         *   2     1  1
         *   3  1     1
         */


        //       0         1A       2B      3C
        /*0*/ {0,0,0},   {2,11,5},{2,8,6},{2,12,9},
        //       4         5A       6B
        /*1*/ {-1,2,6},  {1,1,11},{2,2,8},
        //       7                  8B      9C
        /*2*/ {-2,5,9},           {2,6,2},{3,3,12},
        //       10        11A              12C
        /*3*/ {-3,8,12}, {1,5,1},         {3,9,3},
        //       13
        /*4*/ {INT_MIN,11,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    Pair match = matches.coverPairing(5);
    EXPECT_EQUAL(match, {"A", "B"});
    std::vector<Dx::PartnerLinks::personName> lookupA {
        {"",3,3},{"A",0,2},{"B",0,3},{"C",0,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxCoverA {
        /*
         * C is left but there are no available options for C.
         *
         *      C
         *    0
         */


        //       0         1A       2B      3C
        /*0*/ {0,0,0},   {2,11,5},{2,8,6},{0,3,3},
        //       4         5A       6B
        /*1*/ {-1,2,6},  {1,1,11},{2,2,8},
        //       7                  8B      9C
        /*2*/ {-2,5,9},           {2,6,2},{3,3,3},
        //       10        11A              12C
        /*3*/ {-3,8,12}, {1,5,1},         {3,9,3},
        //       13
        /*4*/ {INT_MIN,11,INT_MIN},
    };
    EXPECT_EQUAL(lookupA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);

    matches.uncoverPairing(5);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);
}

STUDENT_TEST("We will quickly learn that A-B is a bad pairing that leaves C out. Uncover.") {
    /* Here's the world:
     *
     *               A --- B---C
     *                \   /
     *                 \ /
     *                  D
     *
     * There is no perfect matching here, unfortunately.
     */
    const std::map<std::string, std::set<std::string>> provided = {
        { "A", {"B", "D"} },
        { "B", {"A","C","D"} },
        { "C", {"B"} },
        { "D", {"A","B"}},
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0}
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D
         *   1  1  1
         *   2  1        1
         *   3     1  1
         *   4     1     1
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {3,15,7},{1,13,13},{2,16,10},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                    {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,7,15},{3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},          {4,10,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    Pair match = matches.coverPairing(6);
    EXPECT_EQUAL(match, {"A", "B"});
    std::vector<Dx::PartnerLinks::personName> lookupCoverA {
        {"",4,3},{"A",0,2},{"B",0,3},{"C",0,4},{"D",3,0}
    };
    std::vector<Dx::PartnerLinks::personLink> dlxCoverA {
        /*
         * We cannot reach C and D if we pair A and B.
         *
         *      C  D
         *   0
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {3,15,7},{0,3,3}, {0,4,4},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                   {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,7,15},{3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},         {4,4,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);
}

STUDENT_TEST("A-D is a good pairing. Cover then uncover.") {
    /* Here's the world:
     *
     *               A --- B---C
     *                \   /
     *                 \ /
     *                  D
     *
     * There is no perfect matching here, unfortunately.
     */
    const std::map<std::string, std::set<std::string>> provided = {
        { "A", {"B", "D"} },
        { "B", {"A","C","D"} },
        { "C", {"B"} },
        { "D", {"A","B"}},
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0}
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D
         *   1  1  1
         *   2  1        1
         *   3     1  1
         *   4     1     1
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {3,15,7},{1,13,13},{2,16,10},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                    {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,7,15},{3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},          {4,10,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);

    Pair match = matches.coverPairing(9);
    EXPECT_EQUAL(match, {"A", "D"});
    std::vector<Dx::PartnerLinks::personName> lookupCoverA {
        {"",3,2},{"A",0,2},{"B",0,3},{"C",2,0},{"D",3,0}
    };
    std::vector<Dx::PartnerLinks::personLink> dlxCoverA {
        /*
         *      B  C
         *   3  1  1
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {1,12,12},{1,13,13},{2,16,10},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,12},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                     {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,2,2},  {3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},           {4,10,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);

    matches.uncoverPairing(9);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);
}

STUDENT_TEST("Cover A in a world where everyone has two connections then uncover.") {
    /* Here's the world:
     *
     *               C --- B
     *             /        \
     *            E          F
     *             \        /
     *               D --- A
     *
     */
    const std::map<std::string, std::set<std::string>> provided = {
        { "A", {"D", "F"} },
        { "B", {"C", "F"} },
        { "C", {"B", "E"} },
        { "D", {"A", "E"} },
        { "E", {"C", "D"} },
        { "F", {"A", "B"} }
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D  E  F
         *   1  1        1
         *   2  1              1
         *   3     1  1
         *   4     1           1
         *   5        1     1
         *   6           1  1
         */

        //       0          1A       2B       3C         4D       5E        6F
        /*0*/ {0,0,0},   {2,11,8},{2,17,14},{2,20,15},{2,23,9},{2,24,21},{2,18,12},
        //       7          8A                           9D
        /*1*/ {-1,5,9},  {1,1,11},                    {4,4,23},
        //       10         11A                                             12F
        /*2*/ {-2,8,12}, {1,8,1},                                        {6,6,18},
        //       13                  14B      15C
        /*3*/ {-3,11,15},         {2,2,17}, {3,3,20},
        //       16                  17B                                    18F
        /*4*/ {-4,14,18},         {2,14,2},                              {6,12,6},
        //       19                           20C                 21E
        /*5*/ {-5,17,21},                   {3,15,3},          {5,5,24},
        //       22                                      23D      24E
        /*6*/ {-6,20,24},                             {4,9,4}, {5,21,5},
        //       25
        /*7*/ {INT_MIN,23,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(dlxItems, matches.links_);

    Pair match = matches.coverPairing(8);
    EXPECT_EQUAL(match, {"A","D"});

    std::vector<Dx::PartnerLinks::personName> lookupCoverA {
        {"",6,2},{"A",0,2},{"B",0,3},{"C",2,5},{"D",3,5},{"E",3,6},{"F",5,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxCoverA {
        /*
         *      B  C  E  F
         *   3  1  1
         *   4  1        1
         *   5     1  1
         */

        //       0          1A       2B       3C         4D       5E        6F
        /*0*/ {0,0,0},   {2,11,8},{2,17,14},{2,20,15},{2,23,9},{1,21,21},{1,18,18},
        //       7          8A                           9D
        /*1*/ {-1,5,9},  {1,1,11},                    {4,4,23},
        //       10         11A                                             12F
        /*2*/ {-2,8,12}, {1,8,1},                                        {6,6,18},
        //       13                  14B      15C
        /*3*/ {-3,11,15},         {2,2,17}, {3,3,20},
        //       16                  17B                                    18F
        /*4*/ {-4,14,18},         {2,14,2},                              {6,6,6},
        //       19                           20C                 21E
        /*5*/ {-5,17,21},                   {3,15,3},          {5,5,5},
        //       22                                      23D      24E
        /*6*/ {-6,20,24},                             {4,9,4}, {5,21,5},
        //       25
        /*7*/ {INT_MIN,23,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);

    matches.uncoverPairing(8);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(matches.links_, dlxItems);
}

STUDENT_TEST("Depth two cover and uncover. Cover A then B then uncover B.") {
    /* Here's the world:
     *
     *               C --- B
     *             /        \
     *            E          F
     *             \        /
     *               D --- A
     *
     */
    const std::map<std::string, std::set<std::string>> provided = {
        { "A", {"D", "F"} },
        { "B", {"C", "F"} },
        { "C", {"B", "E"} },
        { "D", {"A", "E"} },
        { "E", {"C", "D"} },
        { "F", {"A", "B"} }
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D  E  F
         *   1  1        1
         *   2  1              1
         *   3     1  1
         *   4     1           1
         *   5        1     1
         *   6           1  1
         */

        //       0          1A       2B       3C         4D       5E        6F
        /*0*/ {0,0,0},   {2,11,8},{2,17,14},{2,20,15},{2,23,9},{2,24,21},{2,18,12},
        //       7          8A                           9D
        /*1*/ {-1,5,9},  {1,1,11},                    {4,4,23},
        //       10         11A                                             12F
        /*2*/ {-2,8,12}, {1,8,1},                                        {6,6,18},
        //       13                  14B      15C
        /*3*/ {-3,11,15},         {2,2,17}, {3,3,20},
        //       16                  17B                                    18F
        /*4*/ {-4,14,18},         {2,14,2},                              {6,12,6},
        //       19                           20C                 21E
        /*5*/ {-5,17,21},                   {3,15,3},          {5,5,24},
        //       22                                      23D      24E
        /*6*/ {-6,20,24},                             {4,9,4}, {5,21,5},
        //       25
        /*7*/ {INT_MIN,23,INT_MIN},
    };
    Dx::PartnerLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(dlxItems, matches.links_);

    Pair match = matches.coverPairing(8);
    EXPECT_EQUAL(match, {"A","D"});

    std::vector<Dx::PartnerLinks::personName> lookupCoverA {
        {"",6,2},{"A",0,2},{"B",0,3},{"C",2,5},{"D",3,5},{"E",3,6},{"F",5,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxCoverA {
        /*
         *      B  C  E  F
         *   3  1  1
         *   4  1        1
         *   5     1  1
         */

        //       0          1A       2B       3C         4D       5E        6F
        /*0*/ {0,0,0},   {2,11,8},{2,17,14},{2,20,15},{2,23,9},{1,21,21},{1,18,18},
        //       7          8A                           9D
        /*1*/ {-1,5,9},  {1,1,11},                    {4,4,23},
        //       10         11A                                             12F
        /*2*/ {-2,8,12}, {1,8,1},                                        {6,6,18},
        //       13                  14B      15C
        /*3*/ {-3,11,15},         {2,2,17}, {3,3,20},
        //       16                  17B                                    18F
        /*4*/ {-4,14,18},         {2,14,2},                              {6,6,6},
        //       19                           20C                 21E
        /*5*/ {-5,17,21},                   {3,15,3},          {5,5,5},
        //       22                                      23D      24E
        /*6*/ {-6,20,24},                             {4,9,4}, {5,21,5},
        //       25
        /*7*/ {INT_MIN,23,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);

    // Pair B C but that is a bad choice so we will have to uncover.
    match = matches.coverPairing(14);
    EXPECT_EQUAL(match, {"B","C"});

    std::vector<Dx::PartnerLinks::personName> lookupCoverB {
        {"",6,5},{"A",0,2},{"B",0,3},{"C",0,5},{"D",3,5},{"E",0,6},{"F",5,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxCoverB {
        /*
         * Pairing up B and C will leave us no way to access E and F. Must uncover.
         *
         *       E  F
         *    0
         */

        //       0          1A       2B       3C         4D       5E        6F
        /*0*/ {0,0,0},   {2,11,8},{2,17,14},{2,20,15},{2,23,9},{0,5,5},{0,6,6},
        //       7          8A                           9D
        /*1*/ {-1,5,9},  {1,1,11},                    {4,4,23},
        //       10         11A                                             12F
        /*2*/ {-2,8,12}, {1,8,1},                                        {6,6,18},
        //       13                  14B      15C
        /*3*/ {-3,11,15},         {2,2,17}, {3,3,20},
        //       16                  17B                                    18F
        /*4*/ {-4,14,18},         {2,14,2},                              {6,6,6},
        //       19                           20C                 21E
        /*5*/ {-5,17,21},                   {3,15,3},          {5,5,5},
        //       22                                      23D      24E
        /*6*/ {-6,20,24},                             {4,9,4}, {5,21,5},
        //       25
        /*7*/ {INT_MIN,23,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverB, matches.table_);
    EXPECT_EQUAL(dlxCoverB, matches.links_);

    matches.uncoverPairing(14);
    EXPECT_EQUAL(lookupCoverA, matches.table_);
    EXPECT_EQUAL(dlxCoverA, matches.links_);

    matches.uncoverPairing(8);
    EXPECT_EQUAL(lookup, matches.table_);
    EXPECT_EQUAL(dlxItems, matches.links_);
}


/* * * * * * * * * * * * * * * *            Solve Problem               * * * * * * * * * * * * * */


STUDENT_TEST("Largest shape I will do by hand. After successive calls the network should reset.") {
    /* Here's the world:
     *
     *               A --- B ---C
     *             /        \   \
     *       I----J          E---D
     *       |     \        /
     *       H----- G --- F
     *
     *
     *
     */
    const std::map<std::string, std::set<std::string>> provided = {
        { "A", {"B", "J"} },
        { "B", {"A", "C", "E"} },
        { "C", {"B", "D"} },
        { "D", {"C", "E"} },
        { "E", {"B", "D", "F"} },
        { "F", {"E", "G"} },
        { "G", {"F", "H", "J"} },
        { "H", {"G", "I"} },
        { "I", {"H", "J"} },
        { "J", {"A", "G", "I"} }
    };
    std::vector<Dx::PartnerLinks::personName> lookup {
        {"",10,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,7},{"G",6,8},{"H",7,9},{"I",8,10},{"J",9,0},
    };
    std::vector<Dx::PartnerLinks::personLink> dlxItems {
        /*
         *      A  B  C  D  E  F  G  H  I  J
         *   1  1  1
         *   2  1                          1
         *   3     1  1
         *   4     1        1
         *   5        1  1
         *   6           1  1
         *   7              1  1
         *   8                 1  1
         *   9                    1  1
         *  10                    1        1
         *  11                       1  1
         *  12                          1  1
         *
         */

        //              A         B         C        D          E         F         G         H         I        J
        {0,0,0},    {2,15,12},{3,21,13},{2,24,19},{2,27,25},{3,30,22},{2,33,31},{3,39,34},{2,42,37},{2,45,43},{3,46,16},
        {-1,9,13},  {1,1,15}, {2,2,18},
        {-2,12,16}, {1,12,1},                                                                                 {10,10,40},
        {-3,15,19},           {2,13,21},{3,3,24},
        {-4,18,22},           {2,18,2},                     {5,5,28},
        {-5,21,25},                     {3,19,3}, {4,4,27},
        {-6,24,28},                               {4,25,4}, {5,22,30},
        {-7,27,31},                                         {5,28,5}, {6,6,33},
        {-8,30,34},                                                   {6,31,6}, {7,7,36},
        {-9,33,37},                                                             {7,34,39},{8,8,42},
        {-10,36,40},                                                            {7,36,7},                     {10,16,46},
        {-11,39,43},                                                                      {8,37,8}, {9,9,45},
        {-12,42,46},                                                                                {9,43,9}, {10,40,10},
        {INT_MIN,45,INT_MIN},
    };

    Dx::PartnerLinks network(provided);
    for (int i = 0; i < 11; i++) {
        std::set<Pair> matching = {};
        EXPECT(network.hasPerfectLinks(matching));
        EXPECT_EQUAL(lookup, network.table_);
        EXPECT_EQUAL(dlxItems, network.links_);
    }
}

PROVIDED_TEST("hasPerfectMatching works on a world with just one person.") {
    /* The world is just a single person A, with no others. How sad. :-(
     *
     *                 A
     *
     * There is no perfect matching.
     */

    std::set<Pair> unused;
    std::map<std::string, std::set<std::string>> map = {{"A", {}}};
    Dx::PartnerLinks network(map);
    EXPECT(!network.hasPerfectLinks(unused));
}

PROVIDED_TEST("hasPerfectMatching works on an empty set of people.") {
    /* There actually is a perfect matching - the set of no links meets the
     * requirements.
     */
    std::set<Pair> unused;
    std::map<std::string, std::set<std::string>> map = {};
    Dx::PartnerLinks network(map);
    EXPECT(network.hasPerfectLinks(unused));
}

PROVIDED_TEST("hasPerfectMatching works on a world with two linked people.") {
    /* This world is a pair of people A and B. There should be a perfect matching.
     *
     *               A -- B
     *
     * The matching is {A, B}
     */
    auto links = fromLinks({
        { "A", "B" }
    });

    std::set<Pair> unused;
    Dx::PartnerLinks network(links);
    EXPECT(network.hasPerfectLinks(unused));
}

PROVIDED_TEST("hasPerfectMatching works on a world with two linked people, and produces output.") {
    /* This world is a pair of people A and B. There should be a perfect matching.
     *
     *               A -- B
     *
     * The matching is {A, B}
     */
    auto links = fromLinks({
        { "A", "B" }
    });

    std::set<Pair> expected = {
        { "A", "B" }
    };

    std::set<Pair> matching;
    Dx::PartnerLinks network(links);
    EXPECT(network.hasPerfectLinks(matching));
    EXPECT_EQUAL(matching, expected);
}

PROVIDED_TEST("hasPerfectMatching works on a triangle of people.") {
    /* Here's the world:
     *
     *               A --- B
     *                \   /
     *                 \ /
     *                  C
     *
     * There is no perfect matching here, unfortunately.
     */
    auto links = fromLinks({
        { "A", "B" },
        { "B", "C" },
        { "C", "A" }
    });

    std::set<Pair> unused;
    Dx::PartnerLinks network(links);
    EXPECT(!network.hasPerfectLinks(unused));
}

PROVIDED_TEST("hasPerfectMatching works on a square of people.") {
    /* Here's the world:
     *
     *               A --- B
     *               |     |
     *               |     |
     *               D --- C
     *
     * There are two different perfect matching here: AB / CD, and AD/BD.
     * Either will work.
     */
    auto links = fromLinks({
        { "A", "B" },
        { "B", "C" },
        { "C", "D" },
        { "D", "A" }
    });

    std::set<Pair> unused;
    Dx::PartnerLinks network(links);
    EXPECT(network.hasPerfectLinks(unused));
}

PROVIDED_TEST("hasPerfectMatching works on a square of people, and produces output.") {
    /* Here's the world:
     *
     *               A --- B
     *               |     |
     *               |     |
     *               C --- D
     *
     * There are two different perfect matching here: AB / CD, and AC/BC.
     * Either will work.
     */
    auto links = fromLinks({
        { "A", "B" },
        { "B", "C" },
        { "C", "D" },
        { "D", "A" }
    });

    std::set<Pair> matching;
    Dx::PartnerLinks network(links);
    EXPECT(network.hasPerfectLinks(matching));
    EXPECT(isPerfectMatching(links, matching));
}

PROVIDED_TEST("hasPerfectMatching works on a pentagon of people.") {
    /* Here's the world:
     *
     *               A --- B
     *             /       |
     *            E        |
     *             \       |
     *               D --- C
     *
     * There is no perfect matching here, since the cycle has odd
     * length.
     */
    auto links = fromLinks({
        { "A", "B" },
        { "B", "C" },
        { "C", "D" },
        { "D", "E" },
        { "E", "A" }
    });

    std::set<Pair> unused;
    Dx::PartnerLinks network(links);
    EXPECT(!network.hasPerfectLinks(unused));
}

PROVIDED_TEST("hasPerfectMatching works on a line of six people.") {
    /* Because std::map and std::set internally store items in sorted order, the order
     * in which you iterate over people when making decisions is sensitive
     * to the order of those peoples' names. This test looks at a group like
     * this one, trying out all possible orderings of peoples' names:
     *
     *
     *
     *        * -- * -- * -- * -- * -- *
     *
     *
     *
     * There are 6! = 720 possible permutations of the ordering of these six
     * people. If your code is able to solve the problem correctly for all of
     * those orderings, there's a good chance that you're correctly tracking
     * who is matched at each step. On the other hand, if your code runs into
     * issues here, it may indicate that there's a bug in how you mark who's
     * paired and who isn't.
     */
    std::vector<std::string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        std::map<std::string, std::set<std::string>> links = fromLinks({
            { people[0], people[1] },
            { people[1], people[2] },
            { people[2], people[3] },
            { people[3], people[4] },
            { people[4], people[5] }
        });

        std::set<Pair> matching;
        Dx::PartnerLinks network(links);
        EXPECT(network.hasPerfectLinks(matching));
        EXPECT(isPerfectMatching(links, matching));
    } while (next_permutation(people.begin(), people.end()));
}

PROVIDED_TEST("hasPerfectMatching works on a more complex negative example.") {
    /* Because std::map and std::set internally store items in sorted order, the order
     * in which you iterate over people when making decisions is sensitive
     * to the order of those peoples' names. This test looks at a group like
     * this one, trying out all possible orderings of peoples' names:
     *
     *
     *         *        *
     *          \      /
     *           * -- *
     *          /      \
     *         *        *
     *
     * There are 6! = 720 possible permutations of the ordering of these six
     * people. If your code is able to solve the problem correctly for all of
     * those orderings, there's a good chance that you're correctly tracking
     * who is matched at each step. On the other hand, if your code runs into
     * issues here, it may indicate that there's a bug in how you mark who's
     * paired and who isn't.
     */
    std::vector<std::string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        std::map<std::string, std::set<std::string>> links = fromLinks({
            { people[0], people[2] },
            { people[1], people[2] },
            { people[2], people[3] },
            { people[3], people[4] },
            { people[3], people[5] },
        });

        std::set<Pair> matching;
        Dx::PartnerLinks network(links);
        EXPECT(!network.hasPerfectLinks(matching));
    } while (next_permutation(people.begin(), people.end()));
}

PROVIDED_TEST("hasPerfectMatching works on a more complex positive example.") {
    /* Because std::map and std::set internally store items in sorted order, the order
     * in which you iterate over people when making decisions is sensitive
     * to the order of those peoples' names. This test looks at a group like
     * this one, trying out all possible orderings of peoples' names:
     *
     *               *
     *               |
     *               *
     *              / \
     *             *---*
     *            /     \
     *           *       *
     *
     * There are 6! = 720 possible permutations of the ordering of these
     * people. If your code is able to solve the problem correctly for all of
     * those orderings, there's a good chance that you're correctly tracking
     * who is matched at each step. On the other hand, if your code runs into
     * issues here, it may indicate that there's a bug in how you mark who's
     * paired and who isn't.
     */
    std::vector<std::string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        std::map<std::string, std::set<std::string>> links = fromLinks({
            { people[0], people[1] },
            { people[1], people[2] },
            { people[2], people[3] },
            { people[3], people[1] },
            { people[2], people[4] },
            { people[3], people[5] },
        });

        std::set<Pair> matching;
        Dx::PartnerLinks network(links);
        EXPECT(network.hasPerfectLinks(matching));
        EXPECT(isPerfectMatching(links, matching));
    } while (next_permutation(people.begin(), people.end()));
}

PROVIDED_TEST("hasPerfectMatching works on a caterpillar.") {
    /* Because std::map and std::set internally store items in sorted order, the order
     * in which you iterate over people when making decisions is sensitive
     * to the order of those peoples' names. This test looks at a group like
     * this one, trying out all possible orderings of peoples' names:
     *
     *         *---*---*
     *         |   |   |
     *         *   *   *
     *
     * There are 6! = 720 possible permutations of the ordering of these six
     * people. If your code is able to solve the problem correctly for all of
     * those orderings, there's a good chance that you're correctly tracking
     * who is matched at each step. On the other hand, if your code runs into
     * issues here, it may indicate that there's a bug in how you mark who's
     * paired and who isn't.
     */
    std::vector<std::string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        std::map<std::string, std::set<std::string>> links = fromLinks({
            { people[0], people[1] },
            { people[1], people[2] },
            { people[0], people[3] },
            { people[1], people[4] },
            { people[2], people[5] },
        });

        std::set<Pair> matching;
        Dx::PartnerLinks network(links);
        EXPECT(network.hasPerfectLinks(matching));
        EXPECT(isPerfectMatching(links, matching));
    } while (next_permutation(people.begin(), people.end()));
}

PROVIDED_TEST("hasPerfectMatching stress test: negative example (should take under a second).") {
    /* Here, we're giving a "caterpillar" of people, like this:
     *
     *    *   *   *   *     *   *
     *    |   |   |   |     |   |
     *    *---*---*---* ... *---*
     *    |   |   |   |     |   |
     *    *   *   *   *     *   *
     *
     * This doesn't have a perfect matching, However, it may take some searching
     * to confirm this is the case. At this size, however, it should be
     * almost instanteous to find the solution, since the search space is fairly
     * small and most "wrong" decisions can be detected quickly.
     *
     * On the other hand, if your implementation repeatedly constructs the same
     * matchings over and over again, or keeps exploring even when a person who
     * couldn't be paired with the current setup is found, then the number of
     * options you need to consider will be too large for your computer to handle
     * in any reasonable time.
     *
     * If you're passing the other tests and this test hangs, double-check your
     * code to make sure you aren't repeatedly constructing the same matchings
     * multiple times.
     */

    /* Number of "body segments". */
    const int kRowSize = 10;

    std::vector<Pair> links;
    for (int i = 0; i < kRowSize - 1; i++) {
        links.push_back({ std::to_string(i), std::to_string(i + 1) });
    }
    for (int i = 0; i < kRowSize; i++) {
        links.push_back({ std::to_string(i), std::to_string(i + kRowSize) });
    }
    for (int i = 0; i < kRowSize; i++) {
        links.push_back({ std::to_string(i), std::to_string(i + 2 * kRowSize) });
    }

    std::set<Pair> matching;
    Dx::PartnerLinks network(fromLinks(links));
    EXPECT(!network.hasPerfectLinks(matching));
}

PROVIDED_TEST("hasPerfectMatching stress test: positive example (should take under a second).") {
    /* Here, we're giving a "millipede" of people, like this:
     *
     *    *---*---*---* ... *---*
     *    |   |   |   |     |   |
     *    *   *   *   *     *   *
     *
     * This always has a perfect matching, which is found by pairing each person
     * with the person directly below them. However, it may take some searching
     * to find this particular configuration. At this size, however, it should be
     * almost instanteous to find the solution, since the search space is fairly
     * small and most "wrong" decisions can be detected quickly.
     *
     * On the other hand, if your implementation repeatedly constructs the same
     * matchings over and over again, or keeps exploring even when a person who
     * couldn't be paired with the current setup is found, then the number of
     * options you need to consider will be too large for your computer to handle
     * in any reasonable time.
     *
     * If you're passing the other tests and this test hangs, double-check your
     * code to make sure you aren't repeatedly constructing the same matchings
     * multiple times.
     */

    /* Number of "body segments". */
    const int kRowSize = 10;

    std::vector<Pair> links;
    for (int i = 0; i < kRowSize - 1; i++) {
        links.push_back({ std::to_string(i), std::to_string(i + 1) });
    }
    for (int i = 0; i < kRowSize; i++) {
        links.push_back({ std::to_string(i), std::to_string(i + kRowSize) });
    }

    std::set<Pair> matching;
    EXPECT(Dx::PartnerLinks(fromLinks(links)).hasPerfectLinks(matching));
    EXPECT(isPerfectMatching(fromLinks(links), matching));
}


/* * * * * * * * * * * * *     Bonus: Find All Perfect Matchings        * * * * * * * * * * * * * */


/* It would be nice to find a way to cycle through all perfect matchings as an option in the graph
 * viewer. As of now, there is no way to see the results of this function in any meaningful way.
 * Need to learn more about how drawing works for this application then possibly add to it.
 */

PROVIDED_TEST("getAllPerfectMatching works on a square of people, and produces output.") {
    /* Here's the world:
     *
     *               A --- B
     *               |     |
     *               |     |
     *               D --- C
     *
     * There are two different perfect matching here: AB / CD, and AC/BC.
     * Either will work.
     */
    auto links = fromLinks({
        { "A", "B" },
        { "B", "C" },
        { "C", "D" },
        { "D", "A" }
    });
    std::vector<std::set<Pair>> allMatches = {
        {{"A","B"}, {"D","C"}},
        {{"A","D"}, {"B","C"}}
    };
    Dx::PartnerLinks network(links);
    EXPECT_EQUAL(network.getAllPerfectLinks(), allMatches);
}

STUDENT_TEST("All possible pairings is huge, but all perfect matching configs is just 4.") {
    /* Here's the world:
     *
     *               A --- B ---C
     *             /        \   \
     *       I----J          E---D
     *       |     \        /
     *       H----- G --- F
     *
     *
     *
     */
    const std::map<std::string, std::set<std::string>> provided = {
        { "A", {"B", "J"} },
        { "B", {"A", "C", "E"} },
        { "C", {"B", "D"} },
        { "D", {"C", "E"} },
        { "E", {"B", "D", "F"} },
        { "F", {"E", "G"} },
        { "G", {"F", "H", "J"} },
        { "H", {"G", "I"} },
        { "I", {"H", "J"} },
        { "J", {"A", "G", "I"} }
    };
    std::vector<std::set<Pair>> allMatches = {
        {{ "A", "B" }, { "C", "D" }, { "E", "F" }, { "G", "H" }, { "I", "J" }},
        {{ "A", "B" }, { "C", "D" }, { "E", "F" }, { "G", "J" }, { "H", "I" }},
        {{ "A", "J" }, { "B", "C" }, { "D", "E" }, { "F", "G" }, { "H", "I" }},
        {{ "A", "J" }, { "B", "E" }, { "C", "D" }, { "F", "G" }, { "H", "I" }},
    };
    Dx::PartnerLinks network(provided);
    EXPECT_EQUAL(network.getAllPerfectLinks(), allMatches);
}
