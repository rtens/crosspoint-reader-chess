#pragma once

#include <cstdint>

#include "../Activity.h"
#include "engine/Game.h"

class ChessActivity final : public Activity {
  static constexpr int BOARD = 8;

 private:
  Game game;
  GfxRenderer::Orientation savedOrientation = GfxRenderer::Orientation::Portrait;

  std::string info = "";

  static const int SELECT_PIECE = 0;
  static const int SELECT_MOVE = 1;
  static const int THINKING = 2;

  static const int OTB = 0;
  static const int RANDROM = 1;
  static const int COMPUTER = 2;

  int state = SELECT_PIECE;
  int mode = OTB;

  int selected = 0;
  int selected_piece = 0;

  std::vector<int> pieces;
  std::vector<int> mine;
  std::vector<Move> moves;

  Move last = Move{};

  void copyPieces();
  void savePosition();
  std::string loadPosition();

 public:
  explicit ChessActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Chess", renderer, mappedInput), game() {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;
};