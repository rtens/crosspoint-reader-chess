#pragma once

#include <Game.h>

class ChessActivity;

class ChessState {
 public:
  ChessState(ChessActivity* activity);

  virtual ChessState* up();
  virtual ChessState* down();
  virtual ChessState* left();
  virtual ChessState* right();
  virtual ChessState* move(Move move);

 protected:
  ChessActivity* activity;
};