#include "Move.h"

#include <Game.h>
#include <Logging.h>

#include "../ChessActivity.h"
#include "../ChessState.h"

///////////////////// MoveState ////////////////////

MoveState::MoveState(ChessActivity* activity, ChessState* super) : ChessState(activity) { this->super = super; }

ChessState* MoveState::left() { return super->left(); }

ChessState* MoveState::right() { return super->right(); }

///////////////////// MoveStartState ////////////////////

MoveStartState::MoveStartState(ChessActivity* activity, ChessState* super) : MoveState(activity, super) {
  LOG_DBG("CHESS", "Init MoveStartState");
  activity->btnUp = "";
  activity->btnDown = "Select Piece";
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
  LOG_DBG("CHESS", "Init MoveFromState");
  activity->btnUp = "Show Moves";
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

  for (int i = 0; i < pieceSquares.size(); i++) {
    if (activity->move.from == pieceSquares[i]) {
      selectedPiece = i;
      break;
    }
  }

  onSelectionChanged();
  LOG_DBG("CHESS", "Done init MoveFromState");
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
  if (pieceSquares.size() == 0) return;

  selectedPiece %= pieceSquares.size();
  activity->move = Move{pieceSquares[selectedPiece]};
  activity->statusText = Game::print(activity->move.from);
}

///////////////////// MoveToState ////////////////////

MoveToState::MoveToState(ChessActivity* activity, ChessState* super) : MoveState(activity, super) {
  LOG_DBG("CHESS", "Init MoveToState");
  activity->btnUp = "Make Move";
  activity->btnDown = "Next Move";

  activity->moves = activity->game.moves(activity->move.from);

  if (activity->pov == Game::WHITE) {
    sort(activity->moves.begin(), activity->moves.end(), [](Move a, Move b) { return a.to < b.to; });
  } else {
    sort(activity->moves.begin(), activity->moves.end(), [](Move a, Move b) { return a.to > b.to; });
  }

  onSelectionChanged();
  LOG_DBG("CHESS", "Done init MoveToState");
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
  LOG_DBG("CHESS", "Init MoveCancelState");
  activity->btnUp = "Cancel";
  activity->statusText = "Move " + Game::print(activity->move.from);
  activity->move.to = activity->move.from;

  if (activity->moves.size() == 0) {
    activity->statusText = "No legal moves";
    activity->btnDown = "";
  }

  LOG_DBG("CHESS", "Done init MoveCancelState");
}

ChessState* MoveCancelState::up() {
  activity->moves = {};
  delete this;
  return new MoveFromState(activity, super);
}

ChessState* MoveCancelState::down() {
  if (activity->moves.size() == 0) {
    return this;
  }

  delete this;
  return new MoveToState(activity, super);
}