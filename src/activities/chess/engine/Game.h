#pragma once
using namespace std;

#include <string>
#include <vector>

struct Move {
  int from = -1;
  int to = -1;
};

class Game {
 public:
  static const int EMPTY = 0;

  static const int PAWN = 1;
  static const int BISHOP = 2;
  static const int KNIGHT = 3;
  static const int ROOK = 4;
  static const int KING = 5;
  static const int QUEEN = 6;

  static const int WHITE = 8;
  static const int BLACK = 16;

  static string print(Move move);

  int turn = WHITE;
  int pieces[64] = {};

  string fen();
  void restore(string fen);
  void make(Move move);
  vector<Move> moves(int square);
};