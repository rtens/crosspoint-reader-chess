#include "../board.h"

uint16_t Board::pawnMoves(bool color, uint16_t* moves, int i) {
  // Captures
  uint64_t pawnsCopy = color == 0 ? whitePawns : blackPawns;
  while (pawnsCopy != 0) {
    uint64_t isolatedPawn = pawnsCopy & ((~pawnsCopy) + 1);
    int from = 63 - __builtin_ctzll(isolatedPawn);
    uint64_t movements = 0;
    // Attacks/Captures
    // Check color and generate diagonal attack square
    if (color == 0) {
      if (from % 8 != 0)
        movements |= isolatedPawn << 9;
      if ((from + 1) % 8 != 0)
        movements |= isolatedPawn << 7;
    } else {
      if ((from + 1) % 8 != 0)
        movements |= isolatedPawn >> 9;
      if (from % 8 != 0)
        movements |= isolatedPawn >> 7;
    }
    // Iterate over attack squares and see if the move is valid (capturable piece)
    pawnAttackers |= movements;
    while (movements != 0) {
      uint64_t isolatedAttack = movements & ((~movements) + 1);
      int to = 63 - __builtin_ctzll(isolatedAttack);
      if (((color == WHITE ? blackOccupation() : whiteOccupation()) & isolatedAttack) != 0) {
        if (color == WHITE && (to >= A8 && to <= H8)) {
          // Promo
          moves[i] = movePack(from, to, false, false, 1, true, 0);
          i++;
          moves[i] = movePack(from, to, false, false, 2, true, 0);
          i++;
          moves[i] = movePack(from, to, false, false, 3, true, 0);
          i++;
          moves[i] = movePack(from, to, false, false, 4, true, 0);
        } else if (color == BLACK && (to >= A1 && to <= H1)) {
          moves[i] = movePack(from, to, false, false, 1, true, 0);
          i++;
          moves[i] = movePack(from, to, false, false, 2, true, 0);
          i++;
          moves[i] = movePack(from, to, false, false, 3, true, 0);
          i++;
          moves[i] = movePack(from, to, false, false, 4, true, 0);
        } else {
          moves[i] = movePack(from, to, false, false, 0, true, 0);
        }
        i++;
      }
      // En Passant moves
      if (color == 0) {
        if (depth > 0 && (previousMover[depth - 1] == 6)) {
          if ((from >= 24 && from < 32) && ((previousMoves[depth - 1] & 0b1111000000000000) >> 12) == 1 &&
              ((previousMoves[depth - 1] & 0b0000111111000000) >> 6) == to + 8) {
            moves[i] = movePack(from, to, false, true, false, false, 0);
            i++;
          }
        }
      } else {
        if (depth > 0 && previousMover[depth - 1] == 0) {

          if ((from >= 32 && from < 40) && ((previousMoves[depth - 1] & 0b1111000000000000) >> 12) == 1 &&
              ((previousMoves[depth - 1] & 0b0000111111000000) >> 6) == to - 8) {
            moves[i] = movePack(from, to, false, true, false, false, 0);
            i++;
          }
        }
      }
      movements &= ~isolatedAttack;
    }
    // Pushes
    movements = 0;
    if (color == 0) {
      // Make sure nothing is in front of the white pawn
      if ((combinedOccupation() & (isolatedPawn << 8)) == 0) {
        movements |= (isolatedPawn << 8);
        // Make sure nothing is in the way of double push
        if (from >= 48 && from <= 55 && (combinedOccupation() & (isolatedPawn << 16)) == 0) {
          movements |= (isolatedPawn << 16);
        }
      }
    } else {
      if ((combinedOccupation() & (isolatedPawn >> 8)) == 0) {
        movements |= (isolatedPawn >> 8);
        // Make sure nothing is in the way of double push
        if (from >= 8 && from <= 15 && (combinedOccupation() & (isolatedPawn >> 16)) == 0) {
          movements |= (isolatedPawn >> 16);
        }
      }
    }

    // Push the moves
    while (movements != 0) {
      uint64_t isolatedPush = movements & ((~movements) + 1);
      int to = 63 - __builtin_ctzll(isolatedPush);
      if (color == 0) {
        if ((to >= 0 && to <= 7)) {
          moves[i] = movePack(from, to, false, false, 1, false, 0);
          i++;
          moves[i] = movePack(from, to, false, false, 2, false, 0);
          i++;
          moves[i] = movePack(from, to, false, false, 3, false, 0);
          i++;
          moves[i] = movePack(from, to, false, false, 4, false, 0);
        } else {
          moves[i] = movePack(from, to, from - to > 9, false, false, false, 0);
        }
      } else {
        if ((to >= 56 && to <= 63)) {
          moves[i] = movePack(from, to, false, false, 1, false, 0);
          i++;
          moves[i] = movePack(from, to, false, false, 2, false, 0);
          i++;
          moves[i] = movePack(from, to, false, false, 3, false, 0);
          i++;
          moves[i] = movePack(from, to, false, false, 4, false, 0);
        } else {
          moves[i] = movePack(from, to, to - from > 9, false, false, false, 0);
        }
      }
      i++;
      movements &= ~isolatedPush;
    }

    pawnsCopy &= ~isolatedPawn;
  }
  return i;
}
