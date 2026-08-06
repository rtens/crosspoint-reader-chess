#pragma once

#include <vector>

struct Move {
  int fromSquare = 0;
  int toSquare = 0;
};

class ChessEngine {
 public:
  explicit ChessEngine() {}
  virtual ~ChessEngine() = default;

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

  std::vector<int> board();
  int sideToMove();
  Move lastMove();
  std::vector<Move> legalMoves(int square);
  void makeMove(Move move);
};