#include "GUI/SimpleTest.h"
#include "DisasterPlanning.h"
#include "DisasterLinks.h"
#include "DisasterTags.h"
#include "DisasterUtilities.h"
#include <chrono>


STUDENT_TEST("6x6 Grid Set Based Test.") {
    Map<std::string, Set<std::string>> grid;

    /* Build the grid. */
    char maxRow = 'F';
    int  maxCol = 6;
    for (char row = 'A'; row <= maxRow; row++) {
        for (int col = 1; col <= maxCol; col++) {
            if (row != maxRow) {
                grid[row + std::to_string(col)] += (char(row + 1) + std::to_string(col));
            }
            if (col != maxCol) {
                grid[row + std::to_string(col)] += (char(row) + std::to_string(col + 1));
            }
        }
    }
    grid = makeSymmetric(grid);

    Set<std::string> locations;
    auto start = std::chrono::high_resolution_clock::now();
    EXPECT(canBeMadeDisasterReady(grid, 10, locations));
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "Implementation: Copies of Sets 6x6 Grid" << std::endl;
    std::cout << "Time(microseconds): " << duration.count() << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
}

STUDENT_TEST("6x6 Grid Quadruple Linked DLX Test.") {
    Map<std::string, Set<std::string>> grid;

    /* Build the grid. */
    char maxRow = 'F';
    int  maxCol = 6;
    for (char row = 'A'; row <= maxRow; row++) {
        for (int col = 1; col <= maxCol; col++) {
            if (row != maxRow) {
                grid[row + std::to_string(col)] += (char(row + 1) + std::to_string(col));
            }
            if (col != maxCol) {
                grid[row + std::to_string(col)] += (char(row) + std::to_string(col + 1));
            }
        }
    }
    grid = makeSymmetric(grid);

    Set<std::string> locations;
    DisasterLinks network(grid);

    auto start = std::chrono::high_resolution_clock::now();
    EXPECT(network.isDisasterReady(10, locations));
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "Implementation: Quadruple Linked DLX 6x6 Grid" << std::endl;
    std::cout << "Time(microseconds): " << duration.count() << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
}

STUDENT_TEST("6x6 Grid Depth Tracking DLX Test.") {
    Map<std::string, Set<std::string>> grid;

    /* Build the grid. */
    char maxRow = 'F';
    int  maxCol = 6;
    for (char row = 'A'; row <= maxRow; row++) {
        for (int col = 1; col <= maxCol; col++) {
            if (row != maxRow) {
                grid[row + std::to_string(col)] += (char(row + 1) + std::to_string(col));
            }
            if (col != maxCol) {
                grid[row + std::to_string(col)] += (char(row) + std::to_string(col + 1));
            }
        }
    }
    grid = makeSymmetric(grid);

    Set<std::string> locations;
    DisasterTags network(grid);

    auto start = std::chrono::high_resolution_clock::now();
    EXPECT(network.hasDisasterCoverage(10, locations));
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "Implementation: Depth Tracking DLX 6x6 Grid" << std::endl;
    std::cout << "Time(microseconds): " << duration.count() << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
}

STUDENT_TEST("Ring stress test Set Based.") {
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
    Map<std::string, Set<std::string>> map = makeMap({
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

    Set<std::string> chosen = {};

    DisasterLinks network(map);
    /* We should be able to cover everything with two cities. */
    auto start = std::chrono::high_resolution_clock::now();
    EXPECT(canBeMadeDisasterReady(map, 16, chosen));
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "Implementation: Set Based Confirm Coverage 16 Supplies Ring" << std::endl;
    std::cout << "Time(microseconds): " << duration.count() << std::endl;
    std::cout << "--------------------------------------------" << std::endl;

    /* Those cities should be 1 and 3. */
    EXPECT_EQUAL(chosen, { cities[1], cities[2], cities[3], cities[4], cities[5], cities[6],
                           cities[7],cities[8],cities[9],cities[10],cities[11],cities[12],
                           cities[13],cities[14],cities[15],cities[16], });

    chosen.clear();

    auto start2 = std::chrono::high_resolution_clock::now();
    EXPECT(!network.isDisasterReady(15, chosen));
    auto stop2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(stop2 - start2);
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "Implementation: Set Based Deny Coverage 15 Supplies Ring" << std::endl;
    std::cout << "Time(microseconds): " << duration2.count() << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
}

STUDENT_TEST("Ring stress test quadruple linked DLX.") {
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
    Map<std::string, Set<std::string>> map = makeMap({
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

    Set<std::string> chosen = {};

    DisasterLinks network(map);
    auto start = std::chrono::high_resolution_clock::now();
    /* We should be able to cover everything with two cities. */
    EXPECT(network.isDisasterReady(16, chosen));
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "Implementation: Quadruple Linked DLX Ring Test Confirm 16" << std::endl;
    std::cout << "Time(microseconds): " << duration.count() << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    /* Those cities should be 1 and 3. */
    EXPECT_EQUAL(chosen, { cities[1], cities[2], cities[3], cities[4], cities[5], cities[6],
                           cities[7],cities[8],cities[9],cities[10],cities[11],cities[12],
                           cities[13],cities[14],cities[15],cities[16], });

    chosen.clear();

    auto start2 = std::chrono::high_resolution_clock::now();
    EXPECT(!network.isDisasterReady(15, chosen));
    auto stop2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(stop2 - start2);
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "Implementation: Quadruple Linked DLX Ring Test Deny 15" << std::endl;
    std::cout << "Time(microseconds): " << duration2.count() << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
}

STUDENT_TEST("Ring stress test Depth Based DLX.") {
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
    Map<std::string, Set<std::string>> map = makeMap({
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

    Set<std::string> chosen = {};

    DisasterTags network(map);
    auto start = std::chrono::high_resolution_clock::now();
    /* We should be able to cover everything with two cities. */
    EXPECT(network.hasDisasterCoverage(16, chosen));
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "Implementation: Depth Based DLX Ring Test Confirm 16" << std::endl;
    std::cout << "Time(microseconds): " << duration.count() << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    /* Those cities should be 1 and 3. */
    EXPECT_EQUAL(chosen, { cities[1], cities[2], cities[3], cities[4], cities[5], cities[6],
                           cities[7],cities[8],cities[9],cities[10],cities[11],cities[12],
                           cities[13],cities[14],cities[15],cities[16], });

    chosen.clear();

    auto start2 = std::chrono::high_resolution_clock::now();
    EXPECT(!network.hasDisasterCoverage(15, chosen));
    auto stop2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(stop2 - start2);
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "Implementation: Depth Based DLX Ring Test Deny 15" << std::endl;
    std::cout << "Time(microseconds): " << duration2.count() << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
}
