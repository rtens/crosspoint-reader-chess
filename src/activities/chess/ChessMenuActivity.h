#pragma once

#include <string>
using namespace std;

#include "../Activity.h"

class ChessMenuActivity final : public Activity {
 public:
  enum ITEM : uint8_t { PUZZLE_MIX, DAILY_PUZZLE, ENGINE, OTB, PIECE_SET };

  explicit ChessMenuActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("ChessMenu", renderer, mappedInput) {}

  static bool isMode(int item) { return item != PIECE_SET; }

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;

 private:
  vector<tuple<uint8_t, string, vector<string>>> items = {
      {PUZZLE_MIX, "Solve Puzzles", {"normal", "easier", "harder", "easiest", "hardest"}},
      {DAILY_PUZZLE, "Solve Daily Puzzle", {}},
      {ENGINE, "Play vs Engine", {"Random"}},
      {OTB, "Play vs Friend", {}},
      {PIECE_SET, "Change Piece Set", {"default"}},
  };

  enum State { SELECT_ITEM, SELECT_OPTION };
  State state = SELECT_ITEM;
  uint8_t selectedItem = 0;
  int selectedOption = 0;

  void readPieceSets();
};