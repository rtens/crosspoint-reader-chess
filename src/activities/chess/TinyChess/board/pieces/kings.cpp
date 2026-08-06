#include "../board.h"

uint64_t Board::kingAttacks(uint64_t isolated, uint8_t from) {
  uint64_t attacks = 0ULL;
  attacks |= isolated << 8;
  attacks |= isolated >> 8;
  if ((from) % 8 != 0) {
    // Not on A file
    attacks |= isolated << 1;
    attacks |= isolated >> 7;
    attacks |= isolated << 9;
    //if ((from + 1) % 8 != 0)
      
  }
  if ((from + 1) % 8 != 0) {
    // Not on A file
    attacks |= isolated >> 1;
    attacks |= isolated >> 9;
    attacks |= isolated << 7;
    //if (from % 8 != 0)
      
  }
  return attacks;
}

uint16_t Board::kingMoves(bool color, uint16_t* moves, int i) {
  uint64_t kingsCopy = color == 0 ? whiteKing : blackKing;
  while (kingsCopy != 0) {
    uint64_t isolatedKing = kingsCopy & ((~kingsCopy) + 1);
    uint8_t from = 63 - __builtin_ctzll(isolatedKing);
    uint64_t attacks = kingAttacks(isolatedKing, from);
    kingAttackers |= attacks;
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
    kingsCopy &= ~isolatedKing;
  }
  return i;
}
