#pragma once

#include <Game.h>

#include "../ChessState.h"

class OtbGoingState : public ChessState {
 public:
  OtbGoingState(ChessActivity* activity);

  ChessState* right() override;
  ChessState* move(Move move) override;
};

class OtbStartState : public OtbGoingState {
 public:
  OtbStartState(ChessActivity* activity);
};