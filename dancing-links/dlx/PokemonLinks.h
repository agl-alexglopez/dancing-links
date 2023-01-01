/**
 * Author: Alexander G. Lopez
 * File: PokemonLinks.h
 * --------------------------
 * This class defines a PokemonLinks object that can be used to solve the Pokemon Type Coverage
 * Problem with Donald Knuth's Algorithm X via Dancing Links. At the most abstract level, an exact
 * coverage is one in which we are able to cover each item in a set of items exactly once by
 * choosing options that contain those items. No two options that we pick can cover the same item.
 *
 * I apply this concept to the videogame Pokemon. In Pokemon, there are fundamental nature-related
 * types like Fire, Water, Flying, Fighting and many others. Depending on the Pokemon Generation
 * there can be anywhere from 15 to 18 of these single types. These types act as a large and
 * nuanced layer of complexity that goes over what is fundametally a game of rock-paper-scissors.
 * Types have advantages of over types or are weak to other types. Pokemon can themselves BE these
 * fundamental types. However, they may take on two types, such as Fire-Flying or Bug-Flying. While
 * these fundamental types could be combined to form 306 unique dual types, not to mention the
 * additional 15 to 18 single types, the developers of this game have not done so yet. Depending on
 * the generation there are far fewer types than this, but there are still many. The most recent
 * release is up to 162 unique pokemon types.
 *
 * For the full ruleset of Pokemon look elswhere, but in brief what I do in this problem is
 * determine if there are Perfect and/or Overlapping type coverages for given sets of Attack and
 * Defense types. Attack types can only be those 15 to 18 single types mentioned earlier an
 * defensive types are these sames types with the additional possiblity of combining two types.
 *
 * An exact cover for Defense uses the following rules to determine if such a cover is possible.
 *
 *      - We may pick a maximum of 6 Pokemon for a team.
 *      - We defend ourselves from 15 to 18 attack types, depending on the generation of Pokemon.
 *      - Pick at most 6 pokemon that have resistances to every attack type exactly once.
 *      - Resistance means that when an attack type is used on our chosen type it does no more
 *        than x0.5 the damage of a normal attack. So, across all of our chosen pokemon can we
 *        be resistant to every attack type exactly once?
 *      - Some Resistances are better than others. For example, it is possible for a type to be
 *        immune to other types x0.0, very resistant x0.25, or resistant x0.5. These resistances
 *        are scored with x0.0 being given 1 point, x0.25 given 2 points, and x0.5 given 3 points.
 *        The goal is to keep the lowest score possible, NOT the highest.
 *      - Viable teams that we find will be ranked according to the scoring system described above.
 *
 * An exact cover for Attack uses the following rules to determine if an exact cover is possible.
 *
 *      - Every pokemon in a team can learn 4 attacks.
 *      - Attacks are only of a single type. There are no attacks that do two simultaneous types of
 *        damage.
 *      - This means that we can choose 24 attacks. This will never happen because we are always
 *        limited by the maximum number of attack types (15 to 18). So effectively, there is no
 *        depth limit for this search as we can choose every attack at most.
 *      - Our choice of attack types must be effective against every defensive type we are tasked
 *        with damaging exactly once. No two attack types may be effective against the same
 *        defensive type.
 *      - Attacks may be super-effective in two ways against another type: they do double damage x2
 *        or quadruple damage x4. These multipliers are scored with x2 damage earning 5 points and
 *        x4 damage earning 6 points. A higher score is better in this case, NOT a lower score.
 *      - Viable attack type selections will be ranked by the scoring system described above.
 *
 * An overlapping cover is another way to look at both of these problems. An overlapping cover means
 * that we allow overlap in the options we choose covering some of the same items. For example,
 * in the defensive case, our goal is to simply choose at most 6 Pokemon that will resist all
 * attack types in the game, we do not care about how much overlap there is in coverage between
 * the defensive types we choose. We only care about being resistant to all damage types across our
 * entire team.
 *
 * For the Overlapping version of this problem, we use the exact same scoring schemes for Defense
 * and Attack. Overlapping cover will produce a tremendous number of possibilities in some cases
 * so I have limited how long this algorithm can run with a maximum output. I have run out of
 * memory when generating combinations before and could not tell you how many Overlapping covers
 * are possible for some Pokemon Generations. Exact cover, in contrast, is usually much more
 * reasonable to determine because it is much more difficult to achieve. For an even more detailed
 * writeup please see the README.md for this repository, where I use images to help describe the
 * process.
 *
 * For more information on Algorithm X via Dancing Links as Knuth describes it please read his work.
 *
 *      The Art of Computer Programming,
 *      Volume 4B,
 *      Combinatorial Algorithms, Part 2,
 *      Sec. 7.2.2.1,
 *      Pg.65-70,
 *      Knuth
 */
#ifndef POKEMONLINKS_H
#define POKEMONLINKS_H
#include <map>
#include <string>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>
#include "GUI/SimpleTest.h"
#include "Utilities/Resistance.h"
#include "Utilities/RankedSet.h"


class PokemonLinks {
public:

    /* Alter here if you want to change the rules of Pokemon. Normally, there are at most 6 pokemon
     * in a team and each pokemon can have 4 different attacks for a total of 24 slots to fill with
     * different attack types. Max output is a limit that will cut off the recursive algorithm if
     * it runs too long. For example, asking for overlapping defense coverages for all of Gen 9 will
     * run out of memory, so I cut everything off at 100,000 sets generated.
     */
    const std::size_t MAX_OUTPUT_SIZE=100000;
    const int MAX_TEAM_SIZE=6;
    const int MAX_ATTACK_SLOTS=24;

    // The user is asking us for the defensive team they should build or attacks they need.
    typedef enum CoverageType {
        DEFENSE,
        ATTACK
    }CoverageType;

    // This type is entered into our dancing links array for the in place recursive algorithm.
    typedef struct pokeLink {
        int topOrLen;
        int up;
        int down;
        Resistance::Multiplier multiplier; // x0.0, x0.25, x0.5, x1.0, x2, or x4 damage multipliers.
        int depthTag;                      // We use this to efficiently generate overlapping sets.
    }pokeLink;

    // This type, in a seperate vector, controls the base case of our recursion.
    typedef struct typeName {
        std::string name;
        int left;
        int right;
    }typeName;

    /**
     * @brief PokemonLinks            this constructor builds the necessary internal data structures
     *                                to run the exact cover via dancing links algorithm. We need
     *                                to build differently based on attack or defense.
     * @param typeInteractions        map of pokemon types and their resistances to attack types.
     * @param requestedCoverSolution  ATTACK or DEFENSE. Build a team or choose attack types.
     */
    explicit PokemonLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions,
                          const CoverageType requestedCoverSolution);

    /**
     * @brief PokemonLinks      this alternative constructor is helpful when choosing a defensive
     *                          team based on a subset of attack types. For example, we could build
     *                          defenses against the attack types present at specific gyms.
     * @param typeInteractions  the map of types and their defenses for a given generation.
     * @param attackTypes       the subset of attacks we must cover with choices of Pokemon teams.
     */
    explicit PokemonLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions,
                          const std::set<std::string>& attackTypes);

    /**
     * @brief getExactTypeCoverage  an exact type coverage is one in which every "option" we choose
     *                              to cover a given set of "items" will cover each item exactly
     *                              once. For example, if building a team of Pokemon to defend
     *                              against attack types, no two Pokemon in that team could be
     *                              resistant to the same attack types. Sets are ranked by the
     *                              following criteria:
     *
     *                                  For forming defensive teams we score types based on their
     *                                  resistance to each attack type as follows.
     *
     *                                      - x0.0 multiplier is 1 point.
     *                                      - x0.25 multiplier is 2 points.
     *                                      - x0.50 multiplier is 3 points.
     *                                      - x1.0 multiplier or higher is not considered.
     *
     *                                  The lower the score the stronger that typing is. For forming
     *                                  our choices of attack types we score based on their
     *                                  resistance to each attack type as follows.
     *
     *                                      - x2 multiplier is 4 points.
     *                                      - x4 multiplier is 5 points.
     *                                      - x1.0 or lower is not considered.
     *
     *                                  The higher the score the stronger those choices of attack
     *                                  types are for attacking the selected defensive types.
     * @return                      the set of Ranked Sets that form all solutions of exact cover.
     */
    std::set<RankedSet<std::string>> getExactTypeCoverage();

    /**
     * @brief getOverlappingTypeCoverage  an overlapping coverage in which we cover every "item"
     *                                    with our choices of "options." It is allowable for two
     *                                    options cover the same item twice, the goal is to cover
     *                                    the items with any allowable choices. The scoring scheme
     *                                    for generated sets is the same as described in the
     *                                    exact cover version of the problem.
     * @return                            the set of Ranked Sets that form all overlapping covers.
     */
    std::set<RankedSet<std::string>> getOverlappingTypeCoverage();

    /**
     * @brief reachedOutputLimit  for usability of Pokemon Planning application I cut off output at
     *                            a set limit because sets can exceed the size of available memory
     *                            in some cases. Use this function if you want to know if the last
     *                            query to dancing links reached the limit and you are missing
     *                            all possible sets.
     * @return                    true if the last query reached the limit, false if not.
     */
    bool reachedOutputLimit();


    /* * * * * * * * * * * * *  Overloaded Debugging Operators  * * * * * * * * * * * * * * * * * */


    friend bool operator==(const pokeLink& lhs, const pokeLink& rhs);

    friend bool operator!=(const pokeLink& lhs, const pokeLink& rhs);

    friend bool operator==(const typeName& lhs, const typeName& rhs);

    friend bool operator!=(const typeName& lhs, const typeName& rhs);

    friend std::ostream& operator<<(std::ostream& os, const pokeLink& pokeLink);

    friend std::ostream& operator<<(std::ostream& os, const typeName& pokeLink);

    friend std::ostream& operator<<(std::ostream& os, const std::vector<pokeLink>& links);

    friend std::ostream& operator<<(std::ostream& os, const std::vector<typeName>& items);

    friend std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& options);

    friend std::ostream& operator<<(std::ostream& os, const std::set<RankedSet<std::string>>& solution);

    friend std::ostream& operator<<(std::ostream& os, const PokemonLinks& links);

private:


    /* * * * * * * * * * *   Dancing Links Internals and Implementation   * * * * * * * * * * * * */


    /* These data structures contain the core logic of Algorithm X via dancing links. For more
     * detailed information, see the tests in the implementation. These help use acheive in place
     * recursion.
     */
    std::vector<std::string> optionTable_;  // How we know the name of the option we chose.
    std::vector<typeName> itemTable_;       // How we know the names of our items
    std::vector<pokeLink> links_;           // The links that dance!
    int depthLimit_;
    int numItems_;
    int numOptions_;
    CoverageType requestedCoverSolution_;
    bool hitLimit_;

    /**
     * @brief fillExactCoverages  fills the output parameters with every exact cover that can be
     *                            determined for defending against attack types or attacking
     *                            defensive types. Exact covers use options to cover every item
     *                            exactly once.
     * @param exactCoverages      the output parameter that serves as the final solution.
     * @param coverage            the successfully coverages we find while the links dance.
     * @param depthLimit          size of a pokemon team or the number of attacks a team can have.
     */
    void fillExactCoverages(std::set<RankedSet<std::string>>& exactCoverages,
                            RankedSet<std::string>& coverage,
                            int depthLimit);

    /**
     * @brief fillOverlappingCoverages  fills the output parameter with every overlapping cover that
     *                                  can be determined for defending against attack types or
     *                                  attacking defensive types. Overlapping covers use any
     *                                  number of options within their depth limit to cover all
     *                                  items. Two options covering some overlapping items is
     *                                  acceptable. This is slower and I have no way to not generate
     *                                  duplicates other than using a set. Better ideas wanted.
     * @param overlappingCoverages      the output parameter as our final solution if found.
     * @param coverage                  the helper set that fills the output parameter.
     * @param depthTag                  a tag used to signify the recursive depth. Used internally.
     */
    void fillOverlappingCoverages(std::set<RankedSet<std::string>>& overlappingCoverages,
                                  RankedSet<std::string>& coverage,
                                  int depthTag);

    /**
     * @brief chooseItem  choose an item to cover that appears the least across all options. If an
     *                    item becomes inaccessible over the course of recursion I signify this by
     *                    returning -1. That branch should fail at that point.
     * @return            the index in the lookup table and headers of links_ of the item to cover.
     */
    int chooseItem() const;

    /**
     * @brief coverType      perform an exact cover as described by Donald Knuth, eliminating the
     *                       option we have chosen, covering all associated items, and eliminating
     *                       all other options that include those covered items.
     * @param indexInOption  the index in the array we use to start covering and eliminating links.
     * @return               every option we choose contributes to the strength of the RankedSet
     *                       it becomes a part of. Return the strength contribution to the set
     *                       and the name of the option we chose.
     */
    std::pair<int,std::string> coverType(int indexInOption);

    /**
     * @brief uncoverType    undoes the work of the exact cover operation returning the option,
     *                       the items it covered, and all other options that include the items we
     *                       covered back into the links.
     * @param indexInOption  the work will be undone for the same option if given same index.
     */
    void uncoverType(int indexInOption);

    /**
     * @brief hideOptions    takes the options containing the option we chose out of the links. Do
     *                       this in order to cover every item exactly once and not overlap. This
     *                       is the vertical traversal of the links.
     * @param indexInOption  the index we start at in a given option.
     */
    void hideOptions(int indexInOption);

    /**
     * @brief unhideOptions  undoes the work done by the hideOptions operation, returning the other
     *                       options containing covered items in an option back into the links.
     * @param indexInOption  the work will be undone for the same option if given same index.
     */
    void unhideOptions(int indexInOption);

    /**
     * @brief overlappingCoverType  performs a loose or "overlapping" cover of items in a dancing
     *                              links algorithm. We allow other options that cover items already
     *                              covered to stay accessible in the links leading to many more
     *                              solutions being found as multiple options can cover some of the
     *                              same items.
     * @param indexInOption         the index in array used to start covering and eliminating links.
     * @param depthTag              to perform this type of coverage I use a depth tag to know which
     *                              items have already been covered in an option and which still
     *                              need coverage.
     * @return                      the score our option contributes to its RankedSet and name.
     */
    std::pair<int,std::string> overlappingCoverType(int indexInOption, int depthTag);

    /**
     * @brief overlappingUncoverType  undoes the work of the loos cover operation. It uncovers items
     *                                that were covered by an option at the same level of recursion
     *                                in which they were covered, using the depth tags to note
     *                                levels.
     * @param indexInOption           the same index as cover operation will uncover same items.
     */
    void overlappingUncoverType(int indexInOption);


    /* * * * * * * * * * *   Dancing Links Instantiation and Building     * * * * * * * * * * * * */


    /**
     * @brief buildDefenseLinks  defensive links have all typings for a generation as options and
     *                           all single attack types as items. We will build these links along
     *                           with auxillary vectors that help control recursion and record the
     *                           names of the items and options.
     * @param typeInteractions   the map of interactions and resistances between types in a gen.
     */
    void buildDefenseLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions);

    /**
     * @brief buildAttackLinks  attack links have all single attack types for a generation as
     *                          options and all possible Pokemon typings as items in the links.
     * @param typeInteractions  the map of interactions and resistances between types in a gen.
     */
    void buildAttackLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions);

    /**
     * @brief initializeColumns  helper to build the options in our links and the appearances of the
     *                           items across these options.
     * @param typeInteractions   the map of interactions and resistances between types in a gen.
     * @param columnBuilder      the helper data structure to build the columns.
     * @param requestedCoverage  requested coverage to know which multipliers to pay attention to.
     */
    void initializeColumns(const std::map<std::string,std::set<Resistance>>& typeInteractions,
                           std::unordered_map<std::string,int>& columnBuilder,
                           CoverageType requestedCoverage);

    // Dancing links is well suited to internal debugging over just plain unit testing.
    ALLOW_TEST_ACCESS();
};

#endif // POKEMONLINKS_H
