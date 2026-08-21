#include "EngineStates.h"

#include <cstdlib>
#include <ctime>
using namespace std;

#include "../ChessActivity.h"
#include "../ChessState.h"
#include "Engine.h"
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

  activity->move = Move{};
  activity->last = Move{};
  activity->moves.clear();

  if (reset) {
    activity->infoText = "Good luck";
    activity->game.start(Game::STARTPOS);

    srand(time(0));
    activity->pov = (rand() % 2) ? Game::WHITE : Game::BLACK;

    if (activity->pov == Game::BLACK) {
      activity->last = activity->engine.respond();
      activity->game.make(activity->last);
    }

  } else {
    activity->infoText = "Your turn";
    activity->game.start(activity->storage.loadPosition());
    activity->pov = activity->game.turn;
  }
}

ChessState* EngineRunningState::move(Move move) {
  activity->game.make(move);
  activity->last = move;

  if (activity->game.isOver()) {
    delete this;
    return new EngineOverState(activity, "You");
  }

  activity->infoText = "Thinking...";
  activity->requestUpdateAndWait();

  activity->last = activity->engine.respond();
  activity->game.make(activity->last);
  activity->infoText = "Your turn";

  if (activity->game.isOver()) {
    delete this;
    return new EngineOverState(activity, "I");
  }

  activity->storage.savePosition(activity->game.fen());
  activity->movesSinceRefresh += 2;

  return new MoveStartState(activity, this);
}

///////////////// EngineOverState ////////////

EngineOverState::EngineOverState(ChessActivity* activity, string winner) : EngineState(activity) {
  activity->btnUp = "";
  activity->btnDown = "";
  activity->btnLeft = "";

  if (activity->game.isOver() == Game::CHECKMATE) {
    activity->statusText = "CHECKMATE!";
    activity->infoText = "Game over. " + winner + " won.";
  } else {
    activity->statusText = "Stalemate =|";
    activity->infoText = "Game over. It's a draw.";
  }
}