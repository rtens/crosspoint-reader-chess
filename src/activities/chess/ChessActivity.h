#pragma once

#include <cstdint>

#include "../Activity.h"
#include "ChessEngine.h"

class ChessActivity final : public Activity {
  static constexpr int BOARD = 8;

 private:
  GfxRenderer::Orientation savedOrientation = GfxRenderer::Orientation::Portrait;
  ChessEngine engine;
  std::string info = "";

  static const int SELECT_PIECE = 0;
  static const int SELECT_MOVE = 1;
  static const int THINKING = 2;

  static const int COMPUTER = 0;
  static const int OTB = 1;
  static const int RANDROM = 2;

  int state = SELECT_PIECE;
  int mode = COMPUTER;

  int selected = 0;
  int selected_piece = 0;

  std::array<int, 64> pieces;
  std::vector<int> mine;
  std::vector<Move> moves;

  Move response = Move{};

  void savePosition();
  std::string loadPosition();

 public:
  explicit ChessActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Chess", renderer, mappedInput), engine() {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;
};