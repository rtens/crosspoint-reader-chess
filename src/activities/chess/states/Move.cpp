#include "Move.h"

#include <Game.h>

#include "../ChessActivity.h"
#include "../ChessState.h"

MoveState::MoveState(ChessActivity* activity, ChessState* super) : ChessState(activity) { this->super = super; }

ChessState* MoveState::left() { return super->left(); }

ChessState* MoveState::right() { return super->right(); }

///////////////////// MoveStartState ////////////////////

MoveStartState::MoveStartState(ChessActivity* activity, ChessState* super) : MoveState(activity, super) {
  activity->btnUp = "";
  activity->btnDown = "First Piece";
  activity->move = Move{};

  if (activity->game.turn == Game::WHITE) {
    activity->statusText = "White to move";
  } else {
    activity->statusText = "Black to move";
  }
}

ChessState* MoveStartState::down() {
  delete this;
  return new MoveFromState(activity, super);
}

///////////////////// MoveFromState ////////////////////

MoveFromState::MoveFromState(ChessActivity* activity, ChessState* super) : MoveState(activity, super) {
  activity->btnUp = "Select";
  activity->btnDown = "Next Piece";

  for (int i = 0; i < 64; i++) {
    int piece = activity->game.pieces[i];
    if (piece & activity->game.turn) {
      pieceSquares.push_back(i);
    }
  }

  if (activity->pov == Game::BLACK) {
    sort(pieceSquares.begin(), pieceSquares.end(), [](int a, int b) { return a > b; });
  }

  onSelectionChanged();
}

ChessState* MoveFromState::up() {
  delete this;
  return new MoveCancelState(activity, super);
}

ChessState* MoveFromState::down() {
  selectedPiece++;
  onSelectionChanged();
  return this;
}

void MoveFromState::onSelectionChanged() {
  selectedPiece %= pieceSquares.size();
  activity->move = Move{pieceSquares[selectedPiece]};
  activity->statusText = Game::print(activity->move.from);
}

///////////////////// MoveToState ////////////////////

MoveToState::MoveToState(ChessActivity* activity, ChessState* super) : MoveState(activity, super) {
  activity->btnUp = "Make Move";
  activity->btnDown = "Next Move";

  activity->moves = activity->game.moves(activity->move.from);

  if (activity->pov == Game::WHITE) {
    sort(activity->moves.begin(), activity->moves.end(), [](Move a, Move b) { return a.to < b.to; });
  } else {
    sort(activity->moves.begin(), activity->moves.end(), [](Move a, Move b) { return a.to > b.to; });
  }
  onSelectionChanged();
}

ChessState* MoveToState::up() {
  Move move = activity->move;
  activity->move = Move{};
  activity->moves = {};
  delete this;
  return super->move(move);
}

ChessState* MoveToState::down() {
  selectedMove++;

  if (selectedMove == activity->moves.size()) {
    delete this;
    return new MoveCancelState(activity, super);
  }

  onSelectionChanged();
  return this;
}

void MoveToState::onSelectionChanged() {
  if (activity->moves.size() == 0) return;

  selectedMove %= activity->moves.size();
  activity->move = activity->moves[selectedMove];
  activity->statusText = Game::print(activity->move);
}

///////////////////// MoveCancelState ////////////////////

MoveCancelState::MoveCancelState(ChessActivity* activity, ChessState* super) : MoveToState(activity, super) {
  activity->btnUp = "Cancel";
  activity->statusText = "Cancel";
  activity->move.to = activity->move.from;
}

ChessState* MoveCancelState::up() {
  activity->moves = {};
  delete this;
  return new MoveFromState(activity, super);
}

ChessState* MoveCancelState::down() {
  delete this;
  return new MoveToState(activity, super);
}