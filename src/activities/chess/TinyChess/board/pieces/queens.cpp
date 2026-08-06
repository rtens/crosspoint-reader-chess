#include "../board.h"

uint16_t Board::queenMoves(bool color, uint16_t* moves, int i) {
  uint64_t queensCopy = color == 0 ? whiteQueens : blackQueens;
  while (queensCopy != 0) {
    uint64_t isolatedQueen = queensCopy & ((~queensCopy) + 1);
    uint8_t from = 63 - __builtin_ctzll(isolatedQueen);
    uint64_t attacks = rookAttacks(from) | bishopAttacks(from);
    queenAttackers |= attacks;
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
    queensCopy &= ~isolatedQueen;
  }
  return i;
}
