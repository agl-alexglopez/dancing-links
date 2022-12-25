#include "PokemonParser.h"
#include "DisasterParser.h"
#include <iostream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

namespace {

    const int GEN_ONE = 1;
    const std::set<std::string> GEN_TWO_ADDITIONS = {"Dark", "Steel"};
    const int GEN_TWO = 2;
    const int GEN_SIX = 6;
    const std::set<std::string> GET_SIX_ADDITIONS = {"Fairy"};
    const int MAX_GEN_COMMENT_LEN = 4;
    const QString JSON_ALL_TYPES_FILE = "res/pokemon/all-types.json";

    // Might as well use QStrings if I am parsing with them in the first place.
    const std::map<QString, Resistance::Multiplier> DAMAGE_MULTIPLIERS = {
        {"immune", Resistance::ZERO},
        {"quarter", Resistance::ONE_QUARTER},
        {"half", Resistance::ONE_HALF},
        {"normal", Resistance::ONE},
        {"double", Resistance::TWO},
        {"quad", Resistance::FOUR},
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

    std::map<std::string,std::set<Resistance>> setTypeInteractions(std::istream& source) {
        // We need to check the first line of the pokemon map file for the generation info.
        std::string line;
        std::getline(source, line);
        std::string afterHashtag = line.substr(1, line.length() - 1);
        int generation = std::stoi(afterHashtag);
        if (line.length() > MAX_GEN_COMMENT_LEN || generation >= GEN_SIX) {
            return loadAllTypes();
        } else if (generation == GEN_ONE) {
            //return loadGenOne();
        } else if (generation < GEN_SIX) {
            //return loadGensTwoToFive();
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
