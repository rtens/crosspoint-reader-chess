#pragma once

#include <Game.h>
#include <Puzzle.h>

#include <cstdint>

#include "../Activity.h"

using namespace std;

enum Mode { OTB, PUZZLES, COMPUTER };

enum State { SELECT_PIECE, SELECT_MOVE, WAIT, GAME_OVER, IDLE };

class ChessActivity final : public Activity {
 private:
  Game game;
  Puzzle puzzle;

  GfxRenderer::Orientation savedOrientation = GfxRenderer::Orientation::Portrait;

  string header = "Chess";
  string info = "";
  string btnL = "";
  string btnR = "";

  Mode mode = PUZZLES;
  int level = 0;

  State state = SELECT_PIECE;
  int puzzleState = Puzzle::RIGHT;

  int selected = 0;
  int selected_piece = 0;

  vector<int> pieces;
  vector<int> mine;
  vector<Move> moves;
  int over;

  Move last = Move{};

  void make(Move move);
  void copyPieces();

  void savePosition();
  string loadPosition();

  void loadMode();
  void saveMode();

  int loadPuzzleIndex();
  void savePuzzleIndex(int index);
  void startPuzzle();
  // void loadPuzzle(bool download = true);
  void downloadPuzzles(string filename);

 public:
  explicit ChessActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Chess", renderer, mappedInput), game(), puzzle(&game) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;
};