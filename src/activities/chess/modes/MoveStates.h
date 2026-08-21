#pragma once

#include "../ChessState.h"

class MoveState : public ChessState {
 public:
  MoveState(ChessActivity* activity, ChessState* super);

  ChessState* left() override;
  ChessState* right() override;

 protected:
  ChessState* super;
};

class MoveStartState : public MoveState {
 public:
  MoveStartState(ChessActivity* activity, ChessState* super);

  ChessState* down() override;
};

class MoveFromState : public MoveState {
 public:
  MoveFromState(ChessActivity* activity, ChessState* super);

  ChessState* up() override;
  ChessState* down() override;

 private:
  vector<int> pieceSquares{};
  int selectedPiece = 0;

  void onSelectionChanged();
};

class MoveToState : public MoveState {
 public:
  MoveToState(ChessActivity* activity, ChessState* super);

  ChessState* up() override;
  ChessState* down() override;

 private:
  int selectedMove = 0;

  void onSelectionChanged();
};

class MoveCancelState : public MoveToState {
 public:
  MoveCancelState(ChessActivity* activity, ChessState* super);

  ChessState* up() override;
  ChessState* down() override;
};