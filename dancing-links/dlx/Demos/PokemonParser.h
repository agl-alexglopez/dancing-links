/**
 * Author: Alex Lopez
 * File: PokemonParser.h
 * ---------------------
 * This file handles getting Pokemon Data from JSON files and turning it into C++ std compliant
 * data forms like maps and sets. This implementation is heavily dependent on QT libraries for
 * parsing the JSON. I chose to turn the data into C++ std forms to use with the program, even
 * though it would have been possible to just read the data from the files in directly during the
 * program runtime, in case QT becomes deprecated or I want to move this to a CLI implementation.
 * Then, I just need to rewrite the parser that gets me data from JSON files rather than rewrite
 * the whole program.
 */
#ifndef POKEMONPARSER_H
#define POKEMONPARSER_H
#include <map>
#include <set>
#include <string>
#include <vector>
#include "MapParser.h"
#include "set.h"
#include "Utilities/Resistance.h"

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

struct PokemonTest {
    /* This map will hold all types--dual types included--and a map of defense multipliers ranging
     * from x0.0,x0.25,x0.5,x1.0,x2.0,x4.0. In these maps will be the type as the key and
     * all the single attack types that have that multiplier against the current type. There are
     * 18 single attack types. The highest level map holds 162 type keys that exist in Pokemon as
     * of December 2022. However, depending on which generation map you decide to draw some types
     * might be missing. For example, generation one Pokemon did not have types like Fairy, Dark,
     * or Steel.
     */
    std::map<std::string,std::set<Resistance>> typeInteractions;
    MapTest pokemonGenerationMap;
};

/**
 * @brief loadPokemonGeneration  builds the PokemonTest needed to interact with a generation's map
 *                               in the Pokemon Planning GUI.
 * @param source                 the file with the map that gives us info on which gen to build.
 * @return                       the completed pokemon test with map drawing and Pokemon info.
 */
PokemonTest loadPokemonGeneration(std::istream& source);

/**
 * @brief loadSelectedGymsDefense  the user in the GUI may want cover problems solved for selected
 *                                 gyms. We can take back the generation info map we created and
 *                                 alter it to give back only the types requested and their attack
 *                                 weaknesses. I have stored the info for every gym for every map
 *                                 in the all-maps.json file. I can use this to know defense and
 *                                 attack types for every gym. If you want to add a map to the
 *                                 collection, be sure to also input its gym leaders and elite four
 *                                 in this file in the format already present.
 * @param currentGenInteractions   the map we have generated for the user with original type info.
 * @param selectedMap              the current .dst map the user is interacting with.
 * @param selectedGyms             the selection of gyms they would like to attack.
 * @return                         the map of the defensive types for the selected gyms.
 */
std::map<std::string,std::set<Resistance>>
loadSelectedGymsDefense(const std::map<std::string,std::set<Resistance>>& currentGenInteractions,
                        const std::string& selectedMap,
                        const Set<std::string>& selectedGyms);

/**
 * @brief loadSelectedGymsAttacks  the user interacting with the GUI may want to defend themselves
 *                                 from only a selection of gyms. We will get the gym info for the
 *                                 selected map and return the types of attacks present across all
 *                                 of those selections. This set can then be passed to the
 *                                 PokemonLinks dancing links class as a second parameter.
 * @param selectedMap              the current map the user interacts with.
 * @param selected                 the gyms they have selected.
 * @return                         a set of all attack types present across those gyms.
 */
std::set<std::string> loadSelectedGymsAttacks(const std::string& selectedMap,
                                              const Set<std::string>& selected);

#endif // POKEMONPARSER_H
