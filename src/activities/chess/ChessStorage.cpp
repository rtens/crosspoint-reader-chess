#include "ChessStorage.h"

#include <string>
using namespace std;

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Game.h>
#include <Logging.h>

#include "ChessModeSelectionActivity.h"

const string BASE_PATH = "/.chess";
const string CONFIG_FILE = BASE_PATH + "/config.json";
const string MODE_FILE = BASE_PATH + "/mode.json";
const string POSITION_FILE = BASE_PATH + "/position.fen";
const string INDEX_FILE = BASE_PATH + "/puzzle_index.json";

ChessStorage::ChessStorage() {
  if (!Storage.exists(BASE_PATH.c_str())) {
    Storage.mkdir(BASE_PATH.c_str());
  }
}

ChessConfig ChessStorage::loadConfig() {
  LOG_DBG("CHESS", "Loading config");
  ChessConfig config;

  HalFile file = Storage.open(CONFIG_FILE.c_str());
  if (!file) return config;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();
  if (err) return config;

  if (doc["pieceSet"].is<string>()) {
    config.pieceSet = doc["pieceSet"].as<string>();
    LOG_DBG("CHESS", "Config pieceSet: %s", config.pieceSet.c_str());
  }
  for (auto puzzleUrl : config.puzzleUrls) {
    string level = puzzleUrl.first;
    if (doc["puzzleUrls"][level].is<string>()) {
      config.puzzleUrls[level] = doc["puzzleUrls"][level].as<string>();
      LOG_DBG("CHESS", "Config puzzleUrls of %s: %s", level.c_str(), config.puzzleUrls[level].c_str());
    }
  }
  LOG_DBG("CHESS", "Loaded config");
  return config;
}

void ChessStorage::saveMode(ChessMode mode) {
  LOG_DBG("CHESS", "Saving mode %i %s", mode.id, mode.level.c_str());
  auto file = Storage.open(MODE_FILE.c_str(), O_WRITE | O_CREAT | O_TRUNC);
  if (!file) return;

  JsonDocument doc;
  doc["id"] = mode.id;
  doc["level"] = mode.level;

  serializeJson(doc, file);
  file.close();
  LOG_DBG("CHESS", "Saved mode");
}

ChessMode ChessStorage::loadMode() {
  LOG_DBG("CHESS", "Loading mode");
  ChessMode mode{ChessModeSelectionActivity::OTB};

  HalFile file = Storage.open(MODE_FILE.c_str());
  if (!file) return mode;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();
  if (err) return mode;

  if (doc["id"].is<int>()) {
    mode.id = doc["id"].as<int>();
    LOG_DBG("CHESS", "Mode ID: %i", mode.id);
  }
  if (doc["level"].is<string>()) {
    mode.level = doc["level"].as<string>();
    LOG_DBG("CHESS", "Mode level: %s", mode.level.c_str());
  }
  LOG_DBG("CHESS", "Loaded mode");
  return mode;
}

void ChessStorage::savePosition(string fen) {
  LOG_DBG("CHESS", "Saving positoin %s", fen.c_str());
  auto file = Storage.open(POSITION_FILE.c_str(), O_WRITE | O_CREAT | O_TRUNC);
  if (!file) return;

  file.write(fen.c_str(), fen.size());
  file.close();
  LOG_DBG("CHESS", "Saved position");
}

string ChessStorage::loadPosition() {
  LOG_DBG("CHESS", "Loading position");
  HalFile file = Storage.open(POSITION_FILE.c_str());
  if (!file) return Game::STARTPOS;

  char buffer[file.size()];
  file.read(buffer, file.size());
  file.close();

  LOG_DBG("CHESS", "Loaded position %s", string(buffer).c_str());
  return string(buffer);
}

void ChessStorage::savePuzzleIndex(string level, int index) {
  LOG_DBG("CHESS", "Saving puzzle index %s %i in %s", level.c_str(), index, INDEX_FILE.c_str());
  JsonDocument doc;

  HalFile reading = Storage.open(INDEX_FILE.c_str());
  if (reading) {
    deserializeJson(doc, reading);
    reading.close();
  }

  HalFile file = Storage.open(INDEX_FILE.c_str(), O_WRITE | O_CREAT | O_TRUNC);
  if (!file) return;

  doc[level] = index;
  serializeJson(doc, file);

  file.close();
  LOG_DBG("CHESS", "Saved puzzle index");
}

int ChessStorage::loadPuzzleIndex(string level) {
  LOG_DBG("CHESS", "Loading puzzle index %s", level.c_str());
  HalFile file = Storage.open(INDEX_FILE.c_str());
  if (!file) return 0;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();
  if (err) return 0;

  if (doc[level].is<int>()) {
    int index = doc[level];
    LOG_DBG("CHESS", "Loaded puzzle index %i", index);
    return index;
  }

  return 0;
}

string ChessStorage::puzzleFilename(string level) { return BASE_PATH + "/puzzles_" + level + ".json"; }

bool ChessStorage::loadPuzzles(string level, JsonDocument& doc) {
  string filename = puzzleFilename(level);
  LOG_DBG("CHESS", "Reading puzzles %s", filename.c_str());

  HalFile file;
  if (!Storage.openFileForRead("CHESS", filename, file)) {
    LOG_DBG("CHESS", "Cannot open %s", filename.c_str());
    return false;
  }

  DeserializationError err = deserializeJson(doc, file);
  file.close();

  if (err) {
    LOG_ERR("CHESS", "Parsing failed %s", err.c_str());
    return false;
  }

  return true;
}