#include "PokemonParser.h"
#include "DisasterParser.h"
#include <map>
#include <set>
#include <iostream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

namespace {

    const int GEN_ONE = 1;
    const int GEN_2 = 2;
    const int GEN_6 = 6;
    const int GEN_8 = 8;
    const int GEN_9 = 9;
    const int MAX_GEN_COMMENT_LEN = 4;
    const QString JSON_ALL_TYPES_FILE = "res/pokemon/all-types.json";
    const QString JSON_ALL_MAPS_FILE = "res/pokemon/all-maps.json";
    const QString GYM_ATTACKS_KEY = "attack";
    const QString GYM_DEFENSE_KEY = "defense";

    const std::set<std::string> ADDED_GEN_9 = {"Bug-Dark","Fire-Grass","Poison-Steel",
                                               "Electric-Fighting","Normal-Poison","Fighting-Ground",
                                               "Fairy-Fighting","Ghost-Normal","Electric-Rock",
                                               "Electric-Grass","Electric-Psychic"};

    const std::set<std::string> ADDED_GEN_6_TO_8 = {"Fairy","Fighting-Ghost","Electric-Normal",
                                                    "Fire-Normal","Fire-Water","Fighting-Ice",
                                                    "Bug-Ice","Poison-Rock","Ghost-Grass",
                                                    "Fire-Poison","Poison-Psychic","Electric-Poison"};

    const std::set<std::string> ADDED_GEN_2_TO_5 = {"Dark","Steel","Dragon-Normal","Dragon-Fire",
                                                    "Dragon-Water","Dragon-Electric","Dragon-Grass",
                                                    "Dragon-Ice","Dragon-Fighting","Dragon-Poison",
                                                    "Dragon-Ground","Dragon-Psychic","Dragon-Rock",
                                                    "Dragon-Ghost","Dark-Dragon","Dragon-Steel",
                                                    "Bug-Rock","Electric-Water","Fighting-Fire",
                                                    "Bug-Electric","Fire-Psychic","Fighting-Grass",
                                                    "Grass-Ice","Grass-Ground","Bug-Ghost",
                                                    "Grass-Normal","Grass-Rock","Fire-Ground",
                                                    "Grass-Water","Fighting-Normal",
                                                    "Electric-Fire","Electric-Ground","Bug-Psychic",
                                                    "Ghost-Psychic","Ice-Rock","Normal-Water",
                                                    "Normal-Psychic","Fire-Ice","Flying-Psychic",
                                                    "Fire-Rock","Bug-Water","Bug-Fighting",
                                                    "Ground-Ice","Ghost-Ice","Ghost-Water",
                                                    "Ghost-Ground","Grass-Flying","Electric-Ghost",
                                                    "Bug-Ground","Flying-Ground","Fighting-Rock",
                                                    "Ground-Psychic","Flying-Ghost","Fire-Ghost",
                                                    "Bug-Fire","Psychic-Rock","Electric-Ice"};
    const std::string DUAL_TYPE_DELIM = "-";

    // Might as well use QStrings if I am parsing with them in the first place.
    const std::map<QString, Resistance::Multiplier> DAMAGE_MULTIPLIERS = {
        {"immune", Resistance::IMMUNE},
        {"quarter", Resistance::FRAC14},
        {"half", Resistance::FRAC12},
        {"normal", Resistance::NORMAL},
        {"double", Resistance::DOUBLE},
        {"quad", Resistance::QUADRU},
    };


    void getQJsonObject(QJsonObject& jsonObj, const QString& pathToJson) {
        QFile jsonFile(pathToJson);
        if (!jsonFile.open(QIODevice::ReadOnly)) {
            std::cerr << "Could not open json file." << std::endl;
            std::abort();
        }
        QByteArray bytes = jsonFile.readAll();
        jsonFile.close();
        QJsonParseError jsonError;
        QJsonDocument qtJsonDoc = QJsonDocument::fromJson(bytes, &jsonError);
        if (jsonError.error != QJsonParseError::NoError) {
            std::cerr << "Error parsing JSON to QDocument." << std::endl;
            std::abort();
        }
        if (!qtJsonDoc.isObject()) {
            std::cerr << "Error identifying JSON as object at highest level." << std::endl;
            std::abort();
        }
        jsonObj = qtJsonDoc.object();
        if (jsonObj.empty()) {
            std::cerr << "No Data in QJsonObject." << std::endl;
            std::abort();
        }
    }

    // Add filtering as necessary but for now Gen 9 includes all types by default.
    bool isGenNine(std::string& type) {
        (void) type;
        return true;
    }

    bool isGenOneType(const std::string& type) {
        if (ADDED_GEN_9.count(type) || ADDED_GEN_2_TO_5.count(type)
                || ADDED_GEN_6_TO_8.count(type)) {
            return false;
        }
        std::size_t typeDelim = type.find_first_of(DUAL_TYPE_DELIM);
        if (typeDelim != std::string::npos) {
            std::string firstType = type.substr(0, typeDelim);
            std::string secondType = type.substr(typeDelim + 1);
            if (ADDED_GEN_2_TO_5.count(firstType) || ADDED_GEN_6_TO_8.count(firstType)
                    || ADDED_GEN_2_TO_5.count(secondType) || ADDED_GEN_6_TO_8.count(secondType)) {
                return false;
            }
        }
        return true;
    }

    bool isGenTwoToFive(std::string& type) {
        if (ADDED_GEN_6_TO_8.count(type) || ADDED_GEN_9.count(type)) {
            return false;
        }
        std::size_t typeDelim = type.find_first_of(DUAL_TYPE_DELIM);
        if (typeDelim != std::string::npos) {
            std::string firstType = type.substr(0, typeDelim);
            std::string secondType = type.substr(typeDelim + 1);
            if (ADDED_GEN_6_TO_8.count(firstType) || ADDED_GEN_6_TO_8.count(secondType)
                    || ADDED_GEN_9.count(firstType) || ADDED_GEN_9.count(secondType)) {
                return false;
            }
        }
        return true;
    }

    bool isGenSixToEight(std::string& type) {
        if (ADDED_GEN_9.count(type)) {
            return false;
        }
        std::size_t typeDelim = type.find_first_of(DUAL_TYPE_DELIM);
        if (typeDelim != std::string::npos) {
            std::string firstType = type.substr(0, typeDelim);
            std::string secondType = type.substr(typeDelim + 1);
            if (ADDED_GEN_9.count(firstType) || ADDED_GEN_9.count(secondType)) {
                return false;
            }
        }
        return true;
    }

    std::map<std::string,std::set<Resistance>>
    filterPokemonByGeneration(std::function<bool(std::string&)> filterFunc) {
        QJsonObject pokemonData;
        getQJsonObject(pokemonData, JSON_ALL_TYPES_FILE);
        std::map<std::string,std::set<Resistance>> result = {};
        for (const QString& type : pokemonData.keys()) {
            std::string newType = type.toStdString();
            if (!filterFunc(newType)) {
                continue;
            }
            result.insert({newType, {}});
            QJsonObject damageMultipliers = pokemonData[type].toObject();
            for (const QString& mult : damageMultipliers.keys()) {
                Resistance::Multiplier multiplierTag = DAMAGE_MULTIPLIERS.at(mult);

                QJsonArray typesInMultipliers = damageMultipliers[mult].toArray();
                for (const QJsonValueConstRef& t : typesInMultipliers) {
                    std::string type = QString(t.toString()).toStdString();
                    if (filterFunc(type)) {
                        result[newType].insert({type, multiplierTag});
                    }
                }
            }
        }
        return result;
    }

    std::map<std::string,std::set<Resistance>> setTypeInteractions(std::istream& source) {
        // We need to check the first line of the pokemon map file for the generation info.
        std::string line;
        std::getline(source, line);
        std::string afterHashtag = line.substr(1, line.length() - 1);
        int generation = std::stoi(afterHashtag);
        if (line.length() > MAX_GEN_COMMENT_LEN || generation >= GEN_9) {
            return filterPokemonByGeneration(&isGenNine);
        } else if (generation == GEN_ONE) {
            return filterPokemonByGeneration(&isGenOneType);
        } else if (generation < GEN_6) {
            return filterPokemonByGeneration(&isGenTwoToFive);
        } else if (generation <= GEN_8){
            return filterPokemonByGeneration(&isGenSixToEight);
        } else {
            std::cerr << "Could not pick a Pokemon Generation to load." << std::endl;
            return {};
        }
    }
}


PokemonTest loadPokemonGeneration(std::istream& source) {
    PokemonTest generation;
    generation.typeInteractions = setTypeInteractions(source);
    generation.pokemonGenerationMap = loadDisaster(source);
    return generation;
}

std::map<std::string,std::set<Resistance>>
loadSelectedGymsDefense(const std::map<std::string,std::set<Resistance>>& currentGenInteractions,
                        const std::string& selectedMap,
                        const Set<std::string>& selectedGyms) {
    QJsonObject mapData;
    getQJsonObject(mapData, JSON_ALL_MAPS_FILE);
    std::map<std::string,std::set<Resistance>> result = {};

    // Get the data associated with the selected file name for a map.
    QString map = QString::fromStdString(selectedMap);
    QJsonObject gymKeys = mapData[map].toObject();

    // Iterate through all of the gym keys, if a key is in the selected Gyms proceed.
    for (const QString& gym : gymKeys.keys()) {

        if (selectedGyms.contains(gym.toStdString())) {
            // Take all the types for that gym and put that type and its associated resistances in result.
            QJsonArray gymDefenseTypes = gymKeys[gym][GYM_DEFENSE_KEY].toArray();
            for (const QJsonValueConstRef& type : gymDefenseTypes) {
                // You can get the resistances from the generation map we have already built.
                std::string stdVersion = QString(type.toString()).toStdString();


                result[stdVersion] = currentGenInteractions.at(stdVersion);

            }
        }

    }
    // return this much smaller map.
    return result;
}

std::set<std::string> loadSelectedGymsAttacks(const std::string& selectedMap,
                                              const Set<std::string>& selectedGyms) {
    QJsonObject mapData;
    getQJsonObject(mapData, JSON_ALL_MAPS_FILE);
    std::set<std::string> result = {};
    // Get the data for the selected map
    QString map = QString::fromStdString(selectedMap);
    QJsonObject gymKeys = mapData[map].toObject();

    // Iterate through all the gym keys if the key is in the selected Gyms proceed.
    for (const QString& gym : gymKeys.keys()) {

        if (selectedGyms.contains(gym.toStdString())) {
            // Add all attack types to the result set invariant, duplicates will filter out.
            QJsonArray gymAttackTypes = gymKeys[gym][GYM_ATTACKS_KEY].toArray();
            for (const QJsonValueConstRef& type : gymAttackTypes) {
                std::string stdVersion = QString(type.toString()).toStdString();
                result.insert(stdVersion);
            }
        }
    }
    return result;
}
