#include "engine.h"

Engine::Engine() {
  this->board = NULL;
};

Engine::Engine(Board* board) {
  this->board = board;
}
