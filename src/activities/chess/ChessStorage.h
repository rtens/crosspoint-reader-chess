#pragma once

#include <map>
#include <string>
using namespace std;

#include <ArduinoJson.h>

struct ChessConfig {
  int movesUntilRefresh = 20;
  string pieceSet = "default";
  std::map<string, string> puzzleUrls = {
      {"normal", "https://lichess.org/api/puzzle/batch/mix?nb=50"},
      {"easier", "https://lichess.org/api/puzzle/batch/mix?nb=50&difficulty=easier"},
      {"harder", "https://lichess.org/api/puzzle/batch/mix?nb=50&difficulty=harder"},
      {"easiest", "https://lichess.org/api/puzzle/batch/mix?nb=50&difficulty=easiest"},
      {"hardest", "https://lichess.org/api/puzzle/batch/mix?nb=50&difficulty=hardest"},
      {"daily", "https://lichess.org/api/puzzle/daily"},
  };
};

struct ChessMode {
  int id;
  string level = "";
};

class ChessStorage {
 public:
  ChessStorage();

  void saveConfig(ChessConfig config);
  ChessConfig loadConfig();

  void saveMode(ChessMode mode);
  ChessMode loadMode();

  void savePosition(string fen);
  string loadPosition();

  void savePuzzleIndex(string level, int index);
  int loadPuzzleIndex(string level);
  bool loadPuzzles(string level, JsonDocument& doc);
  string puzzleFilename(string level);

  vector<String> listPieceSets();
  uint8_t* loadPieceSet(string name);
};