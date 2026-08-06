#include "engine.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <ctime>
#include <math.h>
#include <chrono>
//position fen rnbqkbnr/ppp1pppp/8/3p4/5P2/8/PPPPP1PP/RNBQKBNR w KQkq - 0 1 moves e2e3 d5d4 e3e4 d4d3 f1d3 h7h5 g1f3 h5h4 e1g1 h4h3 g2h3 e7e6 d3b5 c7c6 b5c4 b7b5 c4e2 b5b4 g1g2 f7f5 e4f5 e6f5 e2c4 g8h6 f3g5 g7g6 g5e6 d8d6 e6f8 h8f8 c2c3 b4c3 d2c3 d6d1 f1d1 c8b7 c1e3 a7a5 e3c5 f8f6 d1e1 e8d7 e1e7 d7c8 b1d2 a5a4 a1d1 g6g5 f4g5 f6g6 c4e6 g6e6 e7e6 a8a5 g5h6 a5c5 h6h7 c8d7 h7h8q d7e6 h8b8 b7a6 b8b6 a6e2 b6c5 e2d1 c5c6 e6e5 d2c4 e5f4 c6d6 f4g5 d6d1 a4a3 b2a3 f5f4 d1g4 g5h6 g4f4 h6g6 h3h4 g6h7 h4h5 h7g7 f4f5 g7g8 h5h6 g8h8 f5g4 h8h7 c4e5 h7h8 h2h3 h8h7 h3h4 h7h8 c3c4 h8h7 a3a4 h7h8 a2a3 h8h7 h4h5 h7h8 c4c5 h8h7 a4a5 h7h8 a3a4 h8h7 c5c6 h7h8 a5a6 h8h7 a4a5 h7h8 c6c7 h8h7 a6a7 h7h8 a5a6 h8h7 c7c8n h7h8 a7a8n h8h7 a6a7 h7h8


int Engine::mvvlvaOrder(uint16_t m){
  if (m == 0){
    return 12;
  }
  uint64_t bb = (1ULL << 63) >> (m & 0b0000000000111111);
  uint64_t bb2 = (1ULL << 63) >> ((m >> 6) & 0b0000000000111111);
  int valueA = 6;
  int valueB = 5;
  if (((board->color == WHITE ? board->whitePawns : board->blackPawns) & bb) != 0){
    valueA = 1;
  }
  if (((board->color == WHITE ? board->whiteKnights : board->blackKnights) & bb) != 0){
    valueA = 2;
  }
  if (((board->color == WHITE ? board->whiteBishops : board->blackBishops) & bb) != 0){
    valueA = 3;
  }
  if (((board->color == WHITE ? board->whiteRooks : board->blackRooks) & bb) != 0){
    valueA = 4;
  }
  if (((board->color == WHITE ? board->whiteQueens : board->blackQueens) & bb) != 0){
    valueA = 5;
  }
  if (((board->color == WHITE ? board->whiteKing : board->blackKing) & bb) != 0){
    valueA = 6;
  }

  if (((board->color == BLACK ? board->whitePawns : board->blackPawns) & bb2) != 0){
    valueB = 4;
  }
  if (((board->color == BLACK ? board->whiteKnights : board->blackKnights) & bb2) != 0){
    valueB = 3;
  }
  if (((board->color == BLACK ? board->whiteBishops : board->blackBishops) & bb2) != 0){
    valueB = 2;
  }
  if (((board->color == BLACK ? board->whiteRooks : board->blackRooks) & bb2) != 0){
    valueB = 1;
  }
  if (((board->color == BLACK ? board->whiteQueens : board->blackQueens) & bb2) != 0){
    valueB = 0;
  }
  return valueA + valueB;
}
int Engine::partition(uint16_t arr[], int start, int end)
{
 
    int pivot = arr[start];
    int pivoteValue = mvvlvaOrder(arr[start]);
    int count = 0;
    for (int i = start + 1; i <= end; i++) {
        if (mvvlvaOrder(arr[i]) <= pivoteValue)
            count++;
    }
 
    // Giving pivot element its correct position
    int pivotIndex = start + count;
    std::swap(arr[pivotIndex], arr[start]);
    int i = start, j = end;
    while (i < pivotIndex && j > pivotIndex) {
        while (mvvlvaOrder(arr[i]) <= pivoteValue) {
            i++;
        }
        while (mvvlvaOrder(arr[j]) > pivoteValue) {
            j--;
        }
        if (i < pivotIndex && j > pivotIndex) {
            std::swap(arr[i++], arr[j--]);
        }
    }
    return pivotIndex;
}
 
void Engine::quickSort(uint16_t arr[], int start, int end)
{
 
    if (start >= end)
        return;
    int p = partition(arr, start, end);
    quickSort(arr, start, p - 1);
    quickSort(arr, p + 1, end);
}

int Engine::search(int alpha, int beta, uint64_t attackers, int depthleft, int originalDepth, uint16_t &bestMove, bool &left, int &totalNodes, bool wasNull, uint16_t prevBest, bool isPV, int pvIndex){
  totalNodes ++;
  bool wasCheckExtension = false;
  if (depthleft <= 0) {
    if (!(board->getKing() & attackers))
      return quiesce(alpha, beta, attackers, totalNodes, 0);
    depthleft = 1;
    wasCheckExtension = true;
  }
  if (alpha >= beta){
    return alpha;
  }
  std::chrono::duration<double, std::milli> ms_double = std::chrono::high_resolution_clock::now() - startClock;
  int elapsed = (int)(ms_double.count());
  if ((maxTime != -1 && elapsed >= maxTime - maxTime*0.05)){
    left = true;
    return alpha;
  }
  uint16_t moves[218];
  this->board->resetAttackers();
  uint64_t pawnAttacks, rookAttacks, knightAttacks, bishopAttacks, queenAttacks, kingAttacks;
  int total = this->board->generateMoves(moves, pawnAttacks, rookAttacks, knightAttacks, bishopAttacks, queenAttacks, kingAttacks);
  quickSort(moves, 0, total-1);
  int score = -100000;
  int bestScore = -100001;
  bool noLegal = true;
  for (int i=0;i<total;i++){
    this->board->setAttackers(pawnAttacks, rookAttacks, knightAttacks, bishopAttacks, queenAttacks, kingAttacks);
    uint64_t isolated = this->board->getKing();
    bool check = (attackers & isolated) != 0;
    this->board->makeMove(moves[i]);
    uint64_t a = this->board->getAttackers();
    if (!this->board->isLegal(attackers, check)) {
      this->board->unmakeMove(moves[i]);
      continue;
    }
    if (originalDepth == depthleft && bestMove == 0){
      bestMove = moves[i];
    }
    //----
    noLegal = false;
    uint16_t nbm = 0;
    if (i == 0){
      score = -search(-beta, -alpha, a, depthleft - 1, originalDepth, nbm, left, totalNodes, false, 0, isPV, pvIndex - 1);
    }
    else {
      score = -search(-alpha-1, -alpha, a, depthleft - 1, originalDepth, nbm, left, totalNodes, false, 0, false, pvIndex - 1);
      if (score > alpha && isPV){
        score = -search(-beta, -alpha, a, depthleft - 1, originalDepth, nbm, left, totalNodes, false, 0, true, pvIndex - 1);
      }
    }
    board->unmakeMove(moves[i]);

    if (score > bestScore){
      bestScore = score;
      if (bestScore >= beta){
        return bestScore;
      }
      if (bestScore > alpha){
        alpha = bestScore;
        bestMove = moves[i];
        if (isPV){
          if (pvIndex >= 0 && !wasCheckExtension){
            PV[pvIndex] = moves[i];
            if (depthleft == originalDepth){
              //board->printMove(PV[pvIndex]);
              //std::cout << " " << pvIndex << "\n";
            }
          }
        }
      }
    }
  }
  if (noLegal && (board->getKing() & attackers) != 0){
    return -100000 + (originalDepth - depthleft);
  }
  if (noLegal){
    return 0;
  }
  return bestScore;
}

int Engine::quiesce(int alpha, int beta, uint64_t attackers, int &totalNodes, int depth){
  totalNodes ++;
  int stand_pat = staticEvaluation(attackers);
  std::chrono::duration<double, std::milli> ms_double = std::chrono::high_resolution_clock::now() - startClock;
  int elapsed = (int)(ms_double.count());
  if ((maxTime != -1 && elapsed >= maxTime - maxTime*0.05))
    return alpha;
  if( stand_pat >= beta )
    return beta;
  if( alpha < stand_pat )
    alpha = stand_pat;

  uint16_t moves[218];
  this->board->resetAttackers();
  uint64_t pawnAttacks, rookAttacks, knightAttacks, bishopAttacks, queenAttacks, kingAttacks;
  int total = this->board->generateMoves(moves, pawnAttacks, rookAttacks, knightAttacks, bishopAttacks, queenAttacks,
                                         kingAttacks);

  quickSort(moves, 0, total-1);
  for (int i = 0; i < total; i++) {
    this->board->setAttackers(pawnAttacks, rookAttacks, knightAttacks, bishopAttacks, queenAttacks, kingAttacks);
    uint64_t isolated = this->board->getKing();
    bool check = (attackers & isolated) != 0;
    this->board->makeMove(moves[i]);
    uint64_t a = this->board->getAttackers();
    if (!this->board->isLegal(attackers, check)) {
      this->board->unmakeMove(moves[i]);
      continue;
    }
    uint8_t flag = (0b1111000000000000 & moves[i]) >> 12;
    if (flag != 0b0100 && flag != 0b1100 && flag != 0b1101 &&  flag != 0b1110 && flag != 0b1111 && flag != 0b0101 && !check){
      this->board->unmakeMove(moves[i]);
      continue;
    }
    int score = -quiesce( -beta, -alpha, a, totalNodes, depth + 1);
    board->unmakeMove(moves[i]);

    if (score >= stand_pat){
      stand_pat = score;
      if (stand_pat >= beta){
        return stand_pat;
      }
      if (stand_pat > alpha){
        alpha = stand_pat;
      }
    }
  }
  return alpha;
}

uint16_t Engine::runSearchID(int m, int& score, int& nodeCount){
  uint16_t runningBest = 0;
  startClock = std::chrono::high_resolution_clock::now();
  int totalNodes = 0;
  int actualTotalNodes = 0;
  int prevElapsed = 1;
  for (int i=1;i<10;i++){
    board->resetAttackers();
    uint64_t pawnAttacks, rookAttacks, knightAttacks, bishopAttacks, queenAttacks, kingAttacks;
    uint16_t moves[218];
    board->color = !board->color;
    int total =
        board->generateMoves(moves, pawnAttacks, rookAttacks, knightAttacks, bishopAttacks, queenAttacks, kingAttacks);
    board->color = !board->color;
    
    uint16_t bestMove = 0;
    bool done = false;
    maxTime = m;
    totalNodes = 0;
    // int s = alphaBeta(-100000, 100000, board->getAttackers(), i, i, bestMove, done, totalNodes, false);
    // Use Principle Variation Search
    // ~45 elo gain
    for (int j=0;j<16;j++){
      PV[j] = 0;
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    int s = search(-100000, 100000, board->getAttackers(), i, i, bestMove, done, totalNodes, false, runningBest, true, 15);
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms_double = t2 - t1;
    double elapsed = (double)(ms_double.count()) / 1000.0;
    // If time left on the clock is not enough to research all the nodes we just did then dont even bother
    actualTotalNodes += totalNodes;
    if (i == 1){
      runningBest = bestMove;
      score = s;
    }
    if (done){
      break;
    }
    else {
      runningBest = bestMove;
      score = s;
      std::cout << "info score cp " << s << " nodes " << totalNodes << " nps " << totalNodes / elapsed << "\n";
      // std::cout << "info score cp " << s << " nodes " << totalNodes << " nps " << totalNodes / elapsed << " pv ";
      // for (int j=15;j>=0;j--){
      //   if (PV[j] == 0){
      //     continue;
      //   }
      //   board->printMove(PV[j]);
      //   std::cout << " ";
      // }
      // std::cout << "\n";
    }
    
  }
  //std::cout << "Searched: " << actualTotalNodes << "\n";
  nodeCount = actualTotalNodes;
  return runningBest;
}


int Engine::getEval() {
  board->resetAttackers();
  uint64_t pawnAttacks, rookAttacks, knightAttacks, bishopAttacks, queenAttacks, kingAttacks = 0;
  uint16_t moves[218];
  board->color = !board->color;
  int total =
      board->generateMoves(moves, pawnAttacks, rookAttacks, knightAttacks, bishopAttacks, queenAttacks, kingAttacks);
  board->color = !board->color;
  uint64_t T = board->getAttackers();
  uint16_t bestMove = 0;
  bool done = false;
  int tot = 0;
  maxTime = -1;
  startClock = std::chrono::high_resolution_clock::now();
  //int EV = pvSearch(-100000, 100000, board->getAttackers(), 1, 1, bestMove, done, tot, false);
  return staticEvaluation(T);
}

uint16_t Engine::runSearch(int depth, int m) {
  startClock = std::chrono::high_resolution_clock::now();
  board->resetAttackers();
  uint64_t pawnAttacks, rookAttacks, knightAttacks, bishopAttacks, queenAttacks, kingAttacks;
  uint16_t moves[218];
  board->color = !board->color;
  int total =
      board->generateMoves(moves, pawnAttacks, rookAttacks, knightAttacks, bishopAttacks, queenAttacks, kingAttacks);
  board->color = !board->color;
  
  uint16_t bestMove = 0;
  bool done = false;
  maxTime = m;
  int totalNodes = 0;
  search(-100000, 100000, board->getAttackers(), depth, depth, bestMove, done, totalNodes, false, 0, true, 15);
  return bestMove;
}