#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include "engine.h"
#define PAWNS 0
#define ROOKS 1
#define KNIGHTS 2
#define BISHOPS 3
#define QUEENS 4
#define KING 5

/* board representation */
#define WHITE 0
#define BLACK 1
int Engine::hammingWeight(uint64_t x) {
  x -= (x >> 1) & 0x5555555555555555;                              // put count of each 2 bits into those 2 bits
  x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);  // put count of each 4 bits into those 4 bits
  x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0f;                         // put count of each 8 bits into those 8 bits
  return (x * 0x0101010101010101) >> 56;  // returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24)
}

int32_t Engine::packScore(int16_t mg, int16_t eg) { return ((int32_t)eg << 16) + (int32_t)mg; }
int16_t Engine::unpack_mg(int32_t packed) { return (int16_t)packed; }

int16_t Engine::unpack_eg(int32_t packed) { return (int16_t)((packed + 0x8000) >> 16); }

int Engine::pstScores(uint64_t a, int& mg, int& eg, int i) {
  int gamePhase = 0;
  uint64_t cpy = a;
  while (cpy != 0) {
    uint64_t isolated = cpy & ((~cpy) + 1);
    int from = 63 - __builtin_ctzll(isolated);
    if (i == 0) {
      mg += pawnPST(from) + material[i].mg;
      eg += egPawnPST(from) + material[i].eg;
    }
    if (i == 1) {
      mg += knightPST(from) + material[i].mg;
      eg += egKnightPST(from) + material[i].eg;
    }
    if (i == 2) {
      mg += bishopPST(from) + material[i].mg;
      eg += egBishopPST(from) + material[i].eg;
    }
    if (i == 3) {
      mg += rookPST(from) + material[i].mg;
      eg += egRookPST(from) + material[i].eg;
    }
    if (i == 4) {
      mg += queenPST(from) + material[i].mg;
      eg += egQueenPST(from) + material[i].eg;
    }
    if (i == 5) {
      mg += kingPST(from) + material[i].mg;
      eg += egKingPST(from) + material[i].eg;
    }
    cpy &= ~isolated;
    gamePhase++;
  }
  return gamePhase;
}
int Engine::pieceSquareTables(int& mgS, int& egS) {
  // Pawns
  int score = 0;
  int whiteMG = 0;
  int whiteEG = 0;
  int blackMG = 0;
  int blackEG = 0;
  int gamePhase = 0;
  bool currColor = board->color;
  board->color = WHITE;
  gamePhase += (pstScores(board->whitePawns, whiteMG, whiteEG, 0)) * gamephaseInc[0];
  board->color = BLACK;
  gamePhase += (pstScores(board->blackPawns, blackMG, blackEG, 0)) * gamephaseInc[0];

  board->color = WHITE;
  gamePhase += (pstScores(board->whiteKnights, whiteMG, whiteEG, 1)) * gamephaseInc[1];
  board->color = BLACK;
  gamePhase += (pstScores(board->blackKnights, blackMG, blackEG, 1)) * gamephaseInc[1];

  board->color = WHITE;
  gamePhase += (pstScores(board->whiteBishops, whiteMG, whiteEG, 2)) * gamephaseInc[2];
  board->color = BLACK;
  gamePhase += (pstScores(board->blackBishops, blackMG, blackEG, 2)) * gamephaseInc[2];

  board->color = WHITE;
  gamePhase += (pstScores(board->whiteRooks, whiteMG, whiteEG, 3)) * gamephaseInc[3];
  board->color = BLACK;
  gamePhase += (pstScores(board->blackRooks, blackMG, blackEG, 3)) * gamephaseInc[3];

  board->color = WHITE;
  gamePhase += (pstScores(board->whiteQueens, whiteMG, whiteEG, 4)) * gamephaseInc[4];
  board->color = BLACK;
  gamePhase += (pstScores(board->blackQueens, blackMG, blackEG, 4)) * gamephaseInc[4];

  board->color = WHITE;
  gamePhase += (pstScores(board->whiteKing, whiteMG, whiteEG, 5)) * gamephaseInc[5];
  board->color = BLACK;
  gamePhase += (pstScores(board->blackKing, blackMG, blackEG, 5)) * gamephaseInc[5];

  board->color = currColor;

  int mgScore = board->color == WHITE ? whiteMG - blackMG : blackMG - whiteMG;
  int egScore = board->color == WHITE ? whiteEG - blackEG : blackEG - whiteEG;
  mgS += mgScore;
  egS += egScore;
  // std::cout << whiteMG << " " << blackMG << " " << mgScore << "\n";
  // std::cout << mgS << "\n";

  int mgPhase = gamePhase;
  if (mgPhase > 24) mgPhase = 24;
  int egPhase = 24 - mgPhase;
  phaseEG = egPhase;
  phaseMG = mgPhase;

  return (mgScore * mgPhase + egScore * egPhase) / 24;
}

int Engine::getDistanceBB(uint64_t bb, int sq, int scale) {
  uint64_t cpy = bb;
  int result = 0;
  while (cpy != 0) {
    uint64_t isolated = cpy & ((~cpy) + 1);
    int from = 63 - __builtin_ctzll(isolated);
    result += board->distance(sq, from) * scale;
    cpy &= ~isolated;
  }
  return result;
}

int Engine::doubledPawn(int& mg, int& eg, bool color) { return 0; }

bool moreThanOne(uint64_t bb) { return bb & (bb - 1); }

int Engine::staticEvaluation(uint64_t& attackers) {
  // Sided

  // Get king distance attackers
#ifdef USE_PST
  int mgScore = 0;
  int egScore = 0;
  int psts = pieceSquareTables(mgScore, egScore);
  int score = 0;
  int kingFrom = 63 - __builtin_ctzll(board->getKing());
  uint64_t kingArea = board->kingAttacks(board->getKing(), kingFrom);

  // King defense
  int cntKing = 0;

  cntKing = hammingWeight(kingArea & board->pieces(BISHOPS, !board->color));
  mgScore += cntKing * kingAttackers[0].mg;
  egScore += cntKing * kingAttackers[0].eg;

  cntKing = hammingWeight(kingArea & board->pieces(ROOKS, !board->color));
  mgScore += cntKing * kingAttackers[1].mg;
  egScore += cntKing * kingAttackers[1].eg;

  cntKing = hammingWeight(kingArea & board->pieces(QUEENS, !board->color));
  mgScore += cntKing * kingAttackers[2].mg;
  egScore += cntKing * kingAttackers[2].eg;

  // King defense (Opposing)
  kingFrom = 63 - __builtin_ctzll(board->pieces(KING, !board->color));
  kingArea = board->kingAttacks(board->pieces(KING, !board->color), kingFrom);

  cntKing = hammingWeight(kingArea & board->pieces(BISHOPS, board->color));
  mgScore -= cntKing * kingAttackers[0].mg;
  egScore -= cntKing * kingAttackers[0].eg;

  cntKing = hammingWeight(kingArea & board->pieces(ROOKS, board->color));
  mgScore -= cntKing * kingAttackers[1].mg;
  egScore -= cntKing * kingAttackers[1].eg;

  cntKing = hammingWeight(kingArea & board->pieces(QUEENS, board->color));
  mgScore -= cntKing * kingAttackers[2].mg;
  egScore -= cntKing * kingAttackers[2].eg;

  // Rook Mobility
  uint64_t cpy = board->pieces(ROOKS, board->color);
  while (cpy != 0) {
    uint64_t isolated = cpy & ((~cpy) + 1);
    uint8_t from = 63 - __builtin_ctzll(isolated);
    uint64_t attacks = board->rookAttacks(from);
    uint64_t file = 0x101010101010101ULL << (7 - (from % 8));
    mgScore += rookMobility[hammingWeight(attacks)].mg;
    egScore += rookMobility[hammingWeight(attacks)].eg;
    if (!(file & board->pieces(PAWNS, board->color))) {
      if (!(file & board->pieces(PAWNS, !board->color))) {
        mgScore += 12;
        egScore += 0;
      } else {
        // mgScore += 1;
        // egScore -= 7;
      }
    }
    cpy &= ~isolated;
  }
  cpy = board->pieces(ROOKS, !board->color);
  while (cpy != 0) {
    uint64_t isolated = cpy & ((~cpy) + 1);
    uint8_t from = 63 - __builtin_ctzll(isolated);
    uint64_t attacks = board->rookAttacks(from);
    uint64_t file = 0x101010101010101ULL << (7 - (from % 8));
    mgScore -= rookMobility[hammingWeight(attacks)].mg;
    egScore -= rookMobility[hammingWeight(attacks)].eg;

    if (!(file & board->pieces(PAWNS, !board->color))) {
      if (!(file & board->pieces(PAWNS, board->color))) {
        mgScore -= 12;
        egScore -= 0;
      } else {
        // mgScore -= 1;
        // egScore += 7;
      }
    }
    cpy &= ~isolated;
  }
  // Bishop Mobility
  cpy = board->pieces(BISHOPS, board->color);
  while (cpy != 0) {
    uint64_t isolated = cpy & ((~cpy) + 1);
    uint8_t from = 63 - __builtin_ctzll(isolated);
    uint64_t attacks = board->bishopAttacks(from);
    mgScore += bishopMobility[hammingWeight(attacks)].mg;
    egScore += bishopMobility[hammingWeight(attacks)].eg;
    cpy &= ~isolated;
  }
  cpy = board->pieces(BISHOPS, !board->color);
  while (cpy != 0) {
    uint64_t isolated = cpy & ((~cpy) + 1);
    uint8_t from = 63 - __builtin_ctzll(isolated);
    uint64_t attacks = board->bishopAttacks(from);
    mgScore -= bishopMobility[hammingWeight(attacks)].mg;
    egScore -= bishopMobility[hammingWeight(attacks)].eg;
    cpy &= ~isolated;
  }

  cpy = board->pieces(QUEENS, board->color);
  while (cpy != 0) {
    uint64_t isolated = cpy & ((~cpy) + 1);
    uint8_t from = 63 - __builtin_ctzll(isolated);
    uint64_t attacks = board->bishopAttacks(from) | board->rookAttacks(from);
    mgScore += queenMobility[hammingWeight(attacks)].mg;
    egScore += queenMobility[hammingWeight(attacks)].eg;
    cpy &= ~isolated;
  }
  cpy = board->pieces(QUEENS, !board->color);
  while (cpy != 0) {
    uint64_t isolated = cpy & ((~cpy) + 1);
    uint8_t from = 63 - __builtin_ctzll(isolated);
    uint64_t attacks = board->bishopAttacks(from) | board->rookAttacks(from);
    mgScore -= queenMobility[hammingWeight(attacks)].mg;
    egScore -= queenMobility[hammingWeight(attacks)].eg;
    cpy &= ~isolated;
  }
  // Pawns
  // cpy = board->pieces(PAWNS, board->color);
  // while (cpy != 0) {
  //   uint64_t isolated = cpy & ((~cpy) + 1);
  //   uint8_t from = 63 - __builtin_ctzll(isolated);
  //   uint64_t file = 0x101010101010101ULL << (7 - (from % 8));

  //   if ((from % 8) != 0 && (from % 8) != 7){
  //     if (!((file | (file << 1) | (file >> 1)) & board->pieces(PAWNS, board->color))){
  //       mgScore += -22;
  //       egScore += 14;
  //     }
  //   }
  //   cpy &= ~isolated;
  // }
  // cpy = board->pieces(PAWNS, !board->color);
  // while (cpy != 0) {
  //   uint64_t isolated = cpy & ((~cpy) + 1);
  //   uint8_t from = 63 - __builtin_ctzll(isolated);
  //   uint64_t file = 0x101010101010101ULL << (7 - (from % 8));

  //   if ((from % 8) != 0 && (from % 8) != 7){
  //     if (!((file | (file << 1) | (file >> 1)) & board->pieces(PAWNS, !board->color))){
  //       mgScore -= -22;
  //       egScore -= 14;
  //     }
  //   }
  //   cpy &= ~isolated;
  // }

  if (hammingWeight(board->pieces(BISHOPS, board->color)) >= 2) {
    mgScore += 22;
    egScore += 68;
  }
  if (hammingWeight(board->pieces(BISHOPS, !board->color)) >= 2) {
    mgScore -= 22;
    egScore -= 68;
  }
  //------------- Doubled Pawns ------------------
  // for (int i=0;i<8;i++){
  //   uint64_t file = 0x101010101010101ULL << i;
  //   mgScore += hammingWeight(file & board->pieces(PAWNS, board->color)) * (-18);
  //   egScore += hammingWeight(file & board->pieces(PAWNS, board->color)) * (-36);

  //   mgScore -= hammingWeight(file & board->pieces(PAWNS, !board->color)) * (-18);
  //   egScore -= hammingWeight(file & board->pieces(PAWNS, !board->color)) * (-36);
  // }

  score = (mgScore * phaseMG + egScore * phaseEG) / 24;
  return score;
#else
  int whitePawns = this->board->pawnsCount(0);
  int blackPawns = this->board->pawnsCount(1);

  int whiteKnights = this->board->knightsCount(0);
  int blackKnights = this->board->knightsCount(1);

  int whiteBishops = this->board->bishopsCount(0);
  int blackBishops = this->board->bishopsCount(1);

  int whiteRooks = this->board->rooksCount(0);
  int blackRooks = this->board->rooksCount(1);

  int whiteQueens = this->board->queensCount(0);
  int blackQueens = this->board->queensCount(1);

  float whiteMaterial = whitePawns * PAWN_MATERIAL_VALUE + whiteKnights * KNIGHT_MATERIAL_VALUE +
                        whiteBishops * BISHOP_MATERIAL_VALUE + whiteRooks * ROOK_MATERIAL_VALUE +
                        whiteQueens * QUEEN_MATERIAL_VALUE;

  float blackMaterial = blackPawns * PAWN_MATERIAL_VALUE + blackKnights * KNIGHT_MATERIAL_VALUE +
                        blackBishops * BISHOP_MATERIAL_VALUE + blackRooks * ROOK_MATERIAL_VALUE +
                        blackQueens * QUEEN_MATERIAL_VALUE;
  float MD = whiteMaterial - blackMaterial;
  MD = MD < 0 ? -MD : MD;
  float PA = whiteMaterial > blackMaterial ? whitePawns : blackPawns;
  float MS = std::min(2400.0f, MD) + (MD * PA * (8000 - (whiteMaterial + blackMaterial))) / (6400 * (PA + 1));
  MS = std::min(3100.0f, MS);
  // return MS * (this->board->color == 1 ? -1 : 1);
  //  White has more material
  if (whiteMaterial > blackMaterial) {
    if (this->board->color == 1) {
      MS *= -1;
    }
  } else {
    if (this->board->color == 0) {
      MS *= -1;
    }
  }
  return MD;
#endif
}

// cpy = board->color == WHITE ? board->whiteBishops : board->blackBishops;
// while (cpy != 0) {
//   uint64_t isolatedRook = cpy & ((~cpy) + 1);
//   uint8_t from = 63 - __builtin_ctzll(isolatedRook);
//   uint64_t attacks = board->bishopAttacks(from);
//   mgScore += bishopMobilityMG[hammingWeight(attacks)];
//   //egScore += bishopMobilityEG[hammingWeight(attacks)];
//   cpy &= ~isolatedRook;
// }

// passedPawn(board->color == WHITE ? board->whitePawns : board->blackPawns, board->color == BLACK ? board->whitePawns :
// board->blackPawns, mgScore, egScore);

// Double attacks
// uint64_t doubleAttacks = (board->queenAttackers & board->rookAttackers) | (board->queenAttackers &
// board->bishopAttackers) | (board->queenAttackers & board->knightAttackers) | (board->queenAttackers &
// board->pawnAttackers); doubleAttacks |= (board->rookAttackers & board->bishopAttackers) | (board->rookAttackers &
// board->knightAttackers) | (board->rookAttackers & board->pawnAttackers); doubleAttacks |= (board->bishopAttackers &
// board->knightAttackers) | (board->bishopAttackers & board->pawnAttackers); doubleAttacks |= (board->knightAttackers &
// board->pawnAttackers);

// Weak squares
// uint64_t weak = 0ULL;
// uint64_t queenAttacks = 0ULL;
// cpy = board->color == WHITE ? board->whiteBishops : board->blackBishops;
// while (cpy != 0) {
//   uint64_t isolatedRook = cpy & ((~cpy) + 1);
//   uint8_t from = 63 - __builtin_ctzll(isolatedRook);
//   queenAttacks |= board->bishopAttacks(from) | board->rookAttacks(from);
//   cpy &= ~isolatedRook;
// }
// //flankDefense += hammingWeight(queenAttacks & kingFlank);

// weak = attackers & (queenAttacks | kingArea);
// kingDanger += 183 * hammingWeight(weak & kingArea);
// kingDanger -= 6 * mgScore / 8 + 37;
// kingDanger -= 1.5 * flankDefense;
// kingDanger -= 4 * (hammingWeight(attackers & kingFlank) + hammingWeight(attackers & kingFlank & doubleAttacks));

// mgScore -= kingDanger * kingDanger / 4096;
// egScore -= kingDanger/16;

// if (hammingWeight(board->color == WHITE ? board->whiteBishops : board->blackBishops) == 2){
//   mgScore += 33;
//   egScore += 65;
// }
// if (hammingWeight(board->color == BLACK ? board->whiteBishops : board->blackBishops) == 2){
//   mgScore -= 33;
//   egScore -= 65;
// }