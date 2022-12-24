/**
 * Author: Alex Lopez
 * File: htmlToJSON.cpp
 * --------------------
 * This file takes an html webpage and converts it to a JSON format that is more portable. I want
 * typing information for every type combination in the pokemon games not just the 18 core single
 * types. The JSON format will be as follows, where the keys within a type name are the amount
 * of damage that the specific type will take from other pokemon types. In this game you can take
 * no damage (x0), quarter damage (x.25), half damage (x.5), normal damage (x1.0), double damage
 * (x2), or quad damage (x4), depending on the typing. Only dual types can receive quarter or quad
 * damage based on weaknesses or resistances.
 *
 *      {
 *          "type-name": {
 *              "immune": [
 *                  ...
 *              ],
 *              "quarter": [
 *                  ...
 *              ],
 *              "half": [
 *                  ...
 *              ],
 *              "normal": [
 *                  ...
 *              ],
 *              "double": [
 *                  ...
 *              ],
 *              "quad": [
 *                  ...
 *              ]
 *          },
 *          "type-name2": {
 *              ...
 *          },
 *          ...
 *          "last-name": {
 *              ...
 *          }
 *      }
 *
 * It will be good to have this format as a resource when building C++ maps for the data later.
 * Also, this is a helpful format for the data in general as I have not found anything like this
 * elsewhere.
 */
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <vector>


namespace {
    /* Unordered pair of strings. I know this is an exact duplicate of the MatchingUtilies.h class
     * but this is more portable. Copy this implementation into any folder in most terminals and
     * and it should work regardless of OS.
     */
    class DualType {
    public:
        /* Creates a pair of two empty strings. */
        DualType() = default;

        /* Sort the strings. */
        DualType(const std::string& one, const std::string& two) {
            if (one < two) {
                one_ = one;
                two_ = two;
            } else {
                one_ = two;
                two_ = one;
            }
        }

        const std::string& first()  const {
            return one_;
        }
        const std::string& second() const {
            return two_;
        }

        // Helper to fill a Pair type during dancing links. It does nothing if the pair is complete.
        void add(const std::string& add) {
            if (one_ == "") {
                if (add < two_) {
                    one_ = add;
                } else {
                    one_ = two_;
                    two_ = add;
                }
            } else if (two_ == "") {
                if (add < one_) {
                    two_ = one_;
                    one_ = add;
                } else {
                    two_ = add;
                }
            }
        }

        bool operator< (const DualType& rhs) const {
            /* Compare first components. */
            auto comp = first().compare(rhs.first());
            if (comp != 0) return comp < 0;

            return second() < rhs.second();
        }

        explicit operator bool() const {
            return this->first() != "" || this->second() != "";
        }

        bool operator== (const DualType& rhs) const {
            return first() == rhs.first() && second() == rhs.second();
        }

        bool operator> (const DualType& rhs) const {
            return rhs < *this;
        }
        bool operator>= (const DualType& rhs) const {
            return !(*this < rhs);
        }
        bool operator<= (const DualType& rhs) const {
            return !(*this > rhs);
        }
        bool operator!= (const DualType& rhs) const {
            return !(*this == rhs);
        }

    private:
        std::string one_;
        std::string two_;
    };

    // As of December 2022 there are 171 unique type combinations.
    const int NUM_TYPES = 171;
    const std::string TYPE_BREAK = "<div class=\"cell-dual-type\">\r";
    const std::string TYPE_FLAG_START = "type-cell\">";
    const std::string TYPE_FLAG_END = "<";
    const std::string NULL_TYPE = "—";
    const std::string FOUR_SPACE_INDENT = "    ";
    const std::string EIGHT_SPACE_INDENT = "        ";
    const std::string TWELVE_SPACE_INDENT = "            ";

    /* The type table always has the same 18 types along the top row. We will know that we process
     * them in the same order every time, we just need to find the damage multipliers they have
     * against each type.
     */
    const std::vector<std::string> FUNDAMENTAL_TYPES = {"Normal","Fire","Water",
                                                        "Electric","Grass","Ice",
                                                        "Fighting","Poison","Ground",
                                                        "Flying","Psychic","Bug",
                                                        "Rock","Ghost","Dragon",
                                                        "Dark","Steel","Fairy"};

    const std::string FUNDAMENTAL_TYPE_DELIM = "<td title=\"";
    const std::string NO_EFFECT = "no effect";
    const std::string NOT_VERY_EFFECTIVE = "not very effective";
    const std::string QUARTER_DAMAGE = "&frac14;";
    const std::string HALF_DAMAGE = "&frac12;";
    const std::string NORMAL_EFFECTIVENESS = "normal effectiveness";
    const std::string SUPER_EFFECTIVE = "super-effective";
    const std::string DOUBLE_DAMAGE = "2";
    const std::string QUAD_DAMAGE = "4";
    const std::string DAMAGE_QUALIFIER_DELIM_START = ">";
    const std::string DAMAGE_QUALIFIER_DELIM_END = "<";
    const std::string TYPE_ADVANTAGE_DELIM = " = ";

    typedef enum DamageMultiplier {
        IMMUNE=0,
        QUARTER=1,
        HALF=2,
        NORMAL=3,
        DOUBLE=4,
        QUAD=5
    }DamageMultiplier;
    const int NUM_DAMAGE_MULTIPLIERS = 6;
    const std::vector<std::string> JSON_DAMAGE_KEYS = {"immune","quarter","half",
                                                       "normal","double","quad"};

    void quitParsing(std::ifstream& htmlTXTFile, std::ofstream& jsonFile) {
        htmlTXTFile.close();
        jsonFile.close();
        std::abort();
    }

    DualType getPokemonType(std::ifstream& htmlTXTFile,
                            std::string& line) {
        // We know the type info is on the next line.
        std::getline(htmlTXTFile, line);
        std::size_t posStart = line.find(TYPE_FLAG_START);
        if (posStart == std::string::npos) {
            std::cerr << "First type of dual type not found. Wrong line?" << std::endl;
            return {"",""};
        }
        posStart += TYPE_FLAG_START.length();
        std::size_t posEnd = line.find_first_of(TYPE_FLAG_END, posStart);
        std::string typeOne = line.substr(posStart, posEnd - posStart);

        posStart = line.find(TYPE_FLAG_START, posEnd);
        if (posStart == std::string::npos) {
            std::cerr << "Second type--even if null--of dual type not found." << std::endl;
            return {"",""};
        }
        posStart += TYPE_FLAG_START.length();
        posEnd = line.find_first_of(TYPE_FLAG_END, posStart);
        std::string typeTwo = line.substr(posStart, posEnd - posStart);
        if (typeTwo == NULL_TYPE) {
            typeTwo.clear();
        }
        return {typeOne, typeTwo};
    }

    DamageMultiplier getDamageMultiplier(const std::string& line, std::size_t pos) {
        // Looking for something like this [>&frac12<], [>&frac14<], [>2<], or [>4<]
        std::size_t openArrow = line.find_first_of(DAMAGE_QUALIFIER_DELIM_START, pos);
        if (openArrow == std::string::npos) {
            std::cerr << "You are not on right line. No damage qualifiers found." << std::endl;
        }
        openArrow++;
        std::size_t closeArrow = line.find_first_of(DAMAGE_QUALIFIER_DELIM_END, openArrow);

        std::string multiplier = line.substr(openArrow, closeArrow - openArrow);
        if (multiplier == QUARTER_DAMAGE) {
            return QUARTER;
        } else if (multiplier == HALF_DAMAGE) {
            return HALF;
        } else if (multiplier == DOUBLE_DAMAGE) {
            return DOUBLE;
        } else if (multiplier == QUAD_DAMAGE) {
            return QUAD;
        } else {
            std::cerr << "Did not find multiplier, found this: " << multiplier << std::endl;
            return IMMUNE;
        }
    }

    bool isCorrectFundamentalType(const std::string& line,
                                  std::size_t& posStart, std::size_t& posEnd,
                                  const std::string& ftype) {
        posStart = line.find(FUNDAMENTAL_TYPE_DELIM, posEnd);
        if (posStart == std::string::npos) {
            std::cerr << "Could not find fundamental type delimiter." << std::endl;
        }
        posStart += FUNDAMENTAL_TYPE_DELIM.length();
        posEnd = line.find_first_of(" ", posStart);
        std::string foundType = line.substr(posStart, posEnd - posStart);
        if (foundType != ftype) {
            std::cerr << "Current type: [" << ftype << "]" << std::endl;
            std::cerr << "Found: [" << foundType << "]." << std::endl;
            return false;
        }
        return true;
    }

    std::vector<std::vector<std::string>>
    enterTypeAdvantages(const std::string& line) {
        // We will fill these arrays with the types that do their respective damage multipliers.
        std::vector<std::vector<std::string>>
        damageClasses(NUM_DAMAGE_MULTIPLIERS, std::vector<std::string>({}));

        // We have a sliding window throughout this huge line to find all the types.
        std::size_t posStart = 0;
        std::size_t posEnd = 0;
        for (const std::string& ftype : FUNDAMENTAL_TYPES) {

            // I will take the position arguments by reference and advance them to save work.
            if (!isCorrectFundamentalType(line, posStart, posEnd, ftype)) {
                return {};
            }

            posStart = line.find(TYPE_ADVANTAGE_DELIM, posEnd);

            if (posStart == std::string::npos) {
                std::cerr << "You may be on the wrong line. No type advantage found." << std::endl;
                return {};
            }
            posStart += TYPE_ADVANTAGE_DELIM.length();

            posEnd = line.find_first_of("\"", posStart);

            std::string effectiveness = line.substr(posStart, posEnd - posStart);

            if (effectiveness == NO_EFFECT) {
                damageClasses[IMMUNE].push_back(ftype);
            } else if (effectiveness == NORMAL_EFFECTIVENESS) {
                damageClasses[NORMAL].push_back(ftype);
            } else if (effectiveness == SUPER_EFFECTIVE || effectiveness == NOT_VERY_EFFECTIVE) {
                DamageMultiplier multiple = getDamageMultiplier(line, posEnd);

                if (multiple == IMMUNE) {return {};}

                damageClasses[multiple].push_back(ftype);
            } else {
                std::cerr << "No effectiveness found, found this " << effectiveness << std::endl;
                return {};
            }
        }
        return damageClasses;
    }

    void writeTypesInMultiplierToJSON(std::ofstream& jsonFile, std::vector<std::string>& types) {
        for (int type = 0; type < types.size(); type++) {

            jsonFile << TWELVE_SPACE_INDENT << "\"" << types[type] << "\"";

            // JSON requires no trailing commas so handle edgecase.
            if (type != types.size() - 1) {
                jsonFile << ",";
            }
            jsonFile << std::endl;
        }
    }

    void writeAdvantagesToJSON(std::ofstream& jsonFile,
                               std::vector<std::vector<std::string>>& typeInteractions) {
        // We will start at immune and work our way through the enum.
        for (int multiplier = 0; multiplier < JSON_DAMAGE_KEYS.size(); multiplier++) {

            jsonFile << EIGHT_SPACE_INDENT
                     << "\"" << JSON_DAMAGE_KEYS[multiplier] << "\": [" << std::endl;

            writeTypesInMultiplierToJSON(jsonFile, typeInteractions[multiplier]);

            // JSON requires no trailing commas so handle edgecase.
            jsonFile << EIGHT_SPACE_INDENT << "]";
            if (multiplier != JSON_DAMAGE_KEYS.size() - 1) {
                jsonFile << ",";
            }
            jsonFile << std::endl;
        }
    }

    void convertToJSON(std::ifstream& htmlTXTFile, std::ofstream& jsonFile) {
        std::set<DualType> seenTypes = {};
        for (std::string line; std::getline(htmlTXTFile, line);) {
            if (line == TYPE_BREAK) {

                DualType newType = getPokemonType(htmlTXTFile, line);
                if (newType.first() == "" && newType.second() == "") {
                    quitParsing(htmlTXTFile, jsonFile);
                }

                if (!seenTypes.count(newType)) {
                    seenTypes.insert(newType);
                    jsonFile << FOUR_SPACE_INDENT << "\"";
                    // We can cout on the DualType to sort the strings for us "" is always first.
                    if (newType.first() != "") {
                        jsonFile << newType.first() << "-";
                    }
                    jsonFile << newType.second() << "\": {" << std::endl;

                    // The type advantages line is always three below this one.
                    std::getline(htmlTXTFile, line);
                    std::getline(htmlTXTFile, line);
                    std::getline(htmlTXTFile, line);

                    std::vector<std::vector<std::string>> typeStats = enterTypeAdvantages(line);

                    if (typeStats.empty()) {
                        quitParsing(htmlTXTFile, jsonFile);
                    }

                    writeAdvantagesToJSON(jsonFile, typeStats);

                    // JSON requires no trailing commas so handle edgecase.
                    jsonFile << FOUR_SPACE_INDENT << "}";
                    if (seenTypes.size() != NUM_TYPES) {
                        jsonFile << ",";
                    }
                    jsonFile << std::endl;
                }
            }
        }
        if (htmlTXTFile.bad()) {
            std::cerr << "Bad Bit Set. Error reading from file." << std::endl;
            quitParsing(htmlTXTFile, jsonFile);
        }
    }
}


int main(int argc, char *argv[]) {

    if (argc == 3) {
        std::ifstream ifs(argv[1], std::ifstream::in);
        if (!ifs) {
            std::cerr << "Cannot open file " << argv[1] << std::endl;
            std::abort();
        }
        std::ofstream ofs(argv[2], std::ofstream::out);
        if (!ofs) {
            std::cerr << "Error creating file at specified location." << std::endl;
            std::abort();
        }

        ofs << "{" << std::endl;

        convertToJSON(ifs, ofs);

        ofs << "}" << std::endl;

        ifs.close();
        ofs.close();
        std::cout << "Successfully Converted HTML to JSON!" << std::endl;
    }
    return 0;
}
