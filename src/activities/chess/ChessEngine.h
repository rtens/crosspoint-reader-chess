#pragma once

class ChessEngine {
  struct Move {
    int fromSquare;
    int toSquare;
  };

 public:
  explicit ChessEngine() {}
  virtual ~ChessEngine() = default;

  virtual void board(int pieces[8][8]);
  virtual int sideToMove();
  virtual Move* lastMove();
  virtual void legalMoves(int square, Move* moves);
  virtual void makeMove(Move move);
};