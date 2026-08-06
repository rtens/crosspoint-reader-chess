#include "../board.h"
#include <iostream>

uint64_t Board::knightAttacks(uint64_t isolated, uint8_t from) {
  uint64_t attacks = 0;
  if (from % 8 != 0) {
    attacks |= isolated << 17;
    attacks |= isolated >> 15;
    // Not A or B file
    if ((from - 1) % 8 != 0) {
      attacks |= isolated << 10;
      attacks |= isolated >> 6;
    }
  }

  // Not H File
  if ((from + 1) % 8 != 0) {
    attacks |= isolated << 15;
    attacks |= isolated >> 17;
    // Not H or F File
    if ((from + 2) % 8 != 0) {
      attacks |= isolated << 6;
      attacks |= isolated >> 10;
    }
  }
  return attacks;
}

uint16_t Board::knightMoves(bool color, uint16_t* moves, int i) {
  uint64_t knightsCopy = color == 0 ? whiteKnights : blackKnights;
  while (knightsCopy != 0) {
    uint64_t isolatedKnight = knightsCopy & ((~knightsCopy) + 1);
    uint8_t from = 63 - __builtin_ctzll(isolatedKnight);
    uint64_t attacks = knightAttacks(isolatedKnight, from);
    knightAttackers |= attacks;
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
    knightsCopy &= ~isolatedKnight;
  }
  return i;
}
