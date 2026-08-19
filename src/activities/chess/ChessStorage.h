#pragma once

#include <string>
using namespace std;

#include <ArduinoJson.h>

#include "ChessModeSelectionActivity.h"

struct ChessConfig {
  string pieceSet = "emoji32";
  string puzzlesUrl = "https://lichess.org/api/puzzle/batch/mix?nb=50&difficulty=";
};

class ChessStorage {
 public:
  ChessStorage();

  ChessConfig loadConfig();

  void saveMode(ChessMode mode);
  ChessMode loadMode();

  void savePosition(string fen);
  string loadPosition();

  void savePuzzleIndex(string level, int index);
  int loadPuzzleIndex(string level);
  bool loadPuzzles(string level, JsonDocument& doc);
  string puzzleFilename(string level);
};