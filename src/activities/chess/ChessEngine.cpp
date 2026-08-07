#include "ChessEngine.h"

#include <Logging.h>

#include <array>
#include <cstdlib>
#include <sstream>
#include <vector>

void ChessEngine::newGame(std::string position) { board.readFEN(position); }

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

std::string ChessEngine::printMove(Move move) { return board.moveToString(move.bin); }

std::vector<Move> ChessEngine::legalMoves(int square) {
  std::vector<Move> legals = {};

  LOG_DBG("CHESS-legalMoves", "getAttackers");
  uint64_t attackers = board.getAttackers();
  uint16_t moves[218];
  LOG_DBG("CHESS-legalMoves", "resetAttackers");
  board.resetAttackers();
  uint64_t pawnAttacks, rookAttacks, knightAttacks, bishopAttacks, queenAttacks, kingAttacks;
  LOG_DBG("CHESS-legalMoves", "generateMoves");
  int total =
      board.generateMoves(moves, pawnAttacks, rookAttacks, knightAttacks, bishopAttacks, queenAttacks, kingAttacks);

  for (int i = 0; i < total; i++) {
    LOG_DBG("CHESS-legalMoves", "check %u", i);
    uint16_t move = moves[i];
    uint8_t from = (move & 0b0000000000111111);
    uint8_t to = ((move >> 6) & 0b0000000000111111);

    if (from != square) continue;

    LOG_DBG("CHESS-legalMoves", "setAttackers");
    board.setAttackers(pawnAttacks, rookAttacks, knightAttacks, bishopAttacks, queenAttacks, kingAttacks);
    LOG_DBG("CHESS-legalMoves", "getKing");
    uint64_t isolated = board.getKing();
    bool check = (attackers & isolated) != 0;
    LOG_DBG("CHESS-legalMoves", "makeMove");
    board.makeMove(move);

    LOG_DBG("CHESS-legalMoves", "isLegal");
    if (board.isLegal(attackers, check)) {
      LOG_DBG("CHESS-legalMoves", "push_back");
      legals.push_back(Move{from, to, move});
    }
    LOG_DBG("CHESS-legalMoves", "unmakeMove");
    board.unmakeMove(move);
  }

  LOG_DBG("CHESS-legalMoves", "sort");
  sort(legals.begin(), legals.end(), [](Move a, Move b) { return a.to < b.to; });

  LOG_DBG("CHESS-legalMoves", "return");
  return legals;
}

void ChessEngine::makeMove(Move move) { board.makeMove(move.bin); }

void ChessEngine::undoLastMove() {}

Move ChessEngine::respond() {
  int sc = 0;
  int nodes = 0;
  uint16_t response = engine.runSearchID(100, sc, nodes);

  board.makeMove(response);

  uint8_t from = (response & 0b0000000000111111);
  uint8_t to = ((response >> 6) & 0b0000000000111111);
  return Move{from, to, response};
}

int ChessEngine::gameOver() { return -1; }

std::string ChessEngine::printPosition() { return "position fen " + board.printFEN(); }