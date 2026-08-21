#pragma once

#include <Game.h>

#include "../ChessState.h"

class EngineState : public ChessState {
 public:
  EngineState(ChessActivity* activity);

  ChessState* right() override;
};

class EngineRunningState : public EngineState {
 public:
  EngineRunningState(ChessActivity* activity, bool reaset = false);

  ChessState* move(Move move) override;
};

class EngineOverState : public EngineState {
 public:
  EngineOverState(ChessActivity* activity, string winner);
};