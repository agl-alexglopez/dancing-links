/**
 * Author: Alexander G. Lopez
 * File: PartnerLinks.h
 * --------------------------
 * This file contains the class I use to implement Algorithm X via Dancing Links. This algorithm
 * is adjusted and used to solve the Perfect Matching and Max Weight Matching problems on a graph.
 * If only perfect matching is desired give the class constructor a Map<string,Set<string>> with
 * a person and their partners. If you want the max weight matching of a graph, provide a
 * Map<string,Map<string,int>> to the constructor with a person and the weights of their partners.
 * You can ask a weighted or unweighted graph for max weight matching, but if you did not provide
 * weight information, you cannot ask for the max weight matching and must construct a new instance
 * of the class with that information.
 *
 * Here is a brief overview of the approach that this implementation uses to solve the problems. For
 * more detail, please see the README.md in the repository.
 *
 * Perfect Matching:
 *
 * In order to solve the Perfect Matching problem for a graph, we set up a matrix as follows.
 *
 * The graph:
 *
 *              C --- B
 *             /       \
 *            E         F
 *             \       /
 *              D --- A
 *
 * Its matrix representation.
 *
 *            *                 <- We select A in the following example.
 *            A  B  C  D  E  F  <- Every person that exists in the world.
 *         1  1        1        <- Partner options for a person based on the graph.
 *         2  1              1
 *         3     1  1
 *         4     1           1
 *         5        1     1
 *         6           1  1
 *
 * From this grid, select a person to "cover." When we cover a person in a perfect matching, that
 * person must disappear from all other pairings in which they appear. The partner in the option we
 * have chosen must also disappear from all other pairings. Neither person is available for future
 * pairings.
 *
 * Here we select to cover person A throught option 1. This means they are paired with D. The matrix
 * shrinks as follows.
 *
 *            B  C  E  F
 *         3  1  1
 *         4  1        1
 *         5     1  1
 *
 * The goal is to shrink the matrix so that there are no items or options remaining. Everyone is
 * paired off. For example, continuing the process for the above grid, we should can only select
 * B-F and C-E to complete the matching.
 *
 *            *                     *
 *            B  C  E  F            C  E           0
 *         3  1  1               5  1  1        0
 *         4  1        1   -->             -->
 *         5     1  1
 *
 * Weighted Matching:
 *
 * In order to solve the weighted matching problem, set up a matrix as follows.
 *
 *               E
 *               | 1
 *               C
 *            1 / \ 5
 *             A---B
 *          1 /  1  \ 1
 *           F       D
 *
 * In a matrix representation, we use the same method as perfect matching except every option
 * includes the weight of the partnership.
 *
 *           A  B  C  D  E  F  <- Every person in the world
 *        1  1     1           <- The weights of the partnerships
 *        1  1        1
 *        1  1              1
 *        5     1  1
 *        1        1  1
 *        1           1  1
 *
 * While we will use the same elimination method when trying possible pairing for a person, there
 * is a slight adjustment we need to make. To find the max weight matching, we need to try every
 * possible pairing configuration. To do so, we must see the outcome of allowing a person to try
 * every possible pairing they have, and the outcome of that person disappearing from everyone
 * else's options. So, we develop a new method for eliminating ONE person from every option they
 * appear in, not both people in a given option.
 *
 *           B  C  D  E  F
 *        5  1  1
 *        1     1  1
 *        1     1  1
 *
 * This branch will then explore every possible pairing that does not include A. We will eventually
 * go back up the stack and see what would happen if we included A and excluded the next person, and
 * so on. This creates a much slower function than Perfect Matching, but it solves the problem. We
 * will eventually discover the Max Weight Matching is C-B, A-F.
 */
#pragma once
#ifndef PartnerLinks_H
#define PartnerLinks_H
#include <limits.h>
#include <string>
#include "Matchmaker.h"
#include "GUI/SimpleTest.h"
#include "vector.h"
#include "hashmap.h"
#include "map.h"
#include "set.h"


class PartnerLinks {

public:

    /* These are the nodes that live in our matrix representation of the network. If the node is
     * a header for a column topOrLen is the number of options that a person appears in. If the node
     * is in the column topOrLen is the index of the header for that column. Up down pointers are
     * indices in the array. The left right pointers are implicit because these nodes are next to
     * one another in an array.
     */
    typedef struct personLink {
        int topOrLen;
        int up;
        int down;
    }personLink;

    /* These nodes control recursion in a seperate array. As partnerships are chosed or individual
     * people are chosen their representation in the lookup array is spliced out of a doubly linked
     * list.
     */
    typedef struct personName {
        std::string name;
        int left;
        int right;
    }personName;

    /* I place all data structures for the dancing links algorithm in one struct. The lookupTable
     * controls the recursion. The links holds all items and options for recursive backtracking.
     */
    typedef struct Network {
        Vector<personName> lookupTable;
        Vector<personLink> links;
        int numPeople;                  // Total people in the network.
        int numPairings;                // The number of pairings or rows in the matrix.
        bool hasSingleton;              // No perfect matching if someone is alone.
        bool isWeighted;                // Must provide weights to ask for max weight matching.
    }Network;

    /**
     * @brief PartnerLinks   the constructor for a world intended to check for Perfect Matching.
     *                       Provide a map of the person, and the set of people they are willing to
     *                       pair with. This will then build a dancing link matrix in order to solve
     *                       future queries.
     * @param possibleLinks  the map of people and partners they are willing to work with.
     */
    PartnerLinks(const Map<std::string, Set<std::string>>& possibleLinks);

    /**
     * @brief PartnerLinks   the constructor for a world intended to check for Max Weight Matching.
     *                       Provide a map of the person, and the map of the partners they are
     *                       willing to pair with and the weights of each partnership. If a negative
     *                       weight is given for a partnership, we treat that partnership as if it
     *                       does not exist.
     * @param possibleLinks  the map of people and map of partners and weights.
     */
    PartnerLinks(const Map<std::string, Map<std::string, int>>& possibleLinks);


    /* * * * * * * * * * * * *  Overloaded Debugging Operators  * * * * * * * * * * * * * * * * * */


    friend bool operator==(const personLink& lhs, const personLink& rhs);

    friend bool operator!=(const personLink& lhs, const personLink& rhs);

    friend bool operator==(const personName& lhs, const personName& rhs);

    friend bool operator!=(const personName& lhs, const personName& rhs);

    friend std::ostream& operator<<(std::ostream& os, const personLink& city);

    friend std::ostream& operator<<(std::ostream& os, const personName& city);

    friend std::ostream& operator<<(std::ostream&os, const Vector<personLink>& links);

    friend std::ostream& operator<<(std::ostream&os, const PartnerLinks& links);


    /* * * * * * * * * * * * *          Matching Solvers        * * * * * * * * * * * * * * * * * */


    /**
     * @brief hasPerfectLinks  determines if an instance of a PartnerLinks matrix can solve the
     *                         Perfect Match problem. A perfect matching is when every person is
     *                         partnered with someone.
     * @param pairs            the output parameter that shows the first perfect matching we found.
     * @return                 true if there is a perfect matching false if not.
     */
    bool hasPerfectLinks(Set<Pair>& pairs);

    /**
     * @brief getMaxWeightMatching  determines the Max Weight Matching of a PartnerLinks matrix. A
     *                              Max Weight Matching is the greatest sum of edge weights we can
     *                              acheive by partnering up people in a network.
     * @return                      the set representing the Max Weight Matching that we found.
     */
    Set<Pair> getMaxWeightMatching();

private:

    /* An instance of a Network can solve either the Perfect Matching or Max Weight Matching
     * problem. However, it must be given the correct information. If a Max Weight Matching is
     * desired, it must have the weights of every partnership in the network.
     */
    Network dlx;


    /* * * * * * * * * * * * *  Algorithm X via Dancing Links   * * * * * * * * * * * * * * * * * */


    /**
     * @brief isPerfectMatching  finds the first available Perfect Matching for a network. The set
     *                           is empty if no matching is found.
     * @param pairs              the output parameter of the found matching.
     * @return                   true if the matching is found, false if not.
     */
    bool isPerfectMatching(Set<Pair>& pairs);

    /**
     * @brief fillWeights  in the spirit of "no copy recursion" that Knuth has created with dancing
     *                     links, I use pass by reference to fill/remove from a soFar pair during
     *                     recursion. The winner pair will record the best weight found and keep
     *                     the Set as an output parameter.
     * @param soFar        the pair of weight and pairs we fill with every possible pairing.
     * @param winner       the pair of weight and pairs that records the best weight found so far.
     */
    void fillWeights(std::pair<int,Set<Pair>>& soFar,
                     std::pair<int,Set<Pair>>& winner);

    int choosePerson();
    int chooseWeightedPerson();

    Pair coverPairing(int index);
    std::pair<int,Pair> coverWeightedPair(int index);
    void coverPerson(int index);
    void uncoverPerson(int index);

    void uncoverPairing(int index);
    void hidePairings(personLink& start, int index);
    void unhidePairings(personLink&start, int index);
    inline void toPairIndex(int& index);


    void initializeHeaders(const Map<std::string, Set<std::string>>& possibleLinks,
                           HashMap<std::string,int>& columnBuilder);
    void initializeHeaders(const Map<std::string, Map<std::string,int>>& possibleLinks,
                           HashMap<std::string,int>& columnBuilder);
    void setPerfectPairs(const std::string& person,
                         const Set<std::string>& personPairs,
                         HashMap<std::string,int>& columnBuilder,
                         Set<Pair>& seenPairs,
                         int& index,
                         int& spacerTitle);
    void setWeightedPairs(const std::string& person,
                          const Map<std::string,int>& personPairs,
                          HashMap<std::string,int>& columnBuilder,
                          Set<Pair>& seenPairs,
                          int& index);

    ALLOW_TEST_ACCESS();
};

#endif // PartnerLinks_H
