using namespace std;
#include "Game.h"

#include <sstream>
#include <string>
#include <vector>

void Game::make(Move move) {
  turn ^= 24;
  pieces[move.to] = pieces[move.from];
  pieces[move.from] = Game::EMPTY;
}

vector<Move> Game::moves(int square) {
  switch (pieces[square]) {
    case Game::WHITE | Game::PAWN:
      if (square / 8 == 6) {
        return vector<Move>{Move{square, square - 8}, Move{square, square - 16}};
      } else {
        return vector<Move>{Move{square, square - 8}};
      }
      break;
    default:
      return vector<Move>{};
  }
}

string Game::print(Move move) {
  stringstream ss;

  ss << char((move.from % 8) + 'a') << 8 - (move.from / 8);
  ss << char((move.to % 8) + 'a') << 8 - (move.to / 8);

  return ss.str();
}

string Game::fen() {
  stringstream ss;

  int empties = 0;
  for (int i = 0; i < 64; i++) {
    if (i > 0 && i % 8 == 0) {
      if (empties > 0) {
        ss << empties;
        empties = 0;
      }
      ss << "/";
    }

    if (pieces[i] == Game::EMPTY) {
      empties++;
      continue;
    } else if (empties > 0) {
      ss << to_string(empties);
      empties = 0;
    }

    switch (pieces[i]) {
      case Game::WHITE | Game::PAWN:
        ss << "P";
        break;
    }
  }

  if (empties > 0) {
    ss << empties;
    empties = 0;
  }

  ss << " ";

  if (turn == Game::WHITE) {
    ss << "w";
  } else {
    ss << "b";
  }

  ss << " - - 0 1";
  return ss.str();
}

void Game::restore(string fen) {
  const int p_pieces = 0;
  const int p_turn = 1;
  const int p_castle = 2;
  const int p_passant = 3;
  int part = p_pieces;

  int row = 0;
  int col = 0;

  for (int i = 0; i < fen.length(); i++) {
    if (fen[i] == ' ') {
      part++;
      continue;
    }

    switch (part) {
      case p_pieces:
        switch (fen[i]) {
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
            for (; col < fen[i] - '0'; col++) {
              pieces[row * 8 + col] = Game::EMPTY;
            }
            break;
          case 'P':
            pieces[row * 8 + col] = Game::WHITE | Game::PAWN;
            col++;
            break;
          case '/':
            row++;
            col = 0;
            break;
        }
        break;

      case p_turn:
        switch (fen[i]) {
          case 'w':
            turn = Game::WHITE;
            break;
          case 'b':
            turn = Game::BLACK;
            break;
        }
    }
  }
}