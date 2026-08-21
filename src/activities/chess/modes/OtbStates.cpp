#include "OtbStates.h"

#include "../ChessActivity.h"
#include "../ChessState.h"
#include "MoveStates.h"

///////////////////// OtbState ////////////////////

OtbState::OtbState(ChessActivity* activity) : ChessState(activity) { activity->btnRight = "Reset"; }

ChessState* OtbState::right() {
  delete this;
  return new MoveStartState(activity, new OtbStartState(activity, true));
}

///////////////////// OtbStartState ////////////////////

OtbStartState::OtbStartState(ChessActivity* activity, bool reset) : OtbMoveState(activity) {
  string fen = Game::STARTPOS;
  if (!reset) {
    fen = activity->storage.loadPosition();
  }

  activity->game.start(fen);
  activity->move = Move{};
  activity->last = Move{};
  activity->moves = {};

  activity->pov = activity->game.turn;
  activity->infoText = "Let's go";

  activity->btnLeft = "";
  if (fen == Game::STARTPOS) {
    activity->btnRight = "";
  }
}

///////////////////// OtbMoveState ////////////////////

OtbMoveState::OtbMoveState(ChessActivity* activity) : OtbState(activity) { activity->infoText = ""; }

ChessState* OtbMoveState::move(Move move) {
  activity->game.make(move);
  activity->storage.savePosition(activity->game.fen());
  activity->last = move;

  if (activity->game.isOver()) {
    delete this;
    return new OtbOverState(activity);
  }

  activity->movesSinceRefresh++;

  delete this;
  return new OtbMoveMadeState(activity);
}

///////////////////// OtbMoveMadeState ////////////////////

OtbMoveMadeState::OtbMoveMadeState(ChessActivity* activity) : OtbState(activity) {
  activity->infoText = "";
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