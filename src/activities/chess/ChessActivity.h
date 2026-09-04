#pragma once

#include <Chess/Board.h>
#include <Chess/Engine.h>
#include <Chess/Game.h>
#include <Chess/Piece.h>
#include <Chess/Puzzle.h>

#include <cstdint>
using namespace std;

#include "../Activity.h"
#include "ChessMenuActivity.h"
#include "ChessState.h"
#include "ChessStorage.h"

class ChessActivity final : public Activity {
 public:
  Chess::Board board;
  Chess::Game game;
  Chess::Puzzle* puzzle = 0;
  Chess::Engine* engine = 0;

  ChessConfig config;
  ChessStorage storage;

  int movesSinceRefresh = 0;
  ChessState* state = 0;
  string level = "";

  string headerText = "Chess";
  string statusText = "";
  string infoText = "";
  string btnLeft = "";
  string btnRight = "";
  string btnUp = "";
  string btnDown = "";

  Chess::Move move;
  vector<Chess::Move> moves;
  Chess::Color pov = Chess::White;

  explicit ChessActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Chess", renderer, mappedInput), board(), game(&board) {}

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

  uint8_t* pieceSet = 0;

  int cellSize;
  int boardSize;
  int boardX;
  int boardY;

  void onModeSelected(ChessMode mode);
  void onPieceSetSelected(string name);
  void calculateLayoutParams();

  void renderHeader();
  void renderSideButtons();
  void renderSideButton(string text, bool left = true);
  void renderStatus();
  void renderBoard();
  void renderDarkSquare(int c, int r);
  void renderPieces();
  void renderDefaultPiece(uint8_t piece, XY sq);
  void renderPieceFromSet(uint8_t piece, XY sq);
  void renderMove();
  void renderLastMove();
  void renderMoves();
  void renderInfo();
  void renderButtons();

  XY squareXY(int c, int r);
  XY squareXY(int i);
};