#include <iostream>

#include "../board.h"
int Board::generateMoves(uint16_t* moves, uint64_t& pawnAttacks, uint64_t& rookAttacks, uint64_t& knightAttacks,
                         uint64_t& bishopAttacks, uint64_t& queenAttacks, uint64_t& kingAttacks) {
  int i = 0;
  if (color == 0) {
    // Checking for checks in castling squares
    if (flagWhiteKingsideCastle) {
      if ((combinedOccupation() & 6ULL) == 0 && (whiteRooks & 1ULL) != 0) {
        moves[i] = 0b0010111110111100;
        i++;
      }
    }
    if (flagWhiteQueensideCastle) {
      if ((combinedOccupation() & 112ULL) == 0 && (whiteRooks & 128ULL) != 0) {
        moves[i] = 0b0011000010001000;
        i++;
      }
    }
  }
  if (color == 1) {
    // Checking for checks in castling squares
    if (flagBlackKingsideCastle) {
      if ((combinedOccupation() & (6ULL << 56)) == 0 && (blackRooks & (1ULL << 56)) != 0) {
        moves[i] = 0b0010000110000100;
        i++;
      }
    }
    if (flagBlackQueensideCastle) {
      if ((combinedOccupation() & (112ULL << 56)) == 0 && (blackRooks & (128ULL << 56)) != 0) {
        moves[i] = 0b0011000010000100;
        i++;
      }
    }
  }

  resetAttackers();
  i = pawnMoves(color, moves, i);
  i = rookMoves(color, moves, i);
  i = knightMoves(color, moves, i);
  i = bishopMoves(color, moves, i);
  i = queenMoves(color, moves, i);
  i = kingMoves(color, moves, i);
  // position startpos moves d2d4 g8f6 c2c4 e7e6 g1f3 f8b4 c1d2 b8c6 d2b4 c6b4 h2h3 b4c6 f3e5 e8g8 g2g3 d8e8 f2f3 d7d6
  // e5g4 c8d7 g4h2 c6a5 e2e3 a8d8 b2b3 a5c6 a2a3 d6d5 c4c5 a7a6 h3h4 f6h5 h1g1 h5f6 g3g4 h7h6 f3f4 g7g5 h4g5 h6g5 f4g5
  // f6e4 h2f3 e6e5 f3e5 c6e5 d4e5 e8e5 a1a2 e5g5 d1c1 d7c6 b3b4 f7f5 f1d3 g5h4 e1d1 e4f2 a2f2 h4f2 g1h1 f5g4 d3h7 g8f7
  // b1d2 g4g3 h7c2 g3g2 c2g6 f7g6 c1b1 g6g5 h1h5 g5h5 b1h7 h5g4 h7g6 g4h3 g6h6 f2h4 h6e6 h4g4 e6g4 h3g4 e3e4 g2g1q d2f1
  // f8f2 d1c1 d5e4 c1b1 e4e3 b1a1 a6a5 a1b1 a5b4 b1a1 b4a3 a1b1 a3a2 b1a1 b7b5
  pawnAttacks = pawnAttackers;
  rookAttacks = rookAttackers;
  knightAttacks = knightAttackers;
  bishopAttacks = bishopAttackers;
  queenAttacks = queenAttackers;
  kingAttacks = kingAttackers;
  fenEnpassant = false;
  return i;
}

int Board::movesFromIndex(int i, uint16_t* moves) {
  if (i == 0) return pawnMoves(0, moves, 0);
  if (i == 1) return rookMoves(0, moves, 0);
  if (i == 2) return knightMoves(0, moves, 0);
  if (i == 3) return bishopMoves(0, moves, 0);
  if (i == 4) return queenMoves(0, moves, 0);
  if (i == 5) return kingMoves(0, moves, 0);

  if (i == 6) return pawnMoves(1, moves, 0);
  if (i == 7) return rookMoves(1, moves, 0);
  if (i == 8) return knightMoves(1, moves, 0);
  if (i == 9) return bishopMoves(1, moves, 0);
  if (i == 10) return queenMoves(1, moves, 0);
  if (i == 11) return kingMoves(1, moves, 0);

  return 0;
}

void Board::resetAttackers() {
  pawnAttackers = 0ULL;
  rookAttackers = 0ULL;
  knightAttackers = 0ULL;
  bishopAttackers = 0ULL;
  queenAttackers = 0ULL;
  kingAttackers = 0ULL;
}

void Board::setAttackers(uint64_t& pawnAttacks, uint64_t& rookAttacks, uint64_t& knightAttacks, uint64_t& bishopAttacks,
                         uint64_t& queenAttacks, uint64_t& kingAttacks) {
  pawnAttackers = pawnAttacks;
  rookAttackers = rookAttacks;
  knightAttackers = knightAttacks;
  bishopAttackers = bishopAttacks;
  queenAttackers = queenAttacks;
  kingAttackers = kingAttacks;
}

uint64_t Board::getAttackers() {
  return pawnAttackers | rookAttackers | knightAttackers | bishopAttackers | queenAttackers | kingAttackers;
}

uint64_t Board::getKing() { return color == 0 ? whiteKing : blackKing; }

bool Board::isLegal(uint64_t& attackers, bool wasInCheck) {
  // color 0 is white
  uint16_t flag = (previousMoves[depth - 1] & 0b1111000000000000) >> 12;
  if (flag == 0b0011) {
    if (color == 1) {
      if ((120ULL & attackers) != 0) {
        return false;
      }
    }
    if (color == 0) {
      if (((120ULL << 56) & attackers) != 0) {
        // printBoard();
        return false;
      }
    }
  }
  if (flag == 0b0010) {
    if (color == 1) {
      if ((14ULL & attackers) != 0) {
        return false;
      }
    }
    if (color == 0) {
      if (((14ULL << 56) & attackers) != 0) {
        return false;
      }
    }
  }
  uint64_t isolated = color == 1 ? whiteKing : blackKing;
  int from = 63 - __builtin_ctzll(isolated);
  uint64_t attacks;
  uint64_t prevTo = 1ULL << (63 - ((previousMoves[depth - 1] & 0b0000111111000000) >> 6));
  uint64_t prevFrom = 1ULL << (63 - ((previousMoves[depth - 1] & 0b0000000000111111)));
  int prevToNum = (((previousMoves[depth - 1] & 0b0000111111000000) >> 6));
  int prevFromNum = previousMoves[depth - 1] & 0b0000000000111111;
  if (previousMover[depth - 1] == BB_WHITE_KING || previousMover[depth - 1] == BB_BLACK_KING) {
    // Was a king move and we can use cached vision data
    // printBitBoard(attackers);
    // printBoard();
    if ((attackers & isolated) != 0) {
      return false;
    }
  }

  bool calculateRooks = true;
  bool calculateBishops = true;

  // This says that if there is no bishop/queen on the diagonal ray from the king, dont generate it's attack set
  // etc for rooks
  uint64_t rank = (0xFFULL << (8 * (7 - (uint8_t)(from / 8))));
  uint64_t file = (0x101010101010101ULL << (uint8_t)(7 - (from % 8)));
  int diag = 7 - ((63 - from) & 7) - ((63 - from) >> 3);
  uint64_t positiveDiagonalRay = diag >= 0 ? 0x0102040810204080ULL >> diag * 8 : 0x0102040810204080ULL << -diag * 8;
  diag = ((63 - from) & 7) - ((63 - from) >> 3);
  uint64_t negativeDiagonalRay = diag >= 0 ? 0x8040201008040201ULL >> diag * 8 : 0x8040201008040201ULL << -diag * 8;
  uint64_t diagonal = positiveDiagonalRay | negativeDiagonalRay;
  uint64_t lines = rank | file;
  uint64_t colorRooks = color == 0 ? whiteRooks | whiteQueens : blackRooks | blackQueens;
  uint64_t colorBishops = color == 0 ? whiteBishops | whiteQueens : blackBishops | blackQueens;
  if ((lines & colorRooks) == 0) {
    calculateRooks = false;
  }
  if ((diagonal & colorBishops) == 0) {
    calculateBishops = false;
  }

  // First check for DIRECT king checks
  //  Rook attacks
  if (calculateRooks) {
    attacks = rookAttacks(from);
    if ((attacks & (color == 1 ? blackRooks : whiteRooks)) != 0 ||
        (attacks & (color == 1 ? blackQueens : whiteQueens))) {
      return false;
    }
  }

  // Bishops
  if (calculateBishops) {
    attacks = bishopAttacks(from);
    if ((attacks & (color == 1 ? blackBishops : whiteBishops)) != 0 ||
        (attacks & (color == 1 ? blackQueens : whiteQueens))) {
      return false;
    }
  }
  // Knights
  attacks = knightAttacks(isolated, from);
  if ((attacks & (color == 1 ? blackKnights : whiteKnights)) != 0) {
    return false;
  }
  // Pawns
  attacks = 0;
  if (color == 1) {
    if (from % 8 != 0) attacks |= isolated << 9;
    if ((from + 1) % 8 != 0) attacks |= isolated << 7;
  } else {
    if ((from + 1) % 8 != 0) attacks |= isolated >> 9;
    if (from % 8 != 0) attacks |= isolated >> 7;
  }
  if ((attacks & (color == 1 ? blackPawns : whitePawns)) != 0) {
    return false;
  }
  return true;
}

void Board::sliceReAdd() {
  uint64_t prevTo = 1ULL << (63 - ((previousMoves[depth] & 0b0000111111000000) >> 6));
  uint16_t flag = (previousMoves[depth] >> 12);

  if (previousMover[depth] == 3 || previousMover[depth] == 9 || (bishopAttackers & prevTo) != 0) {
    bishopAttackers = 0;
    // Regenerate bishops only
    uint64_t bishopsCopy = color == 0 ? whiteBishops : blackBishops;
    while (bishopsCopy != 0) {
      uint64_t isolatedBishop = bishopsCopy & ((~bishopsCopy) + 1);
      uint8_t from = 63 - __builtin_ctzll(isolatedBishop);
      uint64_t attacks = bishopAttacks(from);
      bishopAttackers |= attacks;
      bishopsCopy &= ~isolatedBishop;
    }
  }
  // Rook
  if (previousMover[depth] == 1 || previousMover[depth] == 7 || (rookAttackers & prevTo) != 0 || flag == 0b0010 ||
      flag == 0b0011) {
    rookAttackers = 0;
    // Regenerate bishops only
    uint64_t rooksCopy = color == 0 ? whiteRooks : blackRooks;
    while (rooksCopy != 0) {
      uint64_t isolatedRook = rooksCopy & ((~rooksCopy) + 1);
      uint8_t from = 63 - __builtin_ctzll(isolatedRook);
      uint64_t attacks = rookAttacks(from);
      rookAttackers |= attacks;
      rooksCopy &= ~isolatedRook;
    }
  }
  // Queen
  if (previousMover[depth] == 4 || previousMover[depth] == 10 || (queenAttackers & prevTo) != 0) {
    queenAttackers = 0;
    // Regenerate bishops only
    uint64_t queensCopy = color == 0 ? whiteQueens : blackQueens;
    while (queensCopy != 0) {
      uint64_t isolatedQueen = queensCopy & ((~queensCopy) + 1);
      uint8_t from = 63 - __builtin_ctzll(isolatedQueen);
      uint64_t attacks = rookAttacks(from) | bishopAttacks(from);
      queenAttackers |= attacks;
      queensCopy &= ~isolatedQueen;
    }
  }
  // Pawn
  if (previousMover[depth] == 0 || previousMover[depth] == 6 || (pawnAttackers & prevTo) != 0) {
    pawnAttackers = 0;
    uint64_t pawnsCopy = color == 0 ? whitePawns : blackPawns;
    while (pawnsCopy != 0) {
      uint64_t isolatedPawn = pawnsCopy & ((~pawnsCopy) + 1);
      int from = 63 - __builtin_ctzll(isolatedPawn);
      uint64_t movements = 0;
      // Attacks/Captures
      // Check color and generate diagonal attack square
      if (color == 0) {
        if (from % 8 != 0) movements |= isolatedPawn << 9;
        if ((from + 1) % 8 != 0) movements |= isolatedPawn << 7;
      } else {
        if ((from + 1) % 8 != 0) movements |= isolatedPawn >> 9;
        if (from % 8 != 0) movements |= isolatedPawn >> 7;
      }
      // Iterate over attack squares and see if the move is valid (capturable piece)
      pawnAttackers |= movements;
      pawnsCopy &= ~isolatedPawn;
    }
  }
  // King
  if (previousMover[depth] == 5 || previousMover[depth] == 11 || flag == 0b0010 || flag == 0b0011) {
    kingAttackers = 0;
    uint64_t kingsCopy = color == 0 ? whiteKing : blackKing;
    while (kingsCopy != 0) {
      uint64_t isolatedKing = kingsCopy & ((~kingsCopy) + 1);
      uint8_t from = 63 - __builtin_ctzll(isolatedKing);
      uint64_t attacks = kingAttacks(isolatedKing, from);
      kingAttackers |= attacks;
      kingsCopy &= ~isolatedKing;
    }
  }
  // Knight
  if (previousMover[depth] == 2 || previousMover[depth] == 8) {
    knightAttackers = 0;
    uint64_t knightsCopy = color == 0 ? whiteKnights : blackKnights;
    while (knightsCopy != 0) {
      uint64_t isolatedKnight = knightsCopy & ((~knightsCopy) + 1);
      uint8_t from = 63 - __builtin_ctzll(isolatedKnight);
      uint64_t attacks = knightAttacks(isolatedKnight, from);
      knightAttackers |= attacks;
      knightsCopy &= ~isolatedKnight;
    }
  }
}

void Board::makeMove(uint16_t a) {
  previousMoves[depth] = a;
  uint8_t from = (a & 0b0000000000111111);

  uint8_t to = ((a >> 6) & 0b0000000000111111);
  uint16_t flag = (a & 0b1111000000000000) >> 12;

  uint64_t fromBitboard = 1ULL << (63 - from);
  uint64_t toBitboard = 1ULL << (63 - to);
  bool isCapture = (flag == 0b0100 || flag >= 0b1100);
  bool isEP = (flag == 0b0101);

  if (flag == 0b0011) {
    // printBoard();
    if (color == 0) {
      whiteKing <<= 2;
      whiteRooks &= (~128ULL);
      whiteRooks |= (whiteKing >> 1);

      flagWhiteQueensideCastle = 0;
      wqscDepth = depth;
      flagWhiteKingsideCastle = 0;
      wkscDepth = 0;

      previousMover[depth] = 5;
    }
    if (color == 1) {
      blackKing <<= 2;
      blackRooks &= (~(128ULL << 56));
      blackRooks |= (blackKing >> 1);

      flagBlackKingsideCastle = 0;
      bkscDepth = depth;
      flagBlackQueensideCastle = 0;
      bqscDepth = depth;

      previousMover[depth] = 11;
    }
    color = !color;
    depth++;
    return;
  }
  if (flag == 0b0010) {
    if (color == 0) {
      whiteKing >>= 2;
      whiteRooks &= (~1ULL);
      whiteRooks |= (whiteKing << 1);

      flagWhiteQueensideCastle = 0;
      wqscDepth = depth;
      flagWhiteKingsideCastle = 0;
      wkscDepth = 0;

      previousMover[depth] = 5;
    }
    if (color == 1) {
      blackKing >>= 2;
      blackRooks &= (~(1ULL << 56));
      blackRooks |= (blackKing << 1);

      flagBlackKingsideCastle = 0;
      bkscDepth = depth;
      flagBlackQueensideCastle = 0;
      bqscDepth = depth;

      previousMover[depth] = 11;
    }
    color = !color;

    depth++;

    return;
  }

  // Move piece to position first
  // First iterate over all 12 piece bitboards and select it based on from and to
  if (isEP) {
    if ((*bitboards[color == 0 ? 6 : 0] & (color == 0 ? (toBitboard >> 8) : toBitboard << 8)) != 0) {
      *bitboards[color == 0 ? 6 : 0] &= (~(color == 0 ? (toBitboard >> 8) : toBitboard << 8));
      captures[currentCapture] = color == 0 ? 6 : 0;
      currentCapture++;
    }
  }
  for (int i = 0; i < 12; i++) {
    // This means a piece is present at 'from'
    // Check if it's a capture
    if (isCapture) {
      // Remove the piece at 'to' square
      // Remember what we captured
      if ((*bitboards[i] & toBitboard) != 0) {
        *bitboards[i] &= (~toBitboard);
        captures[currentCapture] = i;
        currentCapture++;
      }
    }

    if ((*bitboards[i] & fromBitboard) != 0) {
      // Remove the piece abd replace it to its new position
      *bitboards[i] &= (~fromBitboard);
      *bitboards[i] |= toBitboard;
      previousMover[depth] = i;
    }
  }
  // Knight promo
  if (flag == 0b1100 || flag == 0b1000) {
    // Remove blub
    *bitboards[previousMover[depth]] &= (~toBitboard);
    if (color == 0) *bitboards[BB_WHITE_KNIGHTS] |= toBitboard;
    if (color == 1) *bitboards[BB_BLACK_KNIGHTS] |= toBitboard;
  }
  // Bishop promo
  if (flag == 0b1101 || flag == 0b1001) {
    // Remove blub
    *bitboards[previousMover[depth]] &= (~toBitboard);
    if (color == 0) *bitboards[BB_WHITE_BISHOPS] |= toBitboard;
    if (color == 1) *bitboards[BB_BLACK_BISHOPS] |= toBitboard;
  }
  // Rook Promo
  if (flag == 0b1110 || flag == 0b1010) {
    // Remove blub
    *bitboards[previousMover[depth]] &= (~toBitboard);
    if (color == 0) *bitboards[BB_WHITE_ROOKS] |= toBitboard;
    if (color == 1) *bitboards[BB_BLACK_ROOKS] |= toBitboard;
  }
  // Queen Promo
  if (flag == 0b1111 || flag == 0b1011) {
    // Remove blub
    *bitboards[previousMover[depth]] &= (~toBitboard);
    if (color == 0) *bitboards[BB_WHITE_QUEENS] |= toBitboard;
    if (color == 1) *bitboards[BB_BLACK_QUEENS] |= toBitboard;
  }

  if (color == 0) {
    // Right white rook has moved/been captured
    // White King moved
    if (((whiteKing & 8ULL) == 0 || (whiteRooks & 1ULL) == 0) && flagWhiteKingsideCastle == 1) {
      // Trip the bit
      wkscDepth = depth;
      flagWhiteKingsideCastle = 0;  // Set the flag to unable to castle
    }
    // Queenside castle
    if (((whiteKing & 8ULL) == 0 || (whiteRooks & 128ULL) == 0) && flagWhiteQueensideCastle == 1) {
      // Trip the bit
      wqscDepth = depth;
      flagWhiteQueensideCastle = 0;  // Set the flag to unable to castle
    }
  }
  if (color == 1) {
    if (((blackKing & (8ULL << 56)) == 0 || (blackRooks & (1ULL << 56)) == 0) && flagBlackKingsideCastle == 1) {
      // Trip the bit
      bkscDepth = depth;
      flagBlackKingsideCastle = 0;  // Set the flag to unable to castle
    }
    // Queenside castle
    if (((blackKing & (8ULL << 56)) == 0 || (blackRooks & (128ULL << 56)) == 0) && flagBlackQueensideCastle == 1) {
      // Trip the bit
      bqscDepth = depth;
      flagBlackQueensideCastle = 0;  // Set the flag to unable to castle
    }
  }

  sliceReAdd();
  color = !color;
  depth++;
}

void Board::unmakeMove(uint16_t a) {
  // if (a == 0b0000111100111101){
  //   std::cout << "FOUND\n";
  // }
  depth--;
  previousMoves[depth] = 0;
  if (depth == wkscDepth) {
    // Weve reached the depth where the castling flag
    // was tripped and so we undo it since we have gone back to the move
    flagWhiteKingsideCastle = 1;
    wkscDepth = -1;
  }
  if (depth == bkscDepth) {
    // Weve reached the depth where the castling flag
    // was tripped and so we undo it since we have gone back to the move
    flagBlackKingsideCastle = 1;
    bkscDepth = -1;
  }
  if (depth == wqscDepth) {
    flagWhiteQueensideCastle = 1;
    wqscDepth = -1;
  }
  if (depth == bqscDepth) {
    // Weve reached the depth where the castling flag
    // was tripped and so we undo it since we have gone back to the move
    flagBlackQueensideCastle = 1;
    bqscDepth = -1;
  }

  uint8_t from = (a & 0b0000000000111111);
  uint8_t to = ((a >> 6) & 0b0000000000111111);
  uint8_t flag = ((a >> 12) & 0b1111);
  uint64_t fromBitboard = 1ULL << (63 - from);
  uint64_t toBitboard = 1ULL << (63 - to);
  bool isCapture = (flag == 0b0100 || flag >= 0b1100);
  bool isEP = (flag == 0b0101);

  if (flag == 0b0010) {
    if (color == 1) {
      whiteKing <<= 2;
      whiteRooks &= ~(whiteKing >> 1);
      whiteRooks |= 1ULL;
      flagWhiteKingsideCastle = 1;
      // std::cout << "Unm,aked\n";
      wkscDepth = -1;
    }
    if (color == 0) {
      blackKing <<= 2;
      blackRooks &= ~(blackKing >> 1);
      blackRooks |= (1ULL << 56);
      flagBlackKingsideCastle = 1;
      // std::cout << "Unm,aked\n";
      bkscDepth = -1;
    }
    previousMover[depth] = -1;
    color = !color;
    return;
  }
  if (flag == 0b0011) {
    // printBoard();
    if (color == 1) {
      whiteKing >>= 2;
      whiteRooks &= ~(whiteKing << 1);
      whiteRooks |= (128ULL);
      flagWhiteQueensideCastle = 1;
      wqscDepth = -1;
    }
    if (color == 0) {
      blackKing >>= 2;
      blackRooks &= ~(blackKing << 1);
      blackRooks |= (128ULL << 56);
      flagBlackQueensideCastle = 1;
      bqscDepth = -1;
    }
    previousMover[depth] = -1;
    color = !color;
    return;
  }

  // Move piece to position first
  // First iterate over all 12 piece bitboards and select it based on from and to
  // Remove the piece and move back

  // Knight promo
  if (flag == 0b1100 || flag == 0b1000) {
    if (color == BLACK) {
      *bitboards[BB_WHITE_KNIGHTS] &= (~toBitboard);
      *bitboards[previousMover[depth]] |= fromBitboard;
    }
    if (color == WHITE) {
      *bitboards[BB_BLACK_KNIGHTS] &= (~toBitboard);
      *bitboards[previousMover[depth]] |= fromBitboard;
    }
  }
  // Bishop promo
  if (flag == 0b1101 || flag == 0b1001) {
    if (color == BLACK) {
      *bitboards[BB_WHITE_BISHOPS] &= (~toBitboard);
      *bitboards[previousMover[depth]] |= fromBitboard;
    }
    if (color == WHITE) {
      *bitboards[BB_BLACK_BISHOPS] &= (~toBitboard);
      *bitboards[previousMover[depth]] |= fromBitboard;
    }
  }
  // Rook Promo
  if (flag == 0b1110 || flag == 0b1010) {
    if (color == BLACK) {
      *bitboards[BB_WHITE_ROOKS] &= (~toBitboard);
      *bitboards[previousMover[depth]] |= fromBitboard;
    }
    if (color == WHITE) {
      *bitboards[BB_BLACK_ROOKS] &= (~toBitboard);
      *bitboards[previousMover[depth]] |= fromBitboard;
    }
  }
  // Queen Promo
  if (flag == 0b1111 || flag == 0b1011) {
    if (color == BLACK) {
      *bitboards[BB_WHITE_QUEENS] &= (~toBitboard);
      *bitboards[previousMover[depth]] |= fromBitboard;
    }
    if (color == WHITE) {
      *bitboards[BB_BLACK_QUEENS] &= (~toBitboard);
      *bitboards[previousMover[depth]] |= fromBitboard;
    }
  }

  if ((*bitboards[previousMover[depth]] & toBitboard) != 0) {
    // Remove the piece abd replace it to its new position
    *bitboards[previousMover[depth]] &= (~toBitboard);
    *bitboards[previousMover[depth]] |= fromBitboard;
  }

  if (isCapture) {
    *bitboards[captures[currentCapture - 1]] |= toBitboard;
    captures[currentCapture - 1] = -1;
    currentCapture--;
  }
  if (isEP) {
    *bitboards[color == 1 ? 6 : 0] |= ((color == 1 ? (toBitboard >> 8) : toBitboard << 8));
    captures[currentCapture] = -1;
    currentCapture--;
  }
  sliceReAdd();
  previousMover[depth] = -1;
  color = !color;
}
