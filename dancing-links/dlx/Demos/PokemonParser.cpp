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

    const std::set<std::string> ADDED_GEN_9 = {"Bug-Dark","Fire-Grass","Poison-Steel",
                                               "Electric-Fighting","Normal-Poison","Fighting-Ground",
                                               "Fairy-Fighting","Ghost-Normal"};

    const std::set<std::string> ADDED_GEN_6 = {"Fairy"};

    const std::set<std::string> ADDED_GEN_2 = {"Dark","Steel","Dragon-Normal","Dragon-Fire",
                                               "Dragon-Water","Dragon-Electric","Dragon-Grass",
                                               "Dragon-Ice","Dragon-Fighting","Dragon-Poison",
                                               "Dragon-Ground","Dragon-Psychic","Dragon-Rock",
                                               "Dragon-Ghost","Dragon-Dark","Dragon-Steel"};
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


    void getQJsonObject(QJsonObject& jsonObj) {
        QFile jsonFile(JSON_ALL_TYPES_FILE);
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

    std::map<std::string,std::set<Resistance>> loadAllTypes() {
        QJsonObject pokemonData;
        getQJsonObject(pokemonData);
        std::map<std::string,std::set<Resistance>> result = {};
        for (const QString& type : pokemonData.keys()) {

            std::string newType = type.toStdString();

            // For example: {"Flying", {}}
            result.insert({newType, {}});

            QJsonObject damageMultipliers = pokemonData[type].toObject();
            for (const QString& mult : damageMultipliers.keys()) {
                Resistance::Multiplier multiplierTag = DAMAGE_MULTIPLIERS.at(mult);

                QJsonArray typesInMultipliers = damageMultipliers[mult].toArray();
                for (const QJsonValueRef& t : typesInMultipliers) {
                    QString tString = t.toString();

                    /* For example:
                     *
                     * {"Flying", {"Fighting x2.0","Ground x0.0"...}
                     *
                     */
                    result[newType].insert({tString.toStdString(), multiplierTag});
                }
            }
        }
        return result;
    }

    bool isGenOneType(const std::string& type) {
        if (ADDED_GEN_9.count(type) || ADDED_GEN_2.count(type) || ADDED_GEN_6.count(type)) {
            return false;
        }
        std::size_t typeDelim = type.find_first_of(DUAL_TYPE_DELIM);
        if (typeDelim != std::string::npos) {
            std::string firstType = type.substr(0, typeDelim);
            std::string secondType = type.substr(typeDelim + 1);
            if (ADDED_GEN_2.count(firstType) || ADDED_GEN_6.count(firstType)
                    || ADDED_GEN_2.count(secondType) || ADDED_GEN_6.count(secondType)) {
                return false;
            }
        }
        return true;
    }

    std::map<std::string,std::set<Resistance>> loadGenOne() {
        QJsonObject pokemonData;
        getQJsonObject(pokemonData);
        std::map<std::string,std::set<Resistance>> result = {};
        for (const QString& type : pokemonData.keys()) {
            std::string newType = type.toStdString();
            if (!isGenOneType(newType)) {
                continue;
            }
            result.insert({newType, {}});
            QJsonObject damageMultipliers = pokemonData[type].toObject();
            for (const QString& mult : damageMultipliers.keys()) {
                Resistance::Multiplier multiplierTag = DAMAGE_MULTIPLIERS.at(mult);

                QJsonArray typesInMultipliers = damageMultipliers[mult].toArray();
                for (const QJsonValueRef& t : typesInMultipliers) {
                    std::string type = QString(t.toString()).toStdString();
                    if (isGenOneType(type)) {
                        result[newType].insert({type, multiplierTag});
                    }
                }
            }
        }
        return result;
    }

    bool isGenTwoToFive(std::string& type) {
        if (ADDED_GEN_6.count(type) || ADDED_GEN_9.count(type)) {
            return false;
        }
        std::size_t typeDelim = type.find_first_of(DUAL_TYPE_DELIM);
        if (typeDelim != std::string::npos) {
            std::string firstType = type.substr(0, typeDelim);
            std::string secondType = type.substr(typeDelim + 1);
            if (ADDED_GEN_6.count(firstType) || ADDED_GEN_6.count(secondType)
                    || ADDED_GEN_9.count(firstType) || ADDED_GEN_9.count(secondType)) {
                return false;
            }
        }
        return true;
    }

    std::map<std::string,std::set<Resistance>> loadGensTwoToFive() {
        QJsonObject pokemonData;
        getQJsonObject(pokemonData);
        std::map<std::string,std::set<Resistance>> result = {};
        for (const QString& type : pokemonData.keys()) {
            std::string newType = type.toStdString();
            if (!isGenTwoToFive(newType)) {
                continue;
            }
            result.insert({newType, {}});
            QJsonObject damageMultipliers = pokemonData[type].toObject();
            for (const QString& mult : damageMultipliers.keys()) {
                Resistance::Multiplier multiplierTag = DAMAGE_MULTIPLIERS.at(mult);

                QJsonArray typesInMultipliers = damageMultipliers[mult].toArray();
                for (const QJsonValueRef& t : typesInMultipliers) {
                    std::string type = QString(t.toString()).toStdString();
                    if (isGenTwoToFive(type)) {
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
            return loadAllTypes();
        } else if (generation == GEN_ONE) {
            return loadGenOne();
        } else if (generation < GEN_6) {
            return loadGensTwoToFive();
        } else {
            // return loadGensSixToEight();
        }
        std::cerr << "Could not pick a Pokemon Generation to load." << std::endl;
        return {};
    }
}


PokemonTest loadPokemonGeneration(std::istream& source) {
    PokemonTest generation;
    generation.typeInteractions = setTypeInteractions(source);
    generation.pokemonGenerationMap = loadDisaster(source);
    return generation;
}
