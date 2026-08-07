#pragma once

#include <Logging.h>

#include <array>
#include <vector>

#include "TinyChess/board/board.h"
#include "TinyChess/engine/engine.h"

struct Move {
  int from = -1;
  int to = -1;
  uint16_t bin = 0;
};

class ChessEngine {
 private:
  Board board;
  Engine engine;

 public:
  explicit ChessEngine() : board(), engine(&board) {}
  virtual ~ChessEngine() = default;

  static const int WHITE_SIDE = 0;
  static const int BLACK_SIDE = 1;
  static const int NEITHER_SIDE = -1;

  static const int EMPTY = 0;
  static const int WHITE_PAWN = 1;
  static const int WHITE_KING = 2;
  static const int WHITE_QUEEN = 3;
  static const int WHITE_BISHOP = 4;
  static const int WHITE_KNIGHT = 5;
  static const int WHITE_ROOK = 6;
  static const int BLACK_PAWN = -1;
  static const int BLACK_KING = -2;
  static const int BLACK_QUEEN = -3;
  static const int BLACK_BISHOP = -4;
  static const int BLACK_KNIGHT = -5;
  static const int BLACK_ROOK = -6;

  void newGame(std::string position = "position startpos");
  std::array<int, 64> pieces();
  std::vector<int> myPieces(int side);
  int sideToMove();
  std::vector<Move> legalMoves(int square);
  void makeMove(Move move);
  Move respond();
  int eval();
  int gameOver();
  std::string printMove(Move move);
  std::string printPosition();
};