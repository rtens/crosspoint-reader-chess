#include "Otb.h"

#include "../ChessActivity.h"
#include "../ChessState.h"
#include "Move.h"

///////////////////// OtbState ////////////////////

OtbState::OtbState(ChessActivity* activity) : ChessState(activity) { activity->btnRight = "Reset"; }

ChessState* OtbState::right() {
  delete this;
  return new MoveStartState(activity, new OtbStartState(activity));
}

///////////////////// OtbStartState ////////////////////

OtbStartState::OtbStartState(ChessActivity* activity) : OtbMoveState(activity) {
  activity->game.start();
  activity->move = Move{};
  activity->last = Move{};
  activity->moves = {};

  activity->pov = activity->game.turn;
  activity->infoText = "Let's go";
}

///////////////////// OtbMoveState ////////////////////

OtbMoveState::OtbMoveState(ChessActivity* activity) : OtbState(activity) { activity->infoText = ""; }

ChessState* OtbMoveState::move(Move move) {
  activity->game.make(move);
  activity->last = move;

  if (activity->game.isOver()) {
    delete this;
    return new OtbOverState(activity);
  }

  delete this;
  return new OtbMoveMadeState(activity);
}

///////////////////// OtbMoveMadeState ////////////////////

OtbMoveMadeState::OtbMoveMadeState(ChessActivity* activity) : OtbState(activity) {
  activity->btnUp = "Undo";
  activity->btnDown = "Next turn";
}

ChessState* OtbMoveMadeState::up() {
  activity->game.undo();
  activity->move = activity->last;
  activity->last = Move{};
  delete this;
  return new MoveFromState(activity, new OtbMoveState(activity));
}

ChessState* OtbMoveMadeState::down() {
  activity->pov = activity->game.turn;
  delete this;
  return new MoveStartState(activity, new OtbMoveState(activity));
}

///////////////////// OtbOverState ////////////////////

OtbOverState::OtbOverState(ChessActivity* activity) : OtbState(activity) {
  activity->btnUp = "";
  activity->btnDown = "";

  if (activity->game.isOver() == Game::CHECKMATE) {
    activity->statusText = "CHECKMATE!";
    if (activity->game.turn == Game::WHITE) {
      activity->infoText = "Game over. Black won.";
    } else {
      activity->infoText = "Game over. White won.";
    }
  } else {
    activity->statusText = "Stalemate =|";
    activity->infoText = "Game over. It's a draw.";
  }
}