#pragma once

#include <cstdint>
#include <functional>
using namespace std;

#include "../Activity.h"

struct ChessMode {
  int id;
  string level = "";
};

class ChessModeSelectionActivity final : public Activity {
 public:
  explicit ChessModeSelectionActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                                      function<void(ChessMode mode)> onModeSelected)
      : Activity("ChessModeSelection", renderer, mappedInput), onModeSelected(onModeSelected) {}

  static const int PUZZLE_MIX = 0;
  static const int DAILY_PUZZLE = 1;
  static const int ENGINE = 2;
  static const int OTB = 3;

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;

 private:
  function<void(ChessMode mode)> onModeSelected;
  enum State { SELECT_MODE, SELECT_LEVEL };
  State state = SELECT_MODE;
  int selectedMode = 0;
  int selectedLevel = 0;
};