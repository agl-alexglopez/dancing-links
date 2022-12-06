#include "PerfectLinks.h"


/* * * * * * * * * * * * * * *    Algorithm X via Dancing Links     * * * * * * * * * * * * * * * */

bool PerfectLinks::hasPerfectLinks(Set<Pair>& pairs) {
    if (dlx.hasSingleton) {
        return false;
    }
    if (dlx.numPeople % 2 != 0) {
        return false;
    }
    return isPerfectMatching(pairs);
}

bool PerfectLinks::isPerfectMatching(Set<Pair>& pairs) {
    if (dlx.lookupTable[0].right == 0) {
        return true;
    }
    // If our previous pairings led to someone that can no longer be reached stop recursion.
    int chosenPerson = choosePerson();
    if (chosenPerson == -1) {
        return false;
    }

    for (int cur = chosenPerson; dlx.links[cur].down != chosenPerson; cur = dlx.links[cur].down) {

        Pair match = coverPerson(cur);

        if (isPerfectMatching(pairs)) {
            pairs.add(match);
            uncoverPerson(cur);
            return true;
        }

        uncoverPerson(cur);
    }
    return false;
}

int PerfectLinks::choosePerson() {
    int head = 0;
    for (int cur = dlx.lookupTable[0].right; cur != head; cur = dlx.lookupTable[cur].right) {
        // Someone has become inaccessible due to other matches.
        if (dlx.links[cur].topOrLen == 0) {
            return -1;
        }
    }
    return dlx.lookupTable[0].right;
}

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

    // In case the other partner is to the left, just decrement index to get to the left.
    if (dlx.links[++index].topOrLen < 0) {
        index -= 2;
    }

    personName p2 = dlx.lookupTable[dlx.links[index].topOrLen];
    dlx.lookupTable[p2.right].left = p2.left;
    dlx.lookupTable[p2.left].right = p2.right;

    // p2 needs to dissapear from all other pairings.
    hidePairings(dlx.links[index], index);

    // This is reported as an output parameter for the pairings we chose.
    return {p1.name, p2.name};
}

void PerfectLinks::uncoverPerson(int index) {

    index = dlx.links[index].down;

    // Go in the reverse order, uncovering person 2 then person 1.
    if (dlx.links[--index].topOrLen < 0) {
        index += 2;
    }

    personName p2 = dlx.lookupTable[dlx.links[index].topOrLen];
    dlx.lookupTable[p2.left].right = dlx.links[index].topOrLen;
    dlx.lookupTable[p2.right].left = dlx.links[index].topOrLen;

    unhidePairings(dlx.links[index], index);

    if (dlx.links[--index].topOrLen < 0) {
        index += 2;
    }


    personName p1 = dlx.lookupTable[dlx.links[index].topOrLen];
    dlx.lookupTable[p1.left].right = dlx.links[index].topOrLen;
    dlx.lookupTable[p1.right].left = dlx.links[index].topOrLen;

    unhidePairings(dlx.links[index], index);

}

void PerfectLinks::unhidePairings(personLink& start, int index) {
    personLink nextPairing = start;
    index = start.up;
    while ((nextPairing = dlx.links[nextPairing.up]) != start) {
        if (index > dlx.lookupTable.size()) {

            if (dlx.links[--index].topOrLen < 0) {
                index += 2;
            }
            personLink cur = dlx.links[index];

            dlx.links[cur.up].down = index;
            dlx.links[cur.down].up = index;
            dlx.links[cur.topOrLen].topOrLen++;
        }
        index = nextPairing.up;
    }

}

void PerfectLinks::hidePairings(personLink& start, int index) {
    personLink nextPairing = start;
    index = start.down;
    while ((nextPairing = dlx.links[nextPairing.down]) != start) {
        // We may need this guard to prevent splicing while on a column header.
        if (index > dlx.lookupTable.size()) {

            // In case the other partner is to the left, just decrement index to go left.
            if (dlx.links[++index].topOrLen < 0) {
                index -= 2;
            }
            personLink cur = dlx.links[index];

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
        {"",3,2},{"A",0,2},{"B",0,3},{"C",2,0},{"D",3,0}
    };
    Vector<PerfectLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);
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

    Pair match = matches.coverPerson(1);
    EXPECT_EQUAL(match, {"A","D"});

    Vector<PerfectLinks::personName> lookupCoverA {
        {"",6,2},{"A",0,2},{"B",0,3},{"C",2,5},{"D",3,5},{"E",3,6},{"F",5,0},
    };
    Vector<PerfectLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);
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
        {"",3,2},{"A",0,2},{"B",0,3},{"C",2,0},{"D",3,0}
    };
    Vector<PerfectLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);

    match = matches.coverPerson(2);
    EXPECT_EQUAL(match, {"B", "C"});
    Vector<PerfectLinks::personName> lookupCoverB {
        {"",0,0},{"A",0,2},{"B",0,3},{"C",0,0},{"D",3,0}
    };
    Vector<PerfectLinks::personLink> dlxCoverB {
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
    EXPECT_EQUAL(lookupCoverB, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverB, matches.dlx.links);
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

    matches.uncoverPerson(1);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);

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

    matches.uncoverPerson(1);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);
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
        {"",3,2},{"A",0,2},{"B",0,3},{"C",2,0},{"D",3,0}
    };
    Vector<PerfectLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);

    matches.uncoverPerson(6);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);
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

    Pair match = matches.coverPerson(1);
    EXPECT_EQUAL(match, {"A","D"});

    Vector<PerfectLinks::personName> lookupCoverA {
        {"",6,2},{"A",0,2},{"B",0,3},{"C",2,5},{"D",3,5},{"E",3,6},{"F",5,0},
    };
    Vector<PerfectLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);

    matches.uncoverPerson(1);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(matches.dlx.links, dlxItems);
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

    Pair match = matches.coverPerson(1);
    EXPECT_EQUAL(match, {"A","D"});

    Vector<PerfectLinks::personName> lookupCoverA {
        {"",6,2},{"A",0,2},{"B",0,3},{"C",2,5},{"D",3,5},{"E",3,6},{"F",5,0},
    };
    Vector<PerfectLinks::personLink> dlxCoverA {
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
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);

    // Pair B C but that is a bad choice so we will have to uncover.
    match = matches.coverPerson(2);
    EXPECT_EQUAL(match, {"B","C"});

    Vector<PerfectLinks::personName> lookupCoverB {
        {"",6,5},{"A",0,2},{"B",0,3},{"C",0,5},{"D",3,5},{"E",0,6},{"F",5,0},
    };
    Vector<PerfectLinks::personLink> dlxCoverB {
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
    EXPECT_EQUAL(lookupCoverB, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverB, matches.dlx.links);

    matches.uncoverPerson(2);
    EXPECT_EQUAL(lookupCoverA, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxCoverA, matches.dlx.links);

    matches.uncoverPerson(1);
    EXPECT_EQUAL(lookup, matches.dlx.lookupTable);
    EXPECT_EQUAL(dlxItems, matches.dlx.links);
}

namespace {

    /* Pairs to world. */
    Map<std::string, Set<std::string>> fromLinks(const Vector<Pair>& pairs) {
        Map<std::string, Set<std::string>> result;
        for (const auto& link: pairs) {
            result[link.first()].add(link.second());
            result[link.second()].add(link.first());
        }
        return result;
    }

    /* Checks if a set of pairs forms a perfect matching. */
    bool isPerfectMatching(const Map<std::string, Set<std::string>>& possibleLinks,
                           const Set<Pair>& matching) {
        /* Need to check that
         *
         * 1. each pair is indeed a possible link,
         * 2. each person appears in exactly one pair.
         */
        Set<std::string> used;
        for (Pair p: matching) {
            /* Are these folks even in the group of people? */
            if (!possibleLinks.containsKey(p.first())) return false;
            if (!possibleLinks.containsKey(p.second())) return false;

            /* If these people are in the group, are they linked? */
            if (!possibleLinks[p.first()].contains(p.second()) ||
                !possibleLinks[p.second()].contains(p.first())) {
                return false;
            }

            /* Have we seen them before? */
            if (used.contains(p.first()) || used.contains(p.second())) {
                return false;
            }

            /* Add them both. */
            used += p.first();
            used += p.second();
        }

        /* Confirm that's everyone. */
        return used.size() == possibleLinks.size();
    }
}

PROVIDED_TEST("hasPerfectMatching works on a world with just one person.") {
    /* The world is just a single person A, with no others. How sad. :-(
     *
     *                 A
     *
     * There is no perfect matching.
     */

    Set<Pair> unused;
    PerfectLinks network({{"A", {}}});
    EXPECT(!network.hasPerfectLinks(unused));
}

PROVIDED_TEST("hasPerfectMatching works on an empty set of people.") {
    /* There actually is a perfect matching - the set of no links meets the
     * requirements.
     */
    Set<Pair> unused;
    PerfectLinks network({});
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

    Set<Pair> unused;
    PerfectLinks network(links);
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

    Set<Pair> expected = {
        { "A", "B" }
    };

    Set<Pair> matching;
    PerfectLinks network(links);
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

    Set<Pair> unused;
    PerfectLinks network(links);
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

    Set<Pair> unused;
    PerfectLinks network(links);
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

    Set<Pair> matching;
    PerfectLinks network(links);
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

    Set<Pair> unused;
    PerfectLinks network(links);
    EXPECT(!network.hasPerfectLinks(unused));
}

PROVIDED_TEST("hasPerfectMatching works on a line of six people.") {
    /* Because Map and Set internally store items in sorted order, the order
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
    Vector<std::string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        Map<std::string, Set<std::string>> links = fromLinks({
            { people[0], people[1] },
            { people[1], people[2] },
            { people[2], people[3] },
            { people[3], people[4] },
            { people[4], people[5] }
        });

        Set<Pair> matching;
        PerfectLinks network(links);
        EXPECT(network.hasPerfectLinks(matching));
        EXPECT(isPerfectMatching(links, matching));
    } while (next_permutation(people.begin(), people.end()));
}

PROVIDED_TEST("hasPerfectMatching works on a more complex negative example.") {
    /* Because Map and Set internally store items in sorted order, the order
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
    Vector<std::string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        Map<std::string, Set<std::string>> links = fromLinks({
            { people[0], people[2] },
            { people[1], people[2] },
            { people[2], people[3] },
            { people[3], people[4] },
            { people[3], people[5] },
        });

        Set<Pair> matching;
        PerfectLinks network(links);
        EXPECT(!network.hasPerfectLinks(matching));
    } while (next_permutation(people.begin(), people.end()));
}

PROVIDED_TEST("hasPerfectMatching works on a more complex positive example.") {
    /* Because Map and Set internally store items in sorted order, the order
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
    Vector<std::string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        Map<std::string, Set<std::string>> links = fromLinks({
            { people[0], people[1] },
            { people[1], people[2] },
            { people[2], people[3] },
            { people[3], people[1] },
            { people[2], people[4] },
            { people[3], people[5] },
        });

        Set<Pair> matching;
        PerfectLinks network(links);
        EXPECT(network.hasPerfectLinks(matching));
        EXPECT(isPerfectMatching(links, matching));
    } while (next_permutation(people.begin(), people.end()));
}

PROVIDED_TEST("hasPerfectMatching works on a caterpillar.") {
    /* Because Map and Set internally store items in sorted order, the order
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
    Vector<std::string> people = { "A", "B", "C", "D", "E", "F" };
    do {
        Map<std::string, Set<std::string>> links = fromLinks({
            { people[0], people[1] },
            { people[1], people[2] },
            { people[0], people[3] },
            { people[1], people[4] },
            { people[2], people[5] },
        });

        Set<Pair> matching;
        PerfectLinks network(links);
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

    Vector<Pair> links;
    for (int i = 0; i < kRowSize - 1; i++) {
        links.add({ std::to_string(i), std::to_string(i + 1) });
    }
    for (int i = 0; i < kRowSize; i++) {
        links.add({ std::to_string(i), std::to_string(i + kRowSize) });
    }
    for (int i = 0; i < kRowSize; i++) {
        links.add({ std::to_string(i), std::to_string(i + 2 * kRowSize) });
    }

    Set<Pair> matching;
    PerfectLinks network(fromLinks(links));
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

    Vector<Pair> links;
    for (int i = 0; i < kRowSize - 1; i++) {
        links.add({ std::to_string(i), std::to_string(i + 1) });
    }
    for (int i = 0; i < kRowSize; i++) {
        links.add({ std::to_string(i), std::to_string(i + kRowSize) });
    }

    Set<Pair> matching;
    PerfectLinks network(fromLinks(links));
    EXPECT(network.hasPerfectLinks(matching));
    EXPECT(isPerfectMatching(fromLinks(links), matching));
}
