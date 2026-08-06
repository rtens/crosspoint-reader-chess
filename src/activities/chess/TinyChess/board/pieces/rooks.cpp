#include "../board.h"

uint64_t Board::rankAttacks(uint8_t from) {
  uint64_t rank = (0xFFULL << (8 * (7 - (uint8_t)(from / 8))));
  return obstructionDifference(from, rank);
}

uint64_t Board::fileAttacks(uint8_t from) {
  uint64_t file = (0x101010101010101ULL << (uint8_t)(7 - (from % 8)));

  return obstructionDifference(from, file);
}

uint64_t Board::rookAttacks(uint8_t from) {
  return rankAttacks(from) | fileAttacks(from);
}

uint16_t Board::rookMoves(bool color, uint16_t* moves, int i) {
  uint64_t rooksCopy = color == 0 ? whiteRooks : blackRooks;
  while (rooksCopy != 0) {
    uint64_t isolatedRook = rooksCopy & ((~rooksCopy) + 1);
    uint8_t from = 63 - __builtin_ctzll(isolatedRook);
    uint64_t attacks = rookAttacks(from);
    rookAttackers |= attacks;
    while (attacks != 0) {
      uint64_t isolatedAttack = attacks & ((~attacks) + 1);
      uint8_t to = 63 - __builtin_ctzll(isolatedAttack);
      // tis a capture of the opposing color
      if (((color == 0 ? blackOccupation() : whiteOccupation()) & isolatedAttack) != 0) {
        moves[i] = (movePack(from, to, false, false, false, true, 0));
        i++;
      } else if (((color == 0 ? whiteOccupation() : blackOccupation()) & isolatedAttack) == 0) {
        // No capture and not our own color
        moves[i] = (movePack(from, to, false, false, false, false, 0));
        i++;
      }
      attacks &= ~isolatedAttack;
    }
    rooksCopy &= ~isolatedRook;
  }
  return i;
}
