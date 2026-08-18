#pragma once

#include <Game.h>
#include <Puzzle.h>

#include <cstdint>
using namespace std;

#include "../Activity.h"
#include "ChessModeSelectionActivity.h"
#include "ChessState.h"

class ChessActivity final : public Activity {
 public:
  struct Config {
    string pieceSet = "emoji32";
  };

  explicit ChessActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Chess", renderer, mappedInput), game(), puzzle(&game) {}

  Game game;
  Puzzle puzzle;

  Config config;

  ChessState* state;

  string headerText = "Chess";
  string statusText = "";
  string infoText = "";
  string btnLeft = "";
  string btnRight = "";
  string btnUp = "";
  string btnDown = "";

  Move move;
  Move last;
  vector<Move> moves;
  int pov = Game::WHITE;

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;

 private:
  struct XY {
    const int x;
    const int y;
  };

  GfxRenderer::Orientation savedOrientation = GfxRenderer::Orientation::Portrait;

  int cellSize;
  int boardSize;
  int boardX;
  int boardY;

  void onModeSelected(ChessMode mode);

  void calculateLayoutParams();
  void renderHeader();
  void renderSideButtons();
  void renderSideButton(string text, bool left = true);
  void renderStatus();
  void renderBoard();
  void renderPieces();
  void renderMove();
  void renderLastMove();
  void renderMoves();
  void renderInfo();
  void renderButtons();
  XY squareXY(int c, int r);
  XY squareXY(int i);
};