#include "PerfectLinks.h"


/* * * * * * * * * * * * * * *    Algorithm X via Dancing Links     * * * * * * * * * * * * * * * */


Pair PerfectLinks::coverPerson(int index) {

    // We always start at the index of the header for that column.
    personLink start = dlx.links[dlx.links[index].down];
    // Now step into the first option for that item.
    index = dlx.links[index].down;


    /* We now must cover the two people in this option in the lookup table. Then go through all
     * other options and eliminate the other pairings in which each appears because they are paired
     * off and therefore no longer accessible to other people that want to pair with them.
     */

    personName p1 = dlx.lookupTable[dlx.links[index].topOrLen];
    dlx.lookupTable[p1.right].left = p1.left;
    dlx.lookupTable[p1.left].right = p1.right;

    // p1 needs to dissapear from all other pairings.
    hidePairings(start, index);

    if (dlx.links[++index].topOrLen < 0) {
        index = dlx.links[index].up;
    }


    // We will not use any intelligent or greedy approaches so next partner will always be to right.
    personName p2 = dlx.lookupTable[dlx.links[index].topOrLen];
    dlx.lookupTable[p2.right].left = p2.left;
    dlx.lookupTable[p2.left].right = p2.right;

    // p2 needs to dissapear from all other pairings.
    hidePairings(dlx.links[index], index);

    // This is reported as an output parameter for the pairings we chose.
    return {p1.name, p2.name};
}

void PerfectLinks::hidePairings(personLink& start, int index) {
    personLink nextPairing = start;
    index = start.down;
    while ((nextPairing = dlx.links[nextPairing.down]) != start) {
        // We may need this guard to prevent splicing while on a column header.
        if (index > dlx.lookupTable.size()) {
            // We should only ever have to hide the other person in the pair. No loop needed.
            personLink cur = dlx.links[index + 1];

            dlx.links[cur.up].down = cur.down;
            dlx.links[cur.down].up = cur.up;
            dlx.links[cur.topOrLen].topOrLen--;
        }
        index = nextPairing.down;
    }
}


/* * * * * * * * * * * * * * *   Constructor to Build the Network   * * * * * * * * * * * * * * * */


PerfectLinks::PerfectLinks(const Map<std::string, Set<std::string>>& possibleLinks) {

    dlx.numPairings = 0;
    dlx.numPeople = 0;
    dlx.hasSingleton = false;

    HashMap<std::string, int> columnBuilder = {};

    // Set up the headers first. Lookup table and first N headers in links.
    dlx.lookupTable.add({"", 0, 1});
    dlx.links.add({});
    int index = 1;
    for (const auto& p : possibleLinks) {

        columnBuilder[p] = index;
        dlx.headerIndexMap[p] = index;

        dlx.lookupTable.add({p, index - 1, index + 1});
        dlx.lookupTable[0].left++;
        // Add the first headers for the item vector. They need count up and down.
        dlx.links.add({0, index, index});

        dlx.numPeople++;
        index++;
    }
    dlx.lookupTable[dlx.lookupTable.size() - 1].right = 0;

    // Begin building the rows with the negative spacer tiles and the subsequent columns.
    int spacerTitle = -1;

    Set<Pair> seenPairs = {};
    for (const auto& p : possibleLinks) {

        const Set<std::string>& preferences = possibleLinks[p];
        if (preferences.isEmpty()) {
            dlx.hasSingleton = true;
        }

        for (const auto& pref : preferences) {
            Pair newPair = {p, pref};

            if (!seenPairs.contains(newPair)) {
                dlx.numPairings++;
                // Update the count for this column.
                dlx.links.add({spacerTitle,     // Negative to mark spacer.
                               index - 2,       // First item in previous option
                               index + 2});     // Last item in current option
                index++;
                std::string sortedFirst = newPair.first();
                dlx.links.add({dlx.headerIndexMap[sortedFirst],index, index});


                dlx.links[dlx.headerIndexMap[sortedFirst]].topOrLen++;
                // This is the necessary adjustment to the column header's up field for a given item.
                dlx.links[dlx.links[columnBuilder[sortedFirst]].down].up = index;
                // The current node is now the new tail in a vertical circular linked list for an item.
                dlx.links[index].up = columnBuilder[sortedFirst];
                dlx.links[index].down = dlx.links[columnBuilder[sortedFirst]].down;
                // Update the old tail to reflect the new addition of an item in its option.
                dlx.links[columnBuilder[sortedFirst]].down = index;
                // Similar to a previous/current coding pattern but in an above/below column.
                columnBuilder[sortedFirst] = index;

                // Repeat the process. We only ever have two items in an option.
                index++;
                std::string sortedSecond = newPair.second();
                dlx.links.add({dlx.headerIndexMap[sortedSecond], index, index});
                dlx.links[dlx.headerIndexMap[sortedSecond]].topOrLen++;
                dlx.links[dlx.links[columnBuilder[sortedSecond]].down].up = index;
                dlx.links[index].up = columnBuilder[sortedSecond];
                dlx.links[index].down = dlx.links[columnBuilder[sortedSecond]].down;
                dlx.links[columnBuilder[sortedSecond]].down = index;
                columnBuilder[sortedSecond] = index;

                // Because all pairings are bidirectional, they should only apear once as options.
                seenPairs.add(newPair);
                index++;
                spacerTitle--;
            }

        }
    }

    dlx.links.add({INT_MIN, index - 2, INT_MIN});

}


/* * * * * * * * * * * * * * * *   Overloaded Operators for Debugging   * * * * * * * * * * * * * */


bool operator==(const PerfectLinks::personLink& lhs, const PerfectLinks::personLink& rhs) {
    return lhs.topOrLen == rhs.topOrLen && lhs.up == rhs.up && lhs.down == rhs.down;
}

bool operator!=(const PerfectLinks::personLink& lhs, const PerfectLinks::personLink& rhs) {
    return !(lhs == rhs);
}

bool operator==(const PerfectLinks::personName& lhs, const PerfectLinks::personName& rhs) {
    return lhs.name == rhs.name && lhs.left == rhs.left && lhs.right == rhs.right;
}

bool operator!=(const PerfectLinks::personName& lhs, const PerfectLinks::personName& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, const PerfectLinks::personLink& person) {
    os << "{ topOrLen: " << person.topOrLen
       << ", up: " << person.up << ", down: " << person.down << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const PerfectLinks::personName& name) {
    os << "{ name: " << name.name << ", left: " << name.left << ", right: " << name.right << " }";
    return os;
}

std::ostream& operator<<(std::ostream&os, const Vector<PerfectLinks::personLink>& links) {
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

std::ostream& operator<<(std::ostream&os, const PerfectLinks& links) {
    os << "LOOKUP ARRAY" << std::endl;
    for (const auto& header : links.dlx.lookupTable) {
        os << "{\"" << header.name << "\"," << header.left << "," << header.right << "},";
    }
    os << std::endl;
    os << "DLX ARRAY" << std::endl;
    for (const auto& item : links.dlx.links) {
        if (item.topOrLen < 0) {
            os << std::endl;
        }
        os << "{" << item.topOrLen << "," << item.up << "," << item.down << "},";
    }
    os << std::endl;
    return os;
}


/* * * * * * * * * * * * * * * *      Test Cases Below this Point       * * * * * * * * * * * * * */


/* * * * * * * * * * * * * * * *             Initialization             * * * * * * * * * * * * * */


STUDENT_TEST("Line of six but tricky due to natural order.") {
    /*
     *
     *       C--D--A--F--B--E
     *
     *
     */
    const Map<std::string, Set<std::string>> provided = {
        {"A", {"D", "F"}},
        {"B", {"E", "F"}},
        {"C", {"D"}},
        {"D", {"A", "C"}},
        {"E", {"B"}},
        {"F", {"A", "B"}},
    };
    Vector<PerfectLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0}
    };
    Vector<PerfectLinks::personLink> dlxItems {
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
    PerfectLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxItems, matches.dlx.links);
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
    const Map<std::string, Set<std::string>> provided = {
        { "A", {"B"} },
        { "B", {"C"} },
        { "C", {"A"} }
    };
    Vector<PerfectLinks::personName> lookup {
        {"",3,1},{"A",0,2},{"B",1,3},{"C",2,0},
    };
    Vector<PerfectLinks::personLink> dlxItems {
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
    PerfectLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);
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
    const Map<std::string, Set<std::string>> provided = {
        { "A", {"D", "F"} },
        { "B", {"C", "F"} },
        { "C", {"B", "E"} },
        { "D", {"A", "E"} },
        { "E", {"C", "D"} },
        { "F", {"A", "B"} }
    };
    Vector<PerfectLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    Vector<PerfectLinks::personLink> dlxItems {
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
    PerfectLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxItems, matches.dlx.links);
}

STUDENT_TEST("Setup works on a disconnected hexagon of people and reportes singleton.") {
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
    const Map<std::string, Set<std::string>> provided = {
        { "A", {} },
        { "B", {"C", "F"} },
        { "C", {"B", "E"} },
        { "D", {"E"} },
        { "E", {"C", "D"} },
        { "F", {"B"} }
    };
    Vector<PerfectLinks::personName> lookup {
        {"",6,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,5},{"E",4,6},{"F",5,0},
    };
    Vector<PerfectLinks::personLink> dlxItems {
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
    PerfectLinks matches(provided);
    EXPECT(matches.dlx.hasSingleton);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxItems, matches.dlx.links);
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
    const Map<std::string, Set<std::string>> provided = {
        { "A",{"B","C"}},
        { "B",{"A","D"}},
        { "C",{"A","D"}},
        { "D",{"C","B"}},
    };
    Vector<PerfectLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0},
    };
    Vector<PerfectLinks::personLink> dlxItems {
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
    PerfectLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);

    // Cover A, this will select option 2, partners are AB.
    Pair match = matches.coverPerson(1);
    EXPECT_EQUAL(match, {"A","B"});

    Vector<PerfectLinks::personName> lookupCoverA {
        {"",4,3},{"A",0,2},{"B",0,3},{"C",0,4},{"D",3,0},
    };
    Vector<PerfectLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);
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
    const Map<std::string, Set<std::string>> provided = {
        { "A", {"B"} },
        { "B", {"C"} },
        { "C", {"A"} }
    };
    Vector<PerfectLinks::personName> lookup {
        {"",3,1},{"A",0,2},{"B",1,3},{"C",2,0},
    };
    Vector<PerfectLinks::personLink> dlxItems {
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
    PerfectLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);

    Pair match = matches.coverPerson(1);
    EXPECT_EQUAL(match, {"A", "B"});
    Vector<PerfectLinks::personName> lookupA {
        {"",3,3},{"A",0,2},{"B",0,3},{"C",0,0},
    };
    Vector<PerfectLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);
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
    const Map<std::string, Set<std::string>> provided = {
        { "A", {"B", "D"} },
        { "B", {"A","C","D"} },
        { "C", {"B"} },
        { "D", {"A","B"}},
    };
    Vector<PerfectLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0}
    };
    Vector<PerfectLinks::personLink> dlxItems {
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
    PerfectLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);

    Pair match = matches.coverPerson(1);
    EXPECT_EQUAL(match, {"A", "B"});
    Vector<PerfectLinks::personName> lookupCoverA {
        {"",4,3},{"A",0,2},{"B",0,3},{"C",0,4},{"D",3,0}
    };
    Vector<PerfectLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);
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
    const Map<std::string, Set<std::string>> provided = {
        { "A", {"B", "D"} },
        { "B", {"A","C","D"} },
        { "C", {"B"} },
        { "D", {"A","B"}},
    };
    Vector<PerfectLinks::personName> lookup {
        {"",4,1},{"A",0,2},{"B",1,3},{"C",2,4},{"D",3,0}
    };
    Vector<PerfectLinks::personLink> dlxItems {
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
    PerfectLinks matches(provided);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);

    Pair match = matches.coverPerson(6);
    EXPECT_EQUAL(match, {"A", "D"});
    Vector<PerfectLinks::personName> lookupCoverA {
        {"",4,1},{"A",0,2},{"B",0,3},{"C",2,0},{"D",3,0}
    };
    Vector<PerfectLinks::personLink> dlxCoverA {
        /*
         *      B  C
         *   3  1  1
         *
         */

        //       0         1A       2B        3C       4D
        /*0*/ {0,0,0},   {2,9,6}, {3,15,7},{1,13,13},{2,16,10},
        //       5         6A       7B
        /*1*/ {-1,3,7},  {1,1,9}, {2,2,2},
        //       8         9A                          10D
        /*2*/ {-2,6,10}, {1,6,1},                    {4,4,16},
        //       11                 12B       13C
        /*3*/ {-3,9,13},          {2,7,2}, {3,3,3},
        //       14                 15B                16D
        /*4*/ {-4,12,16},         {2,12,2},          {4,10,4},
        //       17
              {INT_MIN,15,INT_MIN},
    };
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);
}
