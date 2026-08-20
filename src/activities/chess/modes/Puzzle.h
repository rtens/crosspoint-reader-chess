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

  ChessState* left() override;
  ChessState* right() override;
  ChessState* move(Move move) override;
};

class PuzzleStartState : public PuzzleRightState {
 public:
  PuzzleStartState(ChessActivity* activity);

  ChessState* left() override;

 private:
  bool start();
  void download(function<void()> then);
  string getDownloadError(string url);
};

class PuzzleHintState : public PuzzleRightState {
 public:
  PuzzleHintState(ChessActivity* activity);

  ChessState* right() override;
};

class PuzzleShowState : public PuzzleRightState {
 public:
  PuzzleShowState(ChessActivity* activity);

  ChessState* right() override;
};

class PuzzleWrongState : public PuzzleState {
 public:
  PuzzleWrongState(ChessActivity* activity);

  ChessState* up() override;
  ChessState* left() override;
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
  ChessState* right() override;
};