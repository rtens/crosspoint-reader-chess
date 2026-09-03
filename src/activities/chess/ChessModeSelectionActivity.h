#pragma once

#include <cstdint>
using namespace std;

#include "../Activity.h"

struct ChessMode {
  int id;
  string level = "";
};

class ChessModeSelectionActivity final : public Activity {
 public:
  explicit ChessModeSelectionActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("ChessModeSelection", renderer, mappedInput) {}

  static const int PUZZLE_MIX = 0;
  static const int DAILY_PUZZLE = 1;
  static const int ENGINE = 2;
  static const int OTB = 3;
  static const int PIECE_SET = 4;

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;

 private:
  vector<vector<string>> modes = {
      {"Solve Puzzles", "normal", "easier", "harder", "easiest", "hardest"},
      {"Solve Daily Puzzle"},
      {"Play vs Engine", "Random"},
      {"Play vs Friend"},
      {"Change Piece Set", "default"},
  };

  enum State { SELECT_MODE, SELECT_LEVEL };
  State state = SELECT_MODE;
  int selectedMode = 0;
  int selectedLevel = 0;

  void readPieceSets();
};