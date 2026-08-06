#include "ChessEngine.h"

#include <cstdlib>
#include <vector>

std::vector<int> ChessEngine::board() {
  return {-6, -5, -4, -3, -2, -4, -5, -6, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0,
          0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0,
          0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  6,  5,  4,  3,  2, 4, 5, 6};
}

int ChessEngine::sideToMove() { return 0; }

Move ChessEngine::lastMove() { return Move{}; }

std::vector<Move> ChessEngine::legalMoves(int square) { return {}; }

void ChessEngine::makeMove(Move move) {}
