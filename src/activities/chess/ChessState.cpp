#include "ChessState.h"

ChessState::ChessState(ChessActivity* a) { activity = a; }

ChessState ChessState::up() {}

ChessState ChessState::down() {}

ChessState ChessState::left() {}

ChessState ChessState::right() {}

ChessState ChessState::move(Move move) {}
