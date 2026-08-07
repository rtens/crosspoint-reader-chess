#include "board.h"

#include <stdio.h>

#include <iostream>
#include <sstream>

void moveSet(uint16_t& move, uint8_t from, uint8_t to, bool doublePawnPush, bool enPassant, uint8_t promo, bool capture,
             uint8_t castle) {
  move = (uint16_t)from;
  move |= (uint16_t)to << 6;
  uint8_t flags = 0;
  if (doublePawnPush) {
    flags = 0b00000001;
  } else if (castle != 0) {
    flags = 0b0010 + (castle - 1);
  } else if (capture) {
    if (promo != 0) {
      flags = 0b1100 + (promo - 1);
    } else {
      flags = 0b0100;
    }
  } else if (promo != 0) {
    flags = 0b1000 + (promo - 1);
  } else if (enPassant) {
    flags = 0b0101;
  }

  move |= (uint16_t)flags << 12;
}

uint16_t movePack(uint8_t from, uint8_t to, bool doublePawnPush, bool enPassant, uint8_t promo, bool capture,
                  uint8_t castle) {
  uint16_t move = (uint16_t)0b0000000000000000;
  moveSet(move, from, to, doublePawnPush, enPassant, promo, capture, castle);
  return move;
}

Board::Board() {
  this->whitePawns = 0x000000000000FF00ULL;
  this->blackPawns = 0x00FF000000000000ULL;
  this->whiteKnights = 0x0000000000000042ULL;
  this->blackKnights = 0x4200000000000000ULL;
  this->whiteBishops = 0x0000000000000024ULL;
  this->blackBishops = 0x2400000000000000ULL;
  this->whiteRooks = 0x0000000000000081ULL;
  this->blackRooks = 0x8100000000000000ULL;
  this->whiteQueens = 0x0000000000000010ULL;
  this->blackQueens = 0x1000000000000000ULL;
  this->whiteKing = 0x000000000000008ULL;
  this->blackKing = 0x0800000000000000ULL;

  // this->blackRooks = 0;
  // this->whiteRooks = 0;
  // this->whiteKnights <<= 17;
  // this->blackPawns >>= 16;
  // this->blackPawns &= (0b11110111ULL << 48) ;
  // this->whitePawns &= (0b11110111ULL << 8) ;
  //   this->whitePawns >>= 15;
  //   this->whitePawns <<= 15;
  //   this->whitePawns = 0;

  // this->whiteRooks <<= 24;
  // this->whiteQueens <<= 24;
  // this->whiteKing <<= 24;
  // this->whiteKnights <<= 32;
  // this->blackQueens >>= 9;

  // uint64_t** bitboards = new uint64_t*[12];
  this->bitboards = new uint64_t*[12];
  this->bitboards[BB_WHITE_PAWNS] = &this->whitePawns;
  this->bitboards[BB_WHITE_ROOKS] = &this->whiteRooks;
  this->bitboards[BB_WHITE_KNIGHTS] = &this->whiteKnights;
  this->bitboards[BB_WHITE_BISHOPS] = &this->whiteBishops;
  this->bitboards[BB_WHITE_QUEENS] = &this->whiteQueens;
  this->bitboards[BB_WHITE_KING] = &this->whiteKing;
  this->bitboards[BB_BLACK_PAWNS] = &this->blackPawns;
  this->bitboards[BB_BLACK_ROOKS] = &this->blackRooks;
  this->bitboards[BB_BLACK_KNIGHTS] = &this->blackKnights;
  this->bitboards[BB_BLACK_BISHOPS] = &this->blackBishops;
  this->bitboards[BB_BLACK_QUEENS] = &this->blackQueens;
  this->bitboards[BB_BLACK_KING] = &this->blackKing;

  pawnAttackers = 0ULL;
  rookAttackers = 0ULL;
  knightAttackers = 0ULL;
  bishopAttackers = 0ULL;
  queenAttackers = 0ULL;
  kingAttackers = 0ULL;

  this->color = WHITE;
}

Board::~Board() { delete[] this->bitboards; }

void Board::printBitBoard(uint64_t bb) {
  for (uint8_t y = 0; y < 8; y++) {
    for (uint8_t x = 0; x < 8; x++) {
      const uint8_t index = y * 8 + x;
      printf("%d", bitRead(bb, index));
    }
    printf("\n");
  }
  printf("\n");
}

void Board::printAllBitBoards() {
  std::cout << "White Pawns\n";
  printBitBoard(whitePawns);
  std::cout << "Black Pawns\n";
  printBitBoard(blackPawns);

  std::cout << "White Rooks\n";
  printBitBoard(whiteRooks);
  std::cout << "Black Rooks\n";
  printBitBoard(blackRooks);

  std::cout << "White Knights\n";
  printBitBoard(whiteKnights);
  std::cout << "Black Knights\n";
  printBitBoard(blackKnights);

  std::cout << "White Bishops\n";
  printBitBoard(whiteBishops);
  std::cout << "Black Bishops\n";
  printBitBoard(blackBishops);

  std::cout << "White Queens\n";
  printBitBoard(whiteQueens);
  std::cout << "Black Queens\n";
  printBitBoard(blackQueens);

  std::cout << "White King\n";
  printBitBoard(whiteKing);
  std::cout << "Black King\n";
  printBitBoard(blackKing);
}

std::array<int, 64> Board::all_pieces() {
  std::array<int, 64> pieces = {};
  for (uint8_t index = 0; index < 64; index++) {
    if (bitRead(this->whitePawns, index)) {
      pieces[index] = 1;
    } else if (bitRead(this->blackPawns, index)) {
      pieces[index] = -1;
    } else if (bitRead(this->whiteKnights, index)) {
      pieces[index] = 5;
    } else if (bitRead(this->blackKnights, index)) {
      pieces[index] = -5;
    } else if (bitRead(this->whiteBishops, index)) {
      pieces[index] = 4;
    } else if (bitRead(this->blackBishops, index)) {
      pieces[index] = -4;
    } else if (bitRead(this->whiteRooks, index)) {
      pieces[index] = 6;
    } else if (bitRead(this->blackRooks, index)) {
      pieces[index] = -6;
    } else if (bitRead(this->whiteQueens, index)) {
      pieces[index] = 3;
    } else if (bitRead(this->blackQueens, index)) {
      pieces[index] = -3;
    } else if (bitRead(this->whiteKing, index)) {
      pieces[index] = 2;
    } else if (bitRead(this->blackKing, index)) {
      pieces[index] = -2;
    } else {
      pieces[index] = 0;
    }
  }
  return pieces;
}

void Board::printBoard() {
  for (uint8_t y = 0; y < 8; y++) {
    for (uint8_t x = 0; x < 8; x++) {
      const uint8_t index = y * 8 + x;
      if (bitRead(this->whitePawns, index)) {
        printf("P");
      } else if (bitRead(this->blackPawns, index)) {
        printf("p");
      } else if (bitRead(this->whiteKnights, index)) {
        printf("N");
      } else if (bitRead(this->blackKnights, index)) {
        printf("n");
      } else if (bitRead(this->whiteBishops, index)) {
        printf("B");
      } else if (bitRead(this->blackBishops, index)) {
        printf("b");
      } else if (bitRead(this->whiteRooks, index)) {
        printf("R");
      } else if (bitRead(this->blackRooks, index)) {
        printf("r");
      } else if (bitRead(this->whiteQueens, index)) {
        printf("Q");
      } else if (bitRead(this->blackQueens, index)) {
        printf("q");
      } else if (bitRead(this->whiteKing, index)) {
        printf("K");
      } else if (bitRead(this->blackKing, index)) {
        printf("k");
      } else {
        printf(".");
      }
      printf(" ");
    }
    printf("\n");
  }
  printf("\n");
}

std::string Board::printFEN() {
  std::stringstream ss;

  int empties = 0;
  for (uint8_t y = 0; y < 8; y++) {
    for (uint8_t x = 0; x < 8; x++) {
      const uint8_t index = y * 8 + x;
      std::string symbol = "";

      if (bitRead(this->whitePawns, index)) {
        symbol = "P";
      } else if (bitRead(this->blackPawns, index)) {
        symbol = "p";
      } else if (bitRead(this->whiteKnights, index)) {
        symbol = "N";
      } else if (bitRead(this->blackKnights, index)) {
        symbol = "n";
      } else if (bitRead(this->whiteBishops, index)) {
        symbol = "B";
      } else if (bitRead(this->blackBishops, index)) {
        symbol = "b";
      } else if (bitRead(this->whiteRooks, index)) {
        symbol = "R";
      } else if (bitRead(this->blackRooks, index)) {
        symbol = "r";
      } else if (bitRead(this->whiteQueens, index)) {
        symbol = "Q";
      } else if (bitRead(this->blackQueens, index)) {
        symbol = "q";
      } else if (bitRead(this->whiteKing, index)) {
        symbol = "K";
      } else if (bitRead(this->blackKing, index)) {
        symbol = "k";
      }

      if (symbol == "") {
        empties++;
      } else {
        if (empties > 0) {
          ss << empties;
          empties = 0;
        }
        ss << symbol;
      }
    }
    if (empties > 0) {
      ss << empties;
      empties = 0;
    }
    if (y < 7) {
      ss << "/";
    }
  }

  if (color == 0) {
    ss << " w ";
  } else {
    ss << " b ";
  }

  bool noCastle = true;
  if (flagWhiteKingsideCastle == 1) {
    ss << "K";
    noCastle = false;
  }
  if (flagWhiteQueensideCastle == 1) {
    ss << "Q";
    noCastle = false;
  }
  if (flagBlackKingsideCastle == 1) {
    ss << "k";
    noCastle = false;
  }
  if (flagBlackQueensideCastle == 1) {
    ss << "q";
    noCastle = false;
  }

  if (noCastle) {
    ss << "-";
  }

  ss << " -";

  return ss.str();
}