#include "board.h"
#include <iostream>

void Board::resetMoveGen(){
    int castleCnt = 4;
    int pieceType = 0; // Pawn
    uint64_t runningPieces = 0ULL;
    uint64_t pawnTable = 0ULL;
    uint64_t knightTable = 0ULL;
    uint64_t bishopTable = 0ULL;
    uint64_t rookTable = 0ULL;
    uint64_t queenTable = 0ULL;
    uint64_t kingTable = 0ULL;

    //-
}
// uint16_t Board::getMove(int &pieceType, int &attackingPieceType, uint64_t& runningPieces, uint64_t& pawnTables, uint64_t& knightTables, uint64_t& bishopTables, uint64_t& rookTables, uint64_t& queenTables, uint64_t& kingTables){
//     if (pieceType == 0){
//         // Generate Pawn Move;
//         uint64_t isolatedPawn = pawnTables & ((~pawnTables) + 1);
//         int from = 63 - __builtin_ctzll(isolatedPawn);
//         // Check capture and splice from running
//         // Left diagonal
//         if (color == 0) {
//             if (from % 8 != 0)
//                 if ((*bitboards[attackingPieceType + 6] & (isolatedPawn << 9) ) != 0){
//                     runningPieces &= ~(isolatedPawn << 9);
//                     return 
//                 }
//             if ((from + 1) % 8 != 0)
//                 movements |= isolatedPawn << 7;
//         } else {
//             if ((from + 1) % 8 != 0)
//                 movements |= isolatedPawn >> 9;
//             if (from % 8 != 0)
//                 movements |= isolatedPawn >> 7;
//         }
//     }

//     }
// }

uint64_t Board::generatePawnAttacks(uint64_t &isolatedPawn){
    // Diagonal captures
    int from = 63 - __builtin_ctzll(isolatedPawn);
    uint64_t movements = 0;
    // Attacks/Captures
    // Check color and generate diagonal attack square
    if (color == 0) {
      if (from % 8 != 0){
        movements |= isolatedPawn << 9;
        movements |= isolatedPawn << 1;
      }
      if ((from + 1) % 8 != 0){
        movements |= isolatedPawn << 7;
        movements |= isolatedPawn >> 1;
      }
    } else {
      if ((from + 1) % 8 != 0){
        movements |= isolatedPawn >> 9;
        movements |= isolatedPawn >> 1;
      }
      if (from % 8 != 0){
        movements |= isolatedPawn >> 7;
        movements |= isolatedPawn << 1;
      }
    }
    return movements;
}
uint64_t Board::generatePawnPushes(uint64_t &isolatedPawn){
    // Diagonal captures
    int from = 63 - __builtin_ctzll(isolatedPawn);
    uint64_t movements = 0;
    // Attacks/Captures
    // Check color and generate diagonal attack square
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
    return movements;
}
uint16_t Board::standardSMG(uint64_t &isolatedPawn, uint64_t &attackSet, bool caps){
    uint64_t isolatedAttack = attackSet & ((~attackSet) + 1);
    int from = 63 - __builtin_ctzll(isolatedPawn);
    int to = 63 - __builtin_ctzll(isolatedAttack);
    attackSet &= ~isolatedAttack;
    //printBitBoard(isolatedAttack);
    if ((isolatedAttack & (color == WHITE ? blackOccupation() : whiteOccupation())) != 0)
      return movePack(from, to, false, false, 0, true, 0);
    else if ( (isolatedAttack & (color == WHITE ? whiteOccupation() : blackOccupation())) == 0)
      return movePack(from, to, false, false, 0, false, 0);
    return 0;
}
uint16_t Board::pawnSMG(uint64_t &isolatedPawn, uint64_t &attackSet, bool caps){
    uint64_t isolatedAttack = attackSet & ((~attackSet) + 1);
    int from = 63 - __builtin_ctzll(isolatedPawn);
    int to = 63 - __builtin_ctzll(isolatedAttack);
    attackSet &= ~isolatedAttack;
    if (caps){
      // En Passant
      if (color == 0) {
          if (depth > 0 && (previousMover[depth - 1] == 6)) {
            if ((from >= 24 && from < 32) && ((previousMoves[depth - 1] & 0b1111000000000000) >> 12) == 1 &&
                ((previousMoves[depth - 1] & 0b0000111111000000) >> 6) == to + 8) {
              return movePack(from, to, false, true, false, false, 0);
            }
          }
        } else {
          if (depth > 0 && previousMover[depth - 1] == 0) {

            if ((from >= 32 && from < 40) && ((previousMoves[depth - 1] & 0b1111000000000000) >> 12) == 1 &&
                ((previousMoves[depth - 1] & 0b0000111111000000) >> 6) == to - 8) {
              return movePack(from, to, false, true, false, false, 0);
          }
        }
      }
      if (color == WHITE && (to >= A8 && to <= H8)){
        // Always promo to queen (bad fix later)
        return movePack(from, to, false, false, 4, true, 0);
      }
      if (color == BLACK && (to >= A1 && to <= H1)){
        // Always promo to queen (bad fix later)
        return movePack(from, to, false, false, 4, true, 0);
      }
      return movePack(from, to, false, false, 0, true, 0);
    }
    else {
      if (color == WHITE && (to >= A8 && to <= H8)){
        // Always promo to queen (bad fix later)
        return movePack(from, to, false, false, 4, false, 0);
      }
      if (color == BLACK && (to >= A1 && to <= H1)){
        // Always promo to queen (bad fix later)
        return movePack(from, to, false, false, 4, false, 0);
      }
      return movePack(from, to, std::abs(from - to) > 9, false, 0, false, 0);
    }
    
}