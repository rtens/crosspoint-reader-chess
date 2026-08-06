#pragma once

#include "../board/board.h"
#include <ctime>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include <string>
#include <chrono>

#define USE_PST

#define PAWN_MATERIAL_VALUE 100
#define KNIGHT_MATERIAL_VALUE 325
#define BISHOP_MATERIAL_VALUE 350
#define ROOK_MATERIAL_VALUE 500
#define QUEEN_MATERIAL_VALUE 900

struct S{
  int8_t mg;
  int8_t eg;

  S(int8_t a, int8_t b){
    mg = a;
    eg = b;
  };
};

struct S32{
  int mg;
  int eg;

  S32(int a, int b){
    mg = a;
    eg = b;
  };
};

class Engine {
  public:
    Engine();
    explicit Engine(Board* board);

    uint64_t runPerft(int depth);

    Board* board = nullptr;
    int staticEvaluation(uint64_t& attackers);
    
    int quiesce(int alpha, int beta, uint64_t attackers, int &totalNodes, int depth);
    int search(int alpha, int beta, uint64_t attackers, int depthleft, int originalDepth, uint16_t &bestMove, bool &left, int &totalNodes, bool wasNull, uint16_t prevBest, bool isPV, int pvIndex);

    bool badCapture(uint16_t move);
    uint16_t runSearch(int depth, int maxTime);
    uint16_t runSearchID(int maxTime, int& score, int& nodes);
    int getEval();
    int32_t packScore(int16_t mg, int16_t eg);
    int16_t unpack_mg(int32_t packed);
    int16_t unpack_eg(int32_t packed);

    int pieceSquareTables(int &mgScore, int &egScore);
    int pstScores(uint64_t a, int& scoreA, int& scoreB, int i);
    int passedPawn(uint64_t bb, uint64_t bb2, int& mgScore, int& egScore);

    int8_t pawnPST(int loc);
    int8_t egPawnPST(int loc);

    int8_t rookPST(int loc);
    int8_t egRookPST(int loc);

    int8_t knightPST(int loc);
    int8_t egKnightPST(int loc);

    int8_t bishopPST(int loc);
    int8_t egBishopPST(int loc);

    int8_t queenPST(int loc);
    int8_t egQueenPST(int loc);

    int8_t kingPST(int loc);
    int8_t egKingPST(int loc);

    std::chrono::high_resolution_clock::time_point startClock;
    int maxTime = -1;
    int phaseEG = 0;
    int phaseMG = 0;
    uint16_t PV[16] = {};

    int partition(uint16_t arr[], int start, int end);
    void quickSort(uint16_t arr[], int start, int end);
    int mvvlvaOrder(uint16_t m);

    S rookMobility[15] = {S(1, 1), S(1, 1), S(-39, -19), S(-26, 3), S(-16, 11), S(-13, 18), S(-11, 21), S(-6, 28), S(-2, 30), S(3, 33), S(8, 39), S(13, 41), S(13, 46), S(18, 49), S(17, 52)};
    S bishopMobility[14] = {S(1, 1), S(-50, -62), S(-39, -54), S(-28, -21), S(-17, -9), S(-11, 0), S(0, 16), S(8, 22), S(12, 34), S(13, 38), S(15, 45), S(16, 41), S(13, 42), S(37, 33)};
    S queenMobility[28] = {S(1, 1), S(1, 1), S(1, 1), S(-72, -16), S(-58, -97), S(-28, -55), S(-27, -42), S(-28, 19), S(-20, 19), S(-17, 36), S(-11, 38), S(-7, 49), S(-3, 60), S(0, 64), S(2, 68), S(2, 76), S(1, 84), S(2, 90), S(2, 97), S(2, 101), S(2, 110), S(5, 109), S(5, 111), S(13, 106), S(14, 105), S(24, 104), S(64, 84), S(109, 67)};

    S kingAttackers[3] =  {S(-21, 6), S(-26, 8), S(-21, -3)};

    // PSTS
    S pawn_pst[32] = {S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 126), S(39, 116), S(62, 93), S(78, 55), S(-11, 37), S(21, 47), S(49, 3), S(40, -4), S(-21, -40), S(4, -42), S(6, -60), S(13, -71), S(-32, -65), S(-5, -59), S(-2, -73), S(7, -79), S(-26, -71), S(-1, -61), S(-5, -70), S(3, -67), S(-29, -70), S(4, -60), S(0, -64), S(-4, -55), S(0, 0), S(0, 0), S(0, 0), S(0, 0)};
    S knight_pst[32] = {S(-126, -80), S(-122, -1), S(-60, -9), S(-17, 2), S(-13, -28), S(-4, 0), S(42, 5), S(32, 11), S(-3, -3), S(34, 7), S(67, 30), S(42, 41), S(23, 2), S(22, 27), S(52, 39), S(50, 42), S(-12, 4), S(13, 23), S(21, 37), S(23, 43), S(-26, -2), S(-4, 10), S(5, 21), S(29, 38), S(-27, 6), S(-1, 22), S(7, 19), S(1, 18), S(-32, -11), S(-18, 2), S(4, 25), S(10, 28)};
    S bishop_pst[32] = {S(-41, 10), S(-60, 16), S(-70, 13), S(-99, 24), S(-32, 0), S(-22, 2), S(-24, -1), S(-27, 0), S(-6, 17), S(5, 3), S(5, 2), S(-8, -2), S(-21, 11), S(-12, 6), S(-10, 5), S(-3, 11), S(-9, 0), S(-23, 8), S(-21, 7), S(-4, 7), S(-5, 5), S(-11, 3), S(-8, 4), S(-10, 7), S(-3, 5), S(6, 0), S(2, -12), S(-8, 0), S(0, -4), S(8, 3), S(-3, 3), S(-11, 2)};
    S rook_pst[32] = {S(13, 35), S(-10, 42), S(-14, 50), S(-24, 50), S(-1, 33), S(-14, 44), S(-2, 48), S(0, 45), S(-3, 27), S(20, 28), S(2, 31), S(6, 27), S(-10, 26), S(-5, 25), S(-11, 30), S(-11, 27), S(-28, 20), S(-29, 23), S(-36, 28), S(-22, 24), S(-26, 10), S(-15, 6), S(-27, 13), S(-22, 16), S(-33, 9), S(-19, 9), S(-16, 11), S(-15, 12), S(-12, 14), S(-6, 15), S(-1, 18), S(2, 14)};
    S queen_pst[32] = {S(-2, 41), S(-2, 44), S(-15, 81), S(3, 66), S(1, 52), S(-36, 72), S(-28, 95), S(-52, 126), S(15, 39), S(18, 40), S(-9, 87), S(-20, 95), S(-2, 54), S(-10, 69), S(-20, 74), S(-31, 89), S(-6, 51), S(-10, 60), S(-19, 61), S(-16, 68), S(-4, 30), S(-4, 37), S(-7, 48), S(-11, 47), S(11, -2), S(3, 2), S(8, 6), S(3, 30), S(1, 10), S(2, 6), S(1, 12), S(11, 21)};
    S king_pst[32] = {S(120, -113), S(56, -32), S(57, -21), S(-44, 4), S(-40, 6), S(14, 36), S(-27, 49), S(33, 32), S(-59, 23), S(24, 46), S(1, 56), S(-40, 64), S(-99, 21), S(-74, 49), S(-85, 63), S(-121, 72), S(-113, 14), S(-82, 32), S(-87, 49), S(-126, 65), S(-47, -2), S(-18, 10), S(-61, 30), S(-70, 42), S(42, -29), S(31, -10), S(-10, 8), S(-32, 17), S(44, -75), S(55, -45), S(16, -27), S(8, -32)};


  private:
    int qCheck = 0;
    S32 material[6] = {S32(81, 197), S32(290, 365), S32(314, 364), S32(403, 630), S32(852, 1154), S32(0, 0)};
    int gamephaseInc[12] = {0, 1, 1, 2, 4, 0};
    
    // int mg_value[6] = {85, 389, 409, 527, 1137, 0};
    // int eg_value[6] = {103, 250, 281, 476, 880, 0};

    
    uint64_t perft(int depth, uint64_t attackers, int originalDepth);
    int hammingWeight(uint64_t a);
    int getDistanceBB(uint64_t bb, int sq, int scale);
    int doubledPawn(int& mg, int& eg, bool color);
};
