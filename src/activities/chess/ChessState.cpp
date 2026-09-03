#include "ChessState.h"

ChessState::ChessState(ChessActivity* a) { activity = a; }

ChessState* ChessState::up() { return this; }

ChessState* ChessState::down() { return this; }

ChessState* ChessState::left() { return this; }

ChessState* ChessState::right() { return this; }

ChessState* ChessState::move(Chess::Move move) { return this; }
