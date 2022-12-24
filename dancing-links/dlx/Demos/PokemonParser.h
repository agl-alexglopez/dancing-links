#ifndef POKEMONPARSER_H
#define POKEMONPARSER_H
#include <map>
#include <set>
#include <string>
#include <vector>
#include "DisasterParser.h"
#include "PokemonUtilities.h"

/* Leave a comment at the first line of the Pokemon Generation .dst file you want to construct with
 * the pokemon generation in base 10 numbers. This will determine the types availabe for the given
 * map and generation cover problem. Any of the 9 generations are acceptable. If the
 * first line does not match one of these strings I will just include all pokemon types in the cover
 * problem. Like this.
 *
 * # 1
 * # 2
 * # 3
 * # 4
 * # 5
 * # 6
 * # 7
 * # 8
 * # 9
 *
 * This also means you should start every file with some sort of comment labelling it with
 * identifying info, even if there is not generation specification. Otherwise, the behavior of this
 * parsing is undefined.
 */


typedef struct PokemonTest {
    /* This map will hold all types--dual types included--and a map of defense multipliers ranging
     * from x0.0,x0.25,x0.5,x1.0,x2.0,x4.0. In these maps will be the multiplier as the key and
     * all the single attack types that have that multiplier against the current type. There are
     * 18 single attack types. The highest level map holds 171 type keys that exist in Pokemon as
     * of December 2022. However, depending on which generation map you decide to draw some types
     * might be missing. For example, generation one Pokemon did not have types like Fairy, Dark,
     * or Steel.
     */
    std::map<std::string,std::set<Resistance>> typeInteractions;
    DisasterTest pokemonGenerationMap;
}PokemonTest;


PokemonTest loadPokemonGeneration(std::istream& source);

#endif // POKEMONPARSER_H
