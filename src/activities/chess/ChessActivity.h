#pragma once

#include <cstdint>

#include "../Activity.h"
#include "ChessEngine.h"

class ChessActivity final : public Activity {
  static constexpr int BOARD = 8;

  GfxRenderer::Orientation savedOrientation = GfxRenderer::Orientation::Portrait;
  ChessEngine engine;

 public:
  explicit ChessActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Chess", renderer, mappedInput) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;
};