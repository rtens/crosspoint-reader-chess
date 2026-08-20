#pragma once

#include <Game.h>

#include <functional>

#include "../ChessState.h"

class PuzzleState : public ChessState {
 public:
  PuzzleState(ChessActivity* activity);
};

class PuzzleRightState : public PuzzleState {
 public:
  PuzzleRightState(ChessActivity* activity);

  ChessState* move(Move move) override;
};

class PuzzleStartState : public PuzzleRightState {
 public:
  PuzzleStartState(ChessActivity* activity);

 private:
  bool start();
  void download(function<void()> then);
  string getDownloadError(string url);
};

class PuzzleWrongState : public PuzzleState {
 public:
  PuzzleWrongState(ChessActivity* activity);

  ChessState* up() override;
};

class PuzzleCorrectionState : public PuzzleRightState {
 public:
  PuzzleCorrectionState(ChessActivity* activity);
};

class PuzzleSolvedState : public PuzzleState {
 public:
  PuzzleSolvedState(ChessActivity* activity);

  ChessState* up() override;
  ChessState* down() override;
};