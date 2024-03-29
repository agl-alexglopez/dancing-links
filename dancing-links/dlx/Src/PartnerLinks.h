/**
 * Author: Alexander G. Lopez
 * File: PartnerLinks.h
 * --------------------------
 * This file contains the class I use to implement Algorithm X via Dancing Links. This algorithm
 * is adjusted and used to solve the Perfect Matching and Max Weight Matching problems on a graph.
 * If only perfect matching is desired give the class constructor a std::map<string,std::set<string>> with
 * a person and their partners. If you want the max weight matching of a graph, provide a
 * std::map<string,std::map<string,int>> to the constructor with a person and the weights of their partners.
 * You can ask a weighted or unweighted graph for perfect matching, but if you did not provide
 * weight information, you cannot ask for the max weight matching and must construct a new instance
 * of the class with that information.
 */
#ifndef PartnerLinks_H
#define PartnerLinks_H
#include <map>
#include <set>
#include <unordered_map>
#include <string>
#include <vector>
#include "GUI/SimpleTest.h"
#include "MatchingUtilities.h"

namespace DancingLinks {

class PartnerLinks {

public:

    /* * * * * *   Algorithm X via Dancing Links for Perfect and Max-Weight Matching  * * * * * * */


    /**
     * @brief PartnerLinks   the constructor for a world intended to check for Perfect Matching.
     *                       Provide a map of the person, and the set of people they are willing to
     *                       pair with. This will then build a dancing link matrix in order to solve
     *                       future queries.
     * @param possibleLinks  the map of people and partners they are willing to work with.
     */
    explicit PartnerLinks(const std::map<std::string, std::set<std::string>>& possibleLinks);

    /**
     * @brief PartnerLinks   the constructor for a world intended to check for Max Weight Matching.
     *                       Provide a map of the person, and the map of the partners they are
     *                       willing to pair with and the weights of each partnership. If a negative
     *                       weight is given for a partnership, we treat that partnership as if it
     *                       does not exist.
     * @param possibleLinks  the map of people and map of partners and weights.
     */
    explicit PartnerLinks(const std::map<std::string, std::map<std::string, int>>& possibleLinks);

    /**
     * @brief hasPerfectLinks  determines if an instance of a PartnerLinks matrix can solve the
     *                         Perfect Match problem. A perfect matching is when every person is
     *                         partnered with someone.
     * @param pairs            the output parameter that shows the first perfect matching we found.
     * @return                 true if there is a perfect matching false if not.
     */
    bool hasPerfectLinks(std::set<Pair>& pairs);

    /**
     * @brief getAllPerfectLinks  retrieves every configuration of a graph of people that will
     *                            produce a Perfect Matching. This is not every possible pairing
     *                            of people, rather, only those pairings that will produce
     *                            Perfect Matching.
     * @return                    vector of sets. Each is a unique Perfect Matching configuration.
     */
    std::vector<std::set<Pair>> getAllPerfectLinks();

    /**
     * @brief getMaxWeightMatching  determines the Max Weight Matching of a PartnerLinks matrix. A
     *                              Max Weight Matching is the greatest sum of edge weights we can
     *                              acheive by partnering up people in a network.
     * @return                      the set representing the Max Weight Matching that we found.
     */
    std::set<Pair> getMaxWeightMatching();




private:


    /* These are the nodes that live in our matrix representation of the network. If the node is
     * a header for a column topOrLen is the number of options that a person appears in. If the node
     * is in the column topOrLen is the index of the header for that column. Up down pointers are
     * indices in the array. The left right pointers are implicit because these nodes are next to
     * one another in an array.
     */
    struct personLink {
        int topOrLen;
        int up;
        int down;
    };

    /* These nodes control recursion in a seperate array. As partnerships are chosen or individual
     * people are chosen their representation in the lookup array is spliced out of a doubly linked
     * list.
     */
    struct personName {
        std::string name;
        int left;
        int right;
    };



    /* An instance of a Network can solve either the Perfect Matching or Max Weight Matching
     * problem. However, it must be given the correct information. If a Max Weight Matching is
     * desired, it must have the weights of every partnership in the network.
     */
    std::vector<personName> table_;
    std::vector<personLink> links_;
    int numPeople_;                  // Total people in the network.
    int numPairings_;                // The number of pairings or rows in the matrix.
    bool hasSingleton_;              // No perfect matching if someone is alone.
    bool isWeighted_;                // Must provide weights to ask for max weight matching.


    /* * * * * * * * * * * *    Core Functionality for Algorithm X     * * *  * * * * * * * * * * */


    /**
     * @brief isPerfectMatching  finds the first available Perfect Matching for a network. The set
     *                           is empty if no matching is found.
     * @param pairs              the output parameter of the found matching.
     * @return                   true if the matching is found, false if not.
     */
    bool isPerfectMatching(std::set<Pair>& pairs);

    /**
     * @brief fillPerfectMatchings  finds all available Perfect Matchings for a network. Fills the
     *                              sets that complete this task as pass by reference output
     *                              parameters. Every Perfect Matching configuration is unique.
     * @param soFar                 the helper set we insert/delete from as we build Matchings.
     * @param result                the output parameter we fill with any Perfect Matchings we find.
     */
    void fillPerfectMatchings(std::set<Pair>& soFar, std::vector<std::set<Pair>>& result);

    /**
     * @brief fillWeights  recusively finds the maximum weight pairings possible given a dancing
     *                     links network with weighted partners. Uses the soFar set to store all
     *                     possible pairing combinations while the winner output parameter tracks
     *                     snapshots of the best weight and pairs found so far.
     * @param soFar        the pair of weight and pairs we fill with every possible pairing.
     * @param winner       the pair of weight and pairs that records the best weight found.
     */
    void fillWeights(std::pair<int,std::set<Pair>>& soFar, std::pair<int,std::set<Pair>>& winner);

    /**
     * @brief choosePerson  chooses a person for the Perfect Matching algorithm. It will simply
     *                      select the next person avaialable with no advanced heuristics. However,
     *                      it first checks if anyone has been isolated due to previous parings.
     *                      We willl then know we should stop recursion because someone is alone and
     *                      has no pairing possibilities.
     * @return              the index of the next person to pair or -1 if someone is alone.
     */
    int choosePerson() const;

    /**
     * @brief chooseWeightedPerson  choosing a person in Max Weight matching is different than
     *                              Perfect Matching. It is ok if someone cannot be reached by
     *                              partners. We will select next available person who has partners
     *                              available. If everyone that has not been matched has no possible
     *                              connections, that is ok, but we will return -1 to indicate this.
     * @return                      the index of the next person to pair or -1 if no pairs remain.
     */
    int chooseWeightedPerson() const;

    /**
     * @brief coverPairing  when we cover a pairing in a Perfect Matching we report back the
     *                      partnership as a Pair. This is helpful for proving the recursive
     *                      selections are correct. This selects the option beneath the index given.
     *                      Covering a pair means that both people will dissapear from all other
     *                      partnerships they could have matched with, eliminating those options.
     * @param indexInPair   the index of the pair we want to cover.
     * @return              the pair we have created by selecting this option.
     */
    Pair coverPairing(int indexInPair);

    /**
     * @brief uncoverPairing  uncovers a pairing that was hidden in Perfect Matching or Max Weight
     *                        Matching. The uncovering process is identical across both algorithms.
     *                        Be sure to provide the same index as the option you covered.
     * @param indexInPair     the same index as the index that was covered.
     */
    void uncoverPairing(int indexInPair);

    /**
     * @brief hidePersonPairings  hides other options that include the specified person. This only
     *                            hides pairings including a single person. If you want to hide both
     *                            people in a pair, you must use this on both people.
     * @param indexInPair         the index of the person we are hiding in the selected option.
     */
    void hidePersonPairings(int indexInPair);

    /**
     * @brief unhidePersonPairings  undoes the work of hidePersonPairings if given the same start
     *                              and index.
     * @param indexInPair           the index of the person we are unhiding in the selected option.
     */
    void unhidePersonPairings(int indexInPair);

    /**
     * @brief coverWeightedPair  when we cover a weighted pair in Max Weight Matching we should
     *                           report back the weight of the pair and the names of the people in
     *                           the Pair. This is helpful for proof of correct choices. Covering a
     *                           Pair means that both people disappear from all other possible
     *                           pairings, thus eliminating those options.
     * @param indexInPair        the index of the weighted pair we cover.
     * @return                   an std::pair of the weight of the pair and their names.
     */
    std::pair<int,Pair> coverWeightedPair(int indexInPair);

    /**
     * @brief hidePerson   to generate all possible pairings in any pairing algorithm, we need to
     *                     include every person in future possible pairings and exclude them. To
     *                     exclude a person, we will cover only that person. Instead of eliminating
     *                     every option that includes both people in a Pair, we only eliminate
     *                     other appearances of this individual in other pairings. It is a subtle
     *                     but important difference from covering a pairing.
     * @param indexInPair  the index of the person we cover.
     */
    void hidePerson(int indexInPair);

    /**
     * @brief unhidePerson  undoes the work of covering a person, reinstating all possible pairings
     *                      that include this person. Will undo the same option chosen in
     *                      hidePerson() if given the same index.
     * @param indexInPair   the index of the person to uncover.
     */
    void unhidePerson(int indexInPair);

    /**
     * @brief toPairIndex  helper function to increment the index to the next partner. We might
     *                     need to move left or right.
     * @param indexInPair  the current partner.
     */
    inline int toPairIndex(int indexInPair);


    /* * * * * * * * * *   Build and Initialize Dancing Links Data Structure    * * * * * * * * * */


    /**
     * @brief initializeHeaders  initializes the headers of the dancing link data structure if
     *                           given a map of pairs with no weights.
     * @param possibleLinks      the map of unweighted connections for every person in the graph.
     * @param columnBuilder      the helping data structure we use to build columns in one array.
     */
    void initializeHeaders(const std::map<std::string, std::set<std::string>>& possibleLinks,
                           std::unordered_map<std::string,int>& columnBuilder);

    /**
     * @brief initializeHeaders  initializes the headers of the dancing link data structure if
     *                           given a map of pairs with weights. Negative weights are ignored
     *                           and no pairing is made for that negative match.
     * @param possibleLinks      the map of weighted matches for every person in the graph.
     * @param columnBuilder      the helping data structure we use to build columns in one array.
     */
    void initializeHeaders(const std::map<std::string, std::map<std::string,int>>& possibleLinks,
                           std::unordered_map<std::string,int>& columnBuilder);

    /**
     * @brief setPerfectPairs  creates the internal rows and columns of the dancing links data
     *                         structure. Perfect pairs do not have any weight information and
     *                         options will simply be given a number from 1 to N number of matches.
     * @param person           the person who will set all possible matches for.
     * @param preferences      the set of all people this person is willing to pair with.
     * @param columnBuilder    the structure we use to track and build all columns in one array.
     * @param seenPairs        helper set to keep pairings unique and bidirectional.
     * @param index            we advance the index as an output parameter.
     * @param spacerTitle      we advance the spacerTitle to number each option.
     */
    void setPerfectPairs(const std::string& person,
                         const std::set<std::string>& preferences,
                         std::unordered_map<std::string,int>& columnBuilder,
                         std::set<Pair>& seenPairs,
                         int& index,
                         int& spacerTitle);

    /**
     * @brief setWeightedPairs  creates the internal rows and columns of the dancing links data
     *                          structure. Weighted Pairs have weights for each partnership that
     *                          are placed as negative topOrLen fields in the spacer for each row.
     *                          We can then get the weight and names of any partnership while
     *                          recursing easily. Negative weights are ignored and no pairing made.
     * @param person            the person who's weighted matches we will set.
     * @param preferences       the people this person can pair with and the weight of those pairs.
     * @param columnBuilder     the structure we use to track and build all columns in one array.
     * @param seenPairs         helper set to keep pairings unique and bidirectional.
     * @param index             we advance the index as an output parameter.
     */
    void setWeightedPairs(const std::string& person,
                          const std::map<std::string,int>& preferences,
                          std::unordered_map<std::string,int>& columnBuilder,
                          std::set<Pair>& seenPairs,
                          int& index);


    /* * * * * * * * * * * * *  Overloaded Debugging Operators  * * * * * * * * * * * * * * * * * */


    friend bool operator==(const personLink& lhs, const personLink& rhs);
    friend bool operator!=(const personLink& lhs, const personLink& rhs);
    friend bool operator==(const personName& lhs, const personName& rhs);
    friend bool operator!=(const personName& lhs, const personName& rhs);
    friend std::ostream& operator<<(std::ostream& os, const personLink& city);
    friend std::ostream& operator<<(std::ostream& os, const personName& city);
    friend std::ostream& operator<<(std::ostream&os, const std::vector<personLink>& links);
    friend std::ostream& operator<<(std::ostream&os, const std::vector<personName>& table);
    friend std::ostream& operator<<(std::ostream&os, const PartnerLinks& links);
    // I need to test the internals of the dlx instance so leave this here.
    ALLOW_TEST_ACCESS();
};

} // namespace DancingLinks

#endif // PartnerLinks_H
