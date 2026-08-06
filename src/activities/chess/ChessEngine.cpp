#include "ChessEngine.h"

#include <array>
#include <cstdlib>
#include <vector>

void ChessEngine::newGame() { board.readFEN("position startpos"); }

std::array<int, 64> ChessEngine::pieces() { return board.all_pieces(); }

int ChessEngine::sideToMove() { return 0; }

Move ChessEngine::lastMove() { return Move{}; }

std::vector<Move> ChessEngine::legalMoves(int square) { return {}; }

void ChessEngine::makeMove(Move move) {}
