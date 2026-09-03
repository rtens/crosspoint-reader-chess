#include "OtbStates.h"

#include <Chess/Parse.h>
#include <Chess/Print.h>

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
  string fen = Chess::StartingPosition;
  if (!reset) {
    fen = activity->storage.loadPosition();
  }

  Chess::Parse::fen(fen, activity->board);
  activity->move = Chess::Move{};
  activity->moves = {};

  activity->pov = activity->board.turn;
  activity->infoText = "Let's go";

  activity->btnLeft = "";
  if (fen == Chess::StartingPosition) {
    activity->btnRight = "";
  }
}

///////////////////// OtbMoveState ////////////////////

OtbMoveState::OtbMoveState(ChessActivity* activity) : OtbState(activity) { activity->infoText = ""; }

ChessState* OtbMoveState::move(Chess::Move move) {
  activity->board.make(move);
  activity->storage.savePosition(Chess::Print::fen(activity->board));

  if (activity->game.result() != Chess::Game::Ongoing) {
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
  activity->move = activity->board.last;
  activity->board.undo();

  delete this;
  return new MoveFromState(activity, new OtbMoveState(activity));
}

ChessState* OtbMoveMadeState::down() {
  activity->pov = activity->board.turn;
  delete this;
  return new MoveStartState(activity, new OtbMoveState(activity));
}

///////////////////// OtbOverState ////////////////////

OtbOverState::OtbOverState(ChessActivity* activity) : OtbState(activity) {
  activity->btnUp = "";
  activity->btnDown = "";

  if (activity->game.result() == Chess::Game::Checkmate) {
    activity->statusText = "CHECKMATE!";
    if (activity->board.turn == Chess::White) {
      activity->infoText = "Game over. Black won.";
    } else {
      activity->infoText = "Game over. White won.";
    }
  } else {
    activity->statusText = "Stalemate =|";
    activity->infoText = "Game over. It's a draw.";
  }
}