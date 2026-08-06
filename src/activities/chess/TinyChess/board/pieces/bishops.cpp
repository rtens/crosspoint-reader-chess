#include "../board.h"

uint64_t Board::bishopAttacks(uint8_t from) {
  int diag = 7 - ((63 - from) & 7) - ((63 - from) >> 3);
  uint64_t positiveDiagonalRay = diag >= 0 ? 0x0102040810204080ULL >> diag * 8 : 0x0102040810204080ULL << -diag * 8;
  diag = ((63 - from) & 7) - ((63 - from) >> 3);
  uint64_t negativeDiagonalRay = diag >= 0 ? 0x8040201008040201ULL >> diag * 8 : 0x8040201008040201ULL << -diag * 8;
  return obstructionDifference(from, positiveDiagonalRay) | obstructionDifference(from, negativeDiagonalRay);
}

uint16_t Board::bishopMoves(bool color, uint16_t* moves, int i) {
  uint64_t bishopsCopy = color == 0 ? this->whiteBishops : blackBishops;
  while (bishopsCopy != 0) {
    uint64_t isolatedBishop = bishopsCopy & ((~bishopsCopy) + 1);
    uint8_t from = 63 - __builtin_ctzll(isolatedBishop);
    uint64_t attacks = this->bishopAttacks(from);
    bishopAttackers |= attacks;
    while (attacks != 0) {
      uint64_t isolatedAttack = attacks & ((~attacks) + 1);
      uint8_t to = 63 - __builtin_ctzll(isolatedAttack);
      // tis a capture of the opposing color
      if (((color == 0 ? this->blackOccupation() : this->whiteOccupation()) & isolatedAttack) != 0) {
        moves[i] = (movePack(from, to, false, false, false, true, 0));
        i++;
      } else if (((color == 0 ? whiteOccupation() : blackOccupation()) & isolatedAttack) == 0) {
        // No capture and not our own color
        moves[i] = (movePack(from, to, false, false, false, false, 0));
        i++;
      }
      attacks &= ~isolatedAttack;
    }
    bishopsCopy &= ~isolatedBishop;
  }
  return i;
}
