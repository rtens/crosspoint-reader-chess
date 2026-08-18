#include <Game.h>

class ChessActivity;

class ChessState {
 public:
  ChessState(ChessActivity* activity);

  ChessState up();
  ChessState down();
  ChessState left();
  ChessState right();
  ChessState move(Move move);

 private:
  ChessActivity* activity;
};