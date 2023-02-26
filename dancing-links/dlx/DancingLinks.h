/**
 * Author: Alexander G. Lopez
 * File DisasterLinks.h
 * --------------------
 * This file contains an interface to interact with classes in the DancingLinks namespace. The
 * reason there is multiple classes with similar properties is because I experimented with various
 * implementation details for the Dancing Links object internals and wanted to be able to compare
 * usage and performance. It would have been just as easy to unite the DancingLinks solver into one
 * abstract class, but that was not the purpose of this repository. I learned alot from trying out
 * different optimization to try to solve each problem in the best way I could think of. Below I
 * will include an explanation of each type of problem these classes address. For more details and
 * images, see the README.md for this repository.
 *
 *
 * DisasterLinks
 * -------------------
 *
 * A city is covered if it has supplies or is adjacent to a city with supplies. A city cannot be
 * supplied twice but if a city is safe due to being adjacent to a city with supplies it can still
 * receive supplies.
 *
 * I was required to heavily modify Donald Knuth's implemntation of Algorithm X via Dancing Links
 * to fit this problem. Here is the basic logic behind my choices:
 *
 * Organize the network by all the cities that exist along the top row and all the options for
 * covering them in rows beneath this. Take this world for example.
 *
 *             C
 *             |
 *        A -- D -- B -- F
 *                  |
 *                  E
 *
 * We can represent these connections with the following matrix.
 *
 *            A  B  C  D  E  F    <-Cities as items that exist in the world
 *         B     1     1  1  1    <-The cities that a city will cover if supplied
 *         D  1  1  1  1
 *         A  1        1
 *         C        1  1
 *         E     1        1
 *         F     1           1
 *
 * Notice that a city covers itself. This is important in the case of island cities that are
 * connected to no cities. I have also organized the rows so that the cities with the
 * most connections are at the top and will be explored first. We will now discuss the selection
 * process.
 *
 * We must distribute two supplies. Select the city to cover that is the most isolated in the
 * network. It will have the lowest number of appearances across all sets. The (*) marks our
 * selection.
 *
 *            *
 *            2  4  1  4  2  2
 *            A  B  C  D  E  F
 *         B     1     1  1  1
 *         D  1  1  1  1
 *         A  1        1
 *         C        1  1
 *         E     1        1
 *         F     1           1
 *
 * It is very likely that supplying the most connected city adjacent to the isolated city to supply
 * will be the best choice, so we will try to cover A this way first. Supply city D and make all
 * the cities attached to it safe. This is how our network shrinks.
 *
 *            E  F
 *         B  1  1
 *         A
 *         C
 *         E  1
 *         F    1
 *
 * Notice that A and C no longer can help this network as supply locations. The only helpful options
 * are B E and F. Luckily our selection heuristic will select to cover E with option B.
 *
 *           0
 *         A
 *         C
 *         E
 *         F
 *
 * Thus our world is empty and the problem is solved by supplying cities B and D when given two
 * supplies. If we were given one supply we would not have been able cover this network.
 *
 *
 * DisasterTags
 * -------------------
 *
 * A city is covered if it has supplies or is adjacent to a city with supplies. A city cannot be
 * supplied twice but if a city is safe due to being adjacent to a city with supplies it can still
 * receive supplies.
 *
 * I was required to heavily modify Donald Knuth's implemntation of Algorithm X via Dancing Links
 * to fit this problem. Here is the basic logic behind my choices:
 *
 * Organize the network by all the cities that exist along the top row and all the options for
 * covering them in rows beneath this. Take this world for example.
 *
 *             C
 *             |
 *        A -- D -- B -- F
 *                  |
 *                  E
 *
 * We can represent these connections with the following matrix.
 *
 *            A  B  C  D  E  F    <-Cities as items that exist in the world
 *         B     1     1  1  1    <-The cities that a city will cover if supplied
 *         D  1  1  1  1
 *         A  1        1
 *         C        1  1
 *         E     1        1
 *         F     1           1
 *
 * Notice that a city covers itself. This is important in the case of island cities that are
 * connected to no cities. I have also organized the rows so that the cities with the
 * most connections are at the top and will be explored first. We will now discuss the selection
 * process.
 *
 * We must distribute two supplies. Select the city to cover that is the most isolated in the
 * network. It will have the lowest number of appearances across all sets. The (*) marks our
 * selection. When we supply a city all of the associated cities in whichever supply option we
 * choose get a supply number tag. It is the same as the number of supplies remaining before we
 * give out that supply.
 *
 *            *
 *    Tag --> 2  2  2  2
 *            A  B  C  D  E  F
 *         B     1     1  1  1
 *    Tag --> 2  2  2  2
 *         D  1  1  1  1
 *         A  1        1
 *         C        1  1
 *         E     1        1
 *         F     1           1
 *
 * It is very likely that supplying the most connected city adjacent to the isolated city to supply
 * will be the best choice, so we will try to cover A this way first. Supply city D and make all
 * the cities attached to it safe. This is how our network shrinks.
 *
 *            E  F
 *         B  1  1
 *         A
 *         C
 *         E  1
 *         F    1
 *
 * Notice that A and C no longer can help this network as supply locations. The only helpful options
 * are B E and F. Luckily our selection heuristic will select to cover E with option B.
 *
 *    Tag --> 1  1
 *            E  F
 *    Tag --> 1  1
 *         B  1  1
 *         A
 *         C
 *         E  1
 *         F    1
 *
 *           0
 *         A
 *         C
 *         E
 *         F
 *
 * Thus our world is empty and the problem is solved by supplying cities B and D when given two
 * supplies. If we were given one supply we would not have been able cover this network. Had we
 * needed to take back the supplies from a supply location we would use the supply tag to only
 * uncover all associated cities with that supply tag. This saves a meaningful amount of work
 * when compared to an implementation that uses quadruply linked lists with up,down,left, and right
 * fields.
 *
 *
 * Perfect Matching:
 * -------------------
 *
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
 *
 * Weighted Matching:
 * ------------------
 *
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
 *
 * If you wish to see how these implementations vary from Knuth's original please read
 *
 *      The Art of Computer Programming,
 *      Volume 4B,
 *      Combinatorial Algorithms, Part 2,
 *      Sec. 7.2.2.1,
 *      Pg.65-70,
 *      Knuth
 */
#ifndef DANCINGLINKS_H
#define DANCINGLINKS_H
#include <string>
#include <set>
#include <vector>
#include "Src/MatchingUtilities.h"
#include "Src/DisasterLinks.h"
#include "Src/DisasterTags.h"
#include "Src/PartnerLinks.h"


namespace DancingLinks {
class DisasterLinks;
class DisasterTags;
class PartnerLinks;

/**
 * @brief hasOverlappingCover  determines if it is possible to cover a set of items with options
 *                             that cover various combinations of those items. An overlapping cover
 *                             differs from an exact cover in that it allows for multiple options
 *                             to cover the same items. The goal is to simply cover the items with
 *                             any available combination of options.
 * @param links                the dancing links object we take in to perform our cover search.
 * @param depthLimit           the limit we place on how many options we can choose.
 * @param selectedOptions      the options we select as proof of our cover search.
 * @return                     true if solution false if not. Output param is full for true.
 */
bool hasOverlappingCover(DisasterLinks& links, int depthLimit,
                           std::set<std::string>& selectedOptions);

/**
 * Overloaded version of the above function for a DisasterTags object.
 */
bool hasOverlappingCover(DisasterTags& links, int depthLimit,
                           std::set<std::string>& selectedOptions);

/**
 * @brief getAllOverlappingCovers  finds every possible way to cover items with the provided options
 *                                 in a DisasterLinks object. Overlapping covers allow for multiple
 *                                 options to cover the same items.
 * @param links                    the dancing links class on which we perform a cover search.
 * @param depthLimit               the limit on the number of options we can choose.
 * @return                         the set of sets of options we chose.
 */
std::set<std::set<std::string>> getAllOverlappingCovers(DisasterLinks& links, int depthLimit);

/**
 * Overloaded version of the above function for a DisasterTags object.
 */
std::set<std::set<std::string>> getAllOverlappingCovers(DisasterTags& links, int depthLimit);

/**
 * @brief hasExactCover    determines if an exact cover is possible given the items and options
 *                         available to cover those items. An exact cover is one where the options
 *                         we choose cover every item exactly once across all options. This is
 *                         different from an overlapping cover that allows for multiple options to
 *                         cover the same items.
 * @param links            the dancing links class on which we perform an exact cover search.
 * @param selectedOptions  the options we have selected that satisfy an exact cover of items.
 * @return                 true if the output parameter is full and we found a cover, false if not.
 */
bool hasExactCover(PartnerLinks& links, std::set<Pair>& selectedOptions);

/**
 * @brief getAllExactCovers  finds every possible exact cover of the given items with the provided
 *                           options. Because this is intended for the perfect matching problem on
 *                           a PartnerLinks object, no depth limit is required as everyone must
 *                           be paired off.
 * @param links              the PartnerLinks object on which we perform an exact cover search.
 * @return                   the vector of sets of Pairs that satisfy an exact cover.
 */
std::vector<std::set<Pair>> getAllExactCovers(PartnerLinks& links);

/**
 * @brief getMaxWeightMatching  finds the maximum possible weight matching of items given the
 *                              options to match or "cover" those items.
 * @param links                 the PartnerLinks object on which we search for max weight matching.
 * @return                      the set of Pairs with maximum possible weight.
 */
std::set<Pair> getMaxWeightMatching(PartnerLinks& links);


} // namespace DancingLinks


#endif // DANCINGLINKS_H
