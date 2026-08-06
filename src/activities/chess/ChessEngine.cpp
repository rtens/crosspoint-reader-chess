#include "ChessEngine.h"

#include <array>
#include <cstdlib>
#include <vector>

void ChessEngine::newGame() { board.readFEN("position startpos"); }

std::array<int, 64> ChessEngine::pieces() { return board.all_pieces(); }

std::vector<int> ChessEngine::myPieces() {
  auto pieces = this->pieces();
  std::vector<int> mine = {};
  for (int p = 0; p < 64; p++) {
    if (pieces[p] > 0) {
      mine.push_back(p);
    }
  }
  return mine;
}

int ChessEngine::sideToMove() { return 0; }

Move ChessEngine::lastMove() { return Move{}; }

std::vector<Move> ChessEngine::legalMoves(int square) { return {}; }

void ChessEngine::makeMove(Move move) {}
