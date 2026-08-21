#pragma once

#include <Engine.h>
#include <Game.h>
#include <Puzzle.h>

#include <cstdint>
using namespace std;

#include "../Activity.h"
#include "ChessModeSelectionActivity.h"
#include "ChessState.h"
#include "ChessStorage.h"

class ChessActivity final : public Activity {
 public:
  Game game;
  Puzzle puzzle;
  Engine engine;
  ChessConfig config;
  ChessStorage storage;

  ChessState* state = 0;
  string level = "";

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

  explicit ChessActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Chess", renderer, mappedInput), game(), puzzle(&game), engine(&game) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;

  void startWifi(function<void()> then);

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
  void renderDarkSquare(int c, int r);
  void renderPieces();
  void renderMove();
  void renderLastMove();
  void renderMoves();
  void renderInfo();
  void renderButtons();
  XY squareXY(int c, int r);
  XY squareXY(int i);
};