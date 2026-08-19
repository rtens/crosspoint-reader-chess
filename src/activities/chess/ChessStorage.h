#pragma once

#include "ChessModeSelectionActivity.h"

struct ChessConfig {
  string pieceSet = "emoji32";
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
};