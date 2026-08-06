#include "ChessEngine.h"

#include <array>
#include <cstdlib>
#include <vector>

void ChessEngine::newGame() { board.readFEN("position startpos"); }

std::array<int, 64> ChessEngine::pieces() { return board.all_pieces(); }

std::vector<int> ChessEngine::myPieces(int side) {
  auto pieces = this->pieces();

  std::vector<int> mine = {};
  for (int p = 0; p < 64; p++) {
    if (side == WHITE_SIDE && pieces[p] > 0) {
      mine.push_back(p);
    } else if (side == BLACK_SIDE && pieces[p] < 0) {
      mine.push_back(p);
    }
  }

  sort(mine.begin(), mine.end());
  return mine;
}

int ChessEngine::sideToMove() { return board.color; }

Move ChessEngine::lastMove() { return Move{}; }

std::vector<Move> ChessEngine::legalMoves(int square) {
  uint64_t pawnAttacks, rookAttacks, knightAttacks, bishopAttacks, queenAttacks, kingAttacks;
  uint16_t moves[218];
  int total =
      board.generateMoves(moves, pawnAttacks, rookAttacks, knightAttacks, bishopAttacks, queenAttacks, kingAttacks);

  std::vector<Move> legals = {};
  for (int i = 0; i < total; i++) {
    uint16_t move = moves[i];
    uint8_t from = (move & 0b0000000000111111);
    uint8_t to = ((move >> 6) & 0b0000000000111111);

    if (from == square) {
      legals.push_back(Move{from, to, move});
    }
  }

  sort(legals.begin(), legals.end(), [](Move a, Move b) { return a.to < b.to; });
  return legals;
}

void ChessEngine::makeMove(Move move) { board.makeMove(move.bin); }
