#pragma once

#include <Chess/Game.h>

#include "../ChessState.h"

class OtbState : public ChessState {
 public:
  OtbState(ChessActivity* activity);

  ChessState* right() override;
};

class OtbMoveState : public OtbState {
 public:
  OtbMoveState(ChessActivity* activity);

  ChessState* move(Chess::Move move) override;
};

class OtbStartState : public OtbMoveState {
 public:
  OtbStartState(ChessActivity* activity, bool reset = false);
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