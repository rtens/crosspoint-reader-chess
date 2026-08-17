#pragma once

#include <cstdint>
#include <functional>
using namespace std;

#include "../Activity.h"

class ChessModeSelectionActivity final : public Activity {
 public:
  explicit ChessModeSelectionActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                                      function<void(int mode, int level)> onModeSelected)
      : Activity("ChessModeSelection", renderer, mappedInput), onModeSelected(onModeSelected) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;

 private:
  function<void(int mode, int level)> onModeSelected;
  enum State { SELECT_MODE, SELECT_LEVEL };
  State state = SELECT_MODE;
  int selectedMode = 0;
  int selectedLevel = 0;
};