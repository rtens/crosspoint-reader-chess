#include "Otb.h"

#include "../ChessActivity.h"
#include "../ChessState.h"
#include "Move.h"

OtbStartState::OtbStartState(ChessActivity* activity) : OtbGoingState(activity) {
  activity->game.start();
  activity->last = Move{};
  activity->pov = activity->game.turn;
  activity->infoText = "Let's go";
}

OtbGoingState::OtbGoingState(ChessActivity* activity) : ChessState(activity) {
  activity->infoText = "";
  activity->btnRight = "Reset";
}

ChessState* OtbGoingState::right() {
  delete this;
  return new OtbStartState(activity);
}

ChessState* OtbGoingState::move(Move move) {
  activity->game.make(move);
  activity->last = move;
  activity->pov = activity->game.turn;
  return new MoveStartState(activity, new OtbGoingState(activity));
}