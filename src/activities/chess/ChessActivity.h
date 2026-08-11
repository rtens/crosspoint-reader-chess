#pragma once

#include <Game.h>

#include <cstdint>

#include "../Activity.h"

using namespace std;

class ChessActivity final : public Activity {
  static constexpr int BOARD = 8;

 private:
  Game game;
  GfxRenderer::Orientation savedOrientation = GfxRenderer::Orientation::Portrait;

  string info = "";

  static const int SELECT_PIECE = 0;
  static const int SELECT_MOVE = 1;
  static const int THINKING = 2;
  static const int GAME_OVER = 3;

  static const int OTB = 0;
  static const int RANDROM = 1;
  static const int COMPUTER = 2;

  int state = SELECT_PIECE;
  int mode = OTB;

  int selected = 0;
  int selected_piece = 0;

  vector<int> pieces;
  vector<int> mine;
  vector<Move> moves;
  int over;

  Move last = Move{};

  void copyPieces();
  void savePosition();
  string loadPosition();

 public:
  explicit ChessActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Chess", renderer, mappedInput), game() {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;
};