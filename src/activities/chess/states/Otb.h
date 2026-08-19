#pragma once

#include <Game.h>

#include "../ChessState.h"

class OtbState : public ChessState {
 public:
  OtbState(ChessActivity* activity);

  ChessState* right() override;
};

class OtbStartState : public OtbMoveState {
 public:
  OtbStartState(ChessActivity* activity);
};

class OtbMoveState : public OtbState {
 public:
  OtbMoveState(ChessActivity* activity);

  ChessState* move(Move move) override;
};

class OtbMoveMadeState : public OtbState {
 public:
  OtbMoveMadeState(ChessActivity* activity);

  ChessState* up() override;
  ChessState* down() override;
};

class OtbOverState : public OtbState {
 public:
  OtbOverState(ChessActivity* activity);
};