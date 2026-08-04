#pragma once

#include <cstdint>

#include "../Activity.h"

class ChessActivity final : public Activity {
  static constexpr int BOARD = 8;
  static constexpr int CELL = 48;

  GfxRenderer::Orientation savedOrientation = GfxRenderer::Orientation::Portrait;

 public:
  explicit ChessActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Chess", renderer, mappedInput) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;
};