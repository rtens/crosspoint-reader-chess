#include "EngineStates.h"

#include <cstdlib>
#include <ctime>
using namespace std;

#include <Chess/Engine.h>
#include <Chess/Parse.h>
#include <Chess/Print.h>

#include "../ChessActivity.h"
#include "../ChessState.h"
#include "MoveStates.h"

///////////////////// EngineState ////////////////////

EngineState::EngineState(ChessActivity* activity) : ChessState(activity) { activity->btnRight = "New"; }

ChessState* EngineState::right() {
  delete this;
  return new MoveStartState(activity, new EngineRunningState(activity, true));
}

///////////////////// EngineRunningState ////////////////////

EngineRunningState::EngineRunningState(ChessActivity* activity, bool reset) : EngineState(activity) {
  activity->btnLeft = "";

  activity->move = Chess::Move{};
  activity->moves.clear();

  if (reset) {
    activity->infoText = "Good luck";
    Chess::Parse::fen(Chess::StartingPosition, activity->board);

    srand(time(0));
    activity->pov = (rand() % 2) ? Chess::White : Chess::Black;

    if (activity->pov == Chess::Black) {
      activity->engine->respond();
    }

  } else {
    activity->infoText = "Your turn";
    Chess::Parse::fen(activity->storage.loadPosition(), activity->board);
    activity->pov = activity->board.turn;
  }
}

ChessState* EngineRunningState::move(Chess::Move move) {
  activity->board.make(move);

  if (activity->game.result() != Chess::Game::Ongoing) {
    delete this;
    return new EngineOverState(activity, "You");
  }

  activity->infoText = "Thinking...";
  activity->requestUpdateAndWait();

  activity->engine->respond();
  activity->infoText = "Your turn";

  if (activity->game.result() != Chess::Game::Ongoing) {
    delete this;
    return new EngineOverState(activity, "I");
  }

  activity->storage.savePosition(Chess::Print::fen(activity->board));
  activity->movesSinceRefresh += 2;

  return new MoveStartState(activity, this);
}

///////////////// EngineOverState ////////////

EngineOverState::EngineOverState(ChessActivity* activity, string winner) : EngineState(activity) {
  activity->btnUp = "";
  activity->btnDown = "";
  activity->btnLeft = "";

  if (activity->game.result() == Chess::Game::Checkmate) {
    activity->statusText = "CHECKMATE!";
    activity->infoText = "Game over. " + winner + " won.";
  } else {
    activity->statusText = "Stalemate =|";
    activity->infoText = "Game over. It's a draw.";
  }
}