#include <bitset>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "board.h"

int charToNum(std::string fen, int i) {
  if (fen[i] == 'p') {
    return BB_BLACK_PAWNS;
  }
  if (fen[i] == 'r') {
    return BB_BLACK_ROOKS;
  }
  if (fen[i] == 'n') {
    return BB_BLACK_KNIGHTS;
  }
  if (fen[i] == 'b') {
    return BB_BLACK_BISHOPS;
  }
  if (fen[i] == 'q') {
    return BB_BLACK_QUEENS;
  }
  if (fen[i] == 'k') {
    return BB_BLACK_KING;
  }

  if (fen[i] == 'P') {
    return BB_WHITE_PAWNS;
  }
  if (fen[i] == 'R') {
    return BB_WHITE_ROOKS;
  }
  if (fen[i] == 'N') {
    return BB_WHITE_KNIGHTS;
  }
  if (fen[i] == 'B') {
    return BB_WHITE_BISHOPS;
  }
  if (fen[i] == 'Q') {
    return BB_WHITE_QUEENS;
  }
  if (fen[i] == 'K') {
    return BB_WHITE_KING;
  }
  return -1;
}

uint8_t letterToNum(std::string l, int i) {
  if (l[i] == 'a') {
    return 0;
  }
  if (l[i] == 'b') {
    return 1;
  }
  if (l[i] == 'c') {
    return 2;
  }
  if (l[i] == 'd') {
    return 3;
  }
  if (l[i] == 'e') {
    return 4;
  }
  if (l[i] == 'f') {
    return 5;
  }
  if (l[i] == 'g') {
    return 6;
  }
  if (l[i] == 'h') {
    return 7;
  }
  return 0;
}

uint8_t stringNumToNum(std::string fen, int i) {
  if (fen[i] == '1') {
    return 1;
  }
  if (fen[i] == '2') {
    return 2;
  }
  if (fen[i] == '3') {
    return 3;
  }
  if (fen[i] == '4') {
    return 4;
  }
  if (fen[i] == '5') {
    return 5;
  }
  if (fen[i] == '6') {
    return 6;
  }
  if (fen[i] == '7') {
    return 7;
  }
  if (fen[i] == '8') {
    return 8;
  }

  return 0;
}

void Board::readFEN(std::string fen) {
  depth = 0;
  currentCapture = 0;
  wkscDepth = -1;
  wqscDepth = -1;
  bkscDepth = -1;
  bqscDepth = -1;
  flagWhiteKingsideCastle = 0;
  flagBlackKingsideCastle = 0;
  flagWhiteQueensideCastle = 0;
  flagBlackQueensideCastle = 0;
  std::fill_n(captures, 32, 0);
  std::fill_n(previousMover, 32, 0);
  std::fill_n(previousMoves, 32, 0);
  int row = 0;
  for (int i = 0; i < 12; i++) {
    *bitboards[i] = 0ULL;
  }
  int index = 0;
  fen = fen.substr(9, fen.length());
  // std::cout << fen.substr(0, 9) << "\n";
  if (fen.substr(0, 8) == "startpos") {
    fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" + fen.substr(8, fen.length());
  }
  if (fen.substr(0, 3) == "fen") {
    fen = fen.substr(4, fen.length());
  }
  fen += " ";
  // std::cout << fen << "\n";
  int hasSpace = 0;
  bool movePhase = false;
  uint8_t bankFrom;
  uint8_t bankTo;
  uint8_t flag = 0;
  bool capture = false;
  int castle = -1;
  int bankIndex = 0;
  uint8_t promo = 0;

  for (int i = 0; i < fen.length(); i++) {
    if (!movePhase) {
      if (fen[i] == '1') {
        index += 1;
        i++;
      }
      if (fen[i] == '2') {
        index += 2;
        i++;
      }
      if (fen[i] == '3') {
        index += 3;
        i++;
      }
      if (fen[i] == '4') {
        index += 4;
        i++;
      }
      if (fen[i] == '5') {
        index += 5;
        i++;
      }
      if (fen[i] == '6') {
        index += 6;
        i++;
      }
      if (fen[i] == '7') {
        index += 7;
        i++;
      }
      if (fen[i] == '8') {
        index += 8;
        i++;
      }
    }
    int bb = charToNum(fen, i);
    if (bb != -1 && hasSpace == 0) {
      *bitboards[bb] |= (1ULL << 63) >> (row * 8 + index);
    }
    if (fen[i] == '/') {
      row++;
      index = 0;
    } else {
      index++;
    }
    if (fen[i] == ' ') {
      hasSpace++;
    }
    // after
    if (fen[i] == 'w' && hasSpace == 1) {
      color = 0;
    }
    if (fen[i] == 'b' && hasSpace == 1) {
      color = 1;
    }
    if (fen[i] == 'K' && hasSpace == 2) {
      flagWhiteKingsideCastle = 1;
    }
    if (fen[i] == 'Q' && hasSpace == 2) {
      flagWhiteQueensideCastle = 1;
    }
    if (fen[i] == 'k' && hasSpace == 2) {
      flagBlackKingsideCastle = 1;
    }
    if (fen[i] == 'q' && hasSpace == 2) {
      flagBlackQueensideCastle = 1;
    }
    if (fen[i] == 'm') {
      movePhase = true;
    }
    if (hasSpace >= 7 && fen[i] == ' ') {
      if (bankIndex != 0) {
        if ((combinedOccupation() & (1ULL << (63 - bankTo))) != 0) {
          capture = true;
        }
        // std::cout << (int)bankFrom << " " << (int)bankTo << "\n";
        uint8_t castle = 0;
        currentCapture = 0;
        depth = 0;
        wkscDepth = -1;
        wqscDepth = -1;
        bkscDepth = -1;
        bqscDepth = -1;
        if (bankFrom == 60 && bankTo == 62 && color == 0 && flagWhiteKingsideCastle) {
          makeMove(0b0010111110111100);
        } else if (bankFrom == 60 && bankTo == 58 && color == 0 && flagWhiteQueensideCastle) {
          makeMove(0b0011000010001000);
        } else if (bankFrom == 4 && bankTo == 6 && color == 1 && flagBlackKingsideCastle) {
          makeMove(0b0010000110000100);
        } else if (bankFrom == 4 && bankTo == 2 && color == 1 && flagBlackQueensideCastle) {
          makeMove(0b0011000010000100);
        } else {
          bool doublePush = false;
          bool enPassant = false;
          if (((1ULL << (63 - bankFrom)) & (whitePawns | blackPawns)) != 0) {
            // std::cout << std::abs(bankFrom - bankTo) <<" Here\n";
            if (std::abs((int)bankFrom - (int)bankTo) == 16) {
              doublePush = true;
            }
            if (!capture) {
              if (((1ULL << (63 - bankFrom) << 1) & (color == 0 ? blackPawns : whitePawns)) != 0) {
                if (std::abs((int)bankFrom - (int)bankTo) == (color == 0 ? 9 : 7)) {
                  enPassant = true;
                  capture = false;
                }
              }
              if (((1ULL << (63 - bankFrom) >> 1) & (color == 0 ? blackPawns : whitePawns)) != 0) {
                if (std::abs((int)bankFrom - (int)bankTo) == (color == 0 ? 7 : 9)) {
                  enPassant = true;
                  capture = false;
                }
              }
            }
          }
          // d2d4 g8f6 c2c4 e7e6 g1f3 f8b4 c1d2 b8c6 d2b4 c6b4 h2h3 b4c6 f3e5 e8g8 g2g3 d8e8 f2f3 d7d6 e5g4 c8d7 g4h2
          // c6a5 e2e3 a8d8 b2b3 a5c6 a2a3 d6d5 c4c5 a7a6 h3h4 f6h5 h1g1 h5f6 g3g4 h7h6 f3f4 g7g5 h4g5 h6g5 f4g5 f6e4
          // h2f3 e6e5 f3e5 c6e5 d4e5 e8e5 a1a2 e5g5 d1c1 d7c6 b3b4 f7f5 f1d3 g5h4 e1d1 e4f2 a2f2 h4f2 g1h1 f5g4 d3h7
          // g8f7 b1d2 g4g3 h7c2 g3g2 c2g6 f7g6 c1b1 g6g5 h1h5 g5h5 b1h7 h5g4 h7g6 g4h3 g6h6 f2h4 h6e6 h4g4 e6g4 h3g4
          // e3e4 g2g1q d2f1 f8f2 d1c1 d5e4 c1b1 e4e3 b1a1 a6a5 a1b1 a5b4 b1a1 b4a3 a1b1 a3a2 b1a1 b7b5 c5b6

          makeMove(movePack(bankFrom, bankTo, doublePush, enPassant, promo, capture, 0));
        }
      }
      bankIndex = 0;
      bankFrom = 0;
      bankTo = 0;
      promo = 0;
      flag = 0;
      capture = false;
    }
    if (hasSpace >= 7 && fen[i] != ' ') {
      if (bankIndex == 0) {
        bankFrom = letterToNum(fen, i);
      }
      if (bankIndex == 1) {
        bankFrom += (8 - stringNumToNum(fen, i)) * 8;
      }
      if (bankIndex == 2) {
        bankTo = letterToNum(fen, i);
      }
      if (bankIndex == 3) {
        bankTo += (8 - stringNumToNum(fen, i)) * 8;
      }
      if (bankIndex == 4) {
        if (fen[i] == 'n') {
          promo = 1;
        }
        if (fen[i] == 'b') {
          promo = 2;
        }
        if (fen[i] == 'r') {
          promo = 3;
        }
        if (fen[i] == 'q') {
          promo = 4;
        }
      }
      bankIndex++;
    }
  }
  printBoard();
  // printAllBitBoards();
}

uint64_t Board::pieces(int type) { return *bitboards[type]; }

uint64_t Board::pieces(int type, bool color) { return *bitboards[type + 6 * color]; }

uint64_t Board::whiteOccupation() {
  return whitePawns | whiteRooks | whiteKnights | whiteBishops | whiteQueens | whiteKing;
}

uint64_t Board::blackOccupation() {
  return blackPawns | blackRooks | blackKnights | blackBishops | blackQueens | blackKing;
}

uint64_t Board::combinedOccupation() { return whiteOccupation() | blackOccupation(); }

int Board::distance(int sq1, int sq2) {
  int file1, file2, rank1, rank2;
  int rankDistance, fileDistance;
  file1 = sq1 & 7;
  file2 = sq2 & 7;
  rank1 = sq1 >> 3;
  rank2 = sq2 >> 3;
  rankDistance = std::abs(rank2 - rank1);
  fileDistance = std::abs(file2 - file1);
  return std::max(rankDistance, fileDistance);
}

int Board::hammingWeight(uint64_t x) {
  x -= (x >> 1) & 0x5555555555555555;                              // put count of each 2 bits into those 2 bits
  x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);  // put count of each 4 bits into those 4 bits
  x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0f;                         // put count of each 8 bits into those 8 bits
  return (x * 0x0101010101010101) >> 56;  // returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24)
}

int Board::pawnsCount(bool color) { return hammingWeight(color == WHITE ? whitePawns : blackPawns); }
int Board::knightsCount(bool color) { return hammingWeight(color == WHITE ? whiteKnights : blackKnights); }
int Board::bishopsCount(bool color) { return hammingWeight(color == WHITE ? whiteBishops : blackBishops); }
int Board::rooksCount(bool color) { return hammingWeight(color == WHITE ? whiteRooks : blackRooks); }
int Board::queensCount(bool color) { return hammingWeight(color == WHITE ? whiteQueens : blackQueens); }

void printNumToLetter(uint8_t a) {
  if (a % 8 == 0) {
    std::cout << "a";
  } else if (a % 8 == 1) {
    std::cout << "b";
  } else if (a % 8 == 2) {
    std::cout << "c";
  } else if (a % 8 == 3) {
    std::cout << "d";
  } else if (a % 8 == 4) {
    std::cout << "e";
  } else if (a % 8 == 5) {
    std::cout << "f";
  } else if (a % 8 == 6) {
    std::cout << "g";
  } else if (a % 8 == 7) {
    std::cout << "h";
  }
}
void stringNumToLetter(uint8_t a, std::stringstream& ss) {
  if (a % 8 == 0) {
    ss << "a";
  } else if (a % 8 == 1) {
    ss << "b";
  } else if (a % 8 == 2) {
    ss << "c";
  } else if (a % 8 == 3) {
    ss << "d";
  } else if (a % 8 == 4) {
    ss << "e";
  } else if (a % 8 == 5) {
    ss << "f";
  } else if (a % 8 == 6) {
    ss << "g";
  } else if (a % 8 == 7) {
    ss << "h";
  }
}
void pMove(uint16_t a) {
  uint8_t from = (a & 0b0000000000111111);
  uint8_t to = ((a >> 6) & 0b0000000000111111);
  printNumToLetter(from);
  std::cout << (int)(((63 - from) / 8 + 1));
  printNumToLetter(to);
  std::cout << (int)((63 - to) / 8 + 1);
}

void Board::printMove(uint16_t a, bool bin) {
  if (bin) {
    std::bitset<16> m = a;
    // std::bitset<4> flag = a >> 12;
    std::cout << "Binary Move:" << m << "\n";
    std::cout << "From:" << moveReadFromIndex(a) << "\n";
    std::cout << "To:" << (moveReadToIndex(a) >> 6) << "\n";
    std::cout << "Flag:" << moveReadFlags(a) << "\n";
  } else {
    const uint16_t flags = moveReadFlags(a);
    //    if (flags) {
    //      std::bitset<16> f = flags;
    //      std::cout << " (Flag: " << f << ")";
    //    }
    if (a == 0) {
      std::cout << "0000";
      return;
    }
    if (flags == FLAG_KING_CASTLE) {
      if (color == 0)
        std::cout << "e1g1";
      else
        std::cout << "e8g8";
    } else if (flags == FLAG_QUEEN_CASTLE) {
      if (color == 0)
        std::cout << "e1c1";
      else
        std::cout << "e8c8";
    } else {
      uint8_t from = (a & 0b0000000000111111);
      uint8_t to = ((a >> 6) & 0b0000000000111111);
      printNumToLetter(from);
      std::cout << (int)(((63 - from) / 8 + 1));
      printNumToLetter(to);
      std::cout << (int)((63 - to) / 8 + 1);
      if (flags == FLAG_KNIGHT_PROMOTION || flags == FLAG_KNIGHT_PROMOTION_CAPTURE) {
        std::cout << "n";
      } else if (flags == FLAG_BISHOP_PROMOTION || flags == FLAG_BISHOP_PROMOTION_CAPTURE) {
        std::cout << "b";
      } else if (flags == FLAG_ROOK_PROMOTION || flags == FLAG_ROOK_PROMOTION_CAPTURE) {
        std::cout << "r";
      } else if (flags == FLAG_QUEEN_PROMOTION || flags == FLAG_QUEEN_PROMOTION_CAPTURE) {
        std::cout << "q";
      }
    }
  }
}

std::string Board::moveToString(uint16_t a) {
  const uint16_t flags = moveReadFlags(a);

  if (a == 0) {
    return "0000";
  }

  std::stringstream ss;
  if (flags == FLAG_KING_CASTLE) {
    if (color == 0)
      ss << "e1g1";
    else
      ss << "e8g8";
  } else if (flags == FLAG_QUEEN_CASTLE) {
    if (color == 0)
      ss << "e1c1";
    else
      ss << "e8c8";
  } else {
    uint8_t from = (a & 0b0000000000111111);
    uint8_t to = ((a >> 6) & 0b0000000000111111);
    stringNumToLetter(from, ss);
    ss << (int)(((63 - from) / 8 + 1));
    stringNumToLetter(to, ss);
    ss << (int)((63 - to) / 8 + 1);
    if (flags == FLAG_KNIGHT_PROMOTION || flags == FLAG_KNIGHT_PROMOTION_CAPTURE) {
      ss << "n";
    } else if (flags == FLAG_BISHOP_PROMOTION || flags == FLAG_BISHOP_PROMOTION_CAPTURE) {
      ss << "b";
    } else if (flags == FLAG_ROOK_PROMOTION || flags == FLAG_ROOK_PROMOTION_CAPTURE) {
      ss << "r";
    } else if (flags == FLAG_QUEEN_PROMOTION || flags == FLAG_QUEEN_PROMOTION_CAPTURE) {
      ss << "q";
    }
  }
  return ss.str();
}

uint64_t Board::bitReverse(uint64_t b) {
  b = (b & 0x5555555555555555) << 1 | ((b >> 1) & 0x5555555555555555);
  b = (b & 0x3333333333333333) << 2 | ((b >> 2) & 0x3333333333333333);
  b = (b & 0x0f0f0f0f0f0f0f0f) << 4 | ((b >> 4) & 0x0f0f0f0f0f0f0f0f);
  b = (b & 0x00ff00ff00ff00ff) << 8 | ((b >> 8) & 0x00ff00ff00ff00ff);

  return (b << 48) | ((b & 0xffff0000) << 16) | ((b >> 16) & 0xffff0000) | (b >> 48);
}

// Hyperbola Quintessence for Sliding moves
// uint64_t Board::fileAttacks(uint64_t bitMask, uint8_t from){
//     uint64_t forward, reverse;
//     uint64_t fileMaskEx = (0x101010101010101ULL << (uint16_t)(7 - (from % 8))) & (~bitMask);
//     forward = combinedOccupation() & fileMaskEx;
//     reverse = bitReverse(forward);
//     forward -= bitMask;
//     reverse -= bitReverse(bitMask);
//     forward ^= bitReverse(reverse);
//     forward &= fileMaskEx;
//     return forward;
// }
// //Works ig
// uint64_t Board::rankAttacks(uint64_t s, uint8_t from){
//     uint64_t rank = (0xFFULL << ( 8*(7 - (uint8_t)(from / 8) )));
//     uint64_t o = (combinedOccupation() & rank);
//     uint64_t oPrime = bitReverse(o);
//     uint64_t sPrime = bitReverse(s);
//     uint64_t positiveRayAttacks = o ^ (o - 2*s);
//     uint64_t negativeRayAttacks = bitReverse(oPrime ^ (oPrime - 2*sPrime));
//     uint64_t lineAttacks = (o - 2*s) ^ bitReverse(oPrime - 2*sPrime);
//     return lineAttacks & rank;

// }

// Obstruction Difference
// uint64_t Board::obstructionDifferenceSus(uint64_t isol, uint64_t ray) {
//   uint64_t lower = (ray ^ isol) & (isol - 1);
//   uint64_t upper = (ray ^ isol) & (0xFFFFFFFFFFFFFFFF << (63 - from));
//   uint64_t linemaskEx = lower | upper;
//   lower &= combinedOccupation();
//   upper &= combinedOccupation();
//   uint64_t ms1B = 0x8000000000000000ULL >> __builtin_clzll(lower | 1);
//   uint64_t odiff = upper ^ (upper - ms1B);
//   return odiff & linemaskEx;
// }

uint64_t Board::obstructionDifference(uint8_t from, uint64_t ray) {
  uint64_t isol = 1ull << (63 - from);
  uint64_t lower = (ray ^ isol) & ((1ull << (63 - from)) - 1);
  uint64_t upper = (ray ^ isol) & (0xFFFFFFFFFFFFFFFF << (63 - from));
  uint64_t linemaskEx = lower | upper;
  lower &= combinedOccupation();
  upper &= combinedOccupation();
  uint64_t ms1B = 0x8000000000000000ULL >> __builtin_clzll(lower | 1);
  uint64_t odiff = upper ^ (upper - ms1B);
  return odiff & linemaskEx;
}

uint64_t Board::flipVertical(uint64_t x) {
  return ((x << 56)) | ((x << 40) & (0x00ff000000000000)) | ((x << 24) & (0x0000ff0000000000)) |
         ((x << 8) & (0x000000ff00000000)) | ((x >> 8) & (0x00000000ff000000)) | ((x >> 24) & (0x0000000000ff0000)) |
         ((x >> 40) & (0x000000000000ff00)) | ((x >> 56));
}
uint64_t Board::attackHQ(uint64_t mask, uint64_t isolated, uint8_t from) {
  uint64_t o = combinedOccupation() & mask;
  uint64_t r = bitReverse(o);
  return ((o - isolated) ^ bitReverse(r - (flipVertical(isolated)))) & mask;
}
