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

std::vector<Move> ChessEngine::legalMoves(int square) {
  uint64_t attackers = board.getAttackers();
  uint16_t moves[218];
  board.resetAttackers();
  uint64_t pawnAttacks, rookAttacks, knightAttacks, bishopAttacks, queenAttacks, kingAttacks;
  int total =
      board.generateMoves(moves, pawnAttacks, rookAttacks, knightAttacks, bishopAttacks, queenAttacks, kingAttacks);

  std::vector<Move> legals = {};
  for (int i = 0; i < total; i++) {
    board.setAttackers(pawnAttacks, rookAttacks, knightAttacks, bishopAttacks, queenAttacks, kingAttacks);
    uint64_t isolated = board.getKing();
    bool check = (attackers & isolated) != 0;
    board.makeMove(moves[i]);
    uint64_t a = board.getAttackers();

    if (board.isLegal(attackers, check)) {
      uint16_t move = moves[i];
      uint8_t from = (move & 0b0000000000111111);
      uint8_t to = ((move >> 6) & 0b0000000000111111);

      if (from == square) {
        legals.push_back(Move{from, to, move});
      }
    }
    board.unmakeMove(moves[i]);
  }

  sort(legals.begin(), legals.end(), [](Move a, Move b) { return a.to < b.to; });
  return legals;
}

void ChessEngine::makeMove(Move move) { board.makeMove(move.bin); }

Move ChessEngine::respond() {
  int sc = 0;
  int nodes = 0;
  uint16_t response = engine.runSearchID(500, sc, nodes);
  board.makeMove(response);

  uint8_t from = (response & 0b0000000000111111);
  uint8_t to = ((response >> 6) & 0b0000000000111111);
  return Move{from, to, response};
}

int ChessEngine::eval() { return engine.getEval(); }