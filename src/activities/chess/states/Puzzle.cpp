#include "Puzzle.h"

#include <ArduinoJson.h>
#include <Game.h>
#include <Logging.h>
#include <WiFi.h>

#include <sstream>
using namespace std;

#include "../ChessActivity.h"
#include "Move.h"
#include "activities/network/WifiSelectionActivity.h"
#include "network/HttpDownloader.h"

//////////////// PuzzleState ////////////

PuzzleState::PuzzleState(ChessActivity* activity) : ChessState(activity) {}

////////////// PuzzleStartState //////////////

PuzzleStartState::PuzzleStartState(ChessActivity* activity) : PuzzleRightState(activity) { start(); }

void PuzzleStartState::start(bool downloadIfNeeded) {
  activity->infoText = "Loading puzzle...";
  activity->requestUpdateAndWait();

  string level = activity->level;
  LOG_DBG("CHESS", "Start puzzle %s downloadIfNeeded %i", level.c_str(), downloadIfNeeded);

  JsonDocument doc;
  if (!activity->storage.loadPuzzles(level, doc)) {
    LOG_DBG("CHESS", "Could not load puzzles %s", level.c_str());
    if (downloadIfNeeded) download();
    return;
  }

  int index = activity->storage.loadPuzzleIndex(level);
  LOG_DBG("CHESS", "Index %i", index);

  if (index >= doc["puzzles"].size()) {
    LOG_DBG("CHESS", "End of batch %s (%i >= %i)", level.c_str(), index, doc["puzzles"].size());
    if (downloadIfNeeded) download();
    return;
  }

  LOG_DBG("CHESS", "Parsing puzzzle");
  JsonObject jsonPuzzle = doc["puzzles"][index];
  string pgn = jsonPuzzle["game"]["pgn"];
  JsonArray jsonSolution = jsonPuzzle["puzzle"]["solution"];

  vector<string> solution;
  for (const char* step : jsonSolution) {
    solution.push_back(string(step));
  }

  LOG_DBG("CHESS", "Start puzzle %s", pgn.c_str());
  activity->puzzle.start(pgn, solution);
  activity->pov = activity->game.turn;
  LOG_DBG("CHESS", "Puzzle started");

  activity->move = Move{};
  activity->last = Move{};
  activity->btnLeft = "";
  activity->btnRight = "";

  stringstream infos;
  infos << "Puzzle " << (index + 1) << "/" << doc["puzzles"].size();
  activity->infoText = infos.str();
  LOG_DBG("CHESS", "Done starting puzzle");
}

void PuzzleStartState::download() {
  activity->startWifi([this]() {
    string level = activity->level;
    activity->storage.savePuzzleIndex(level, 0);

    string puzzlesUrl = activity->config.puzzlesUrl + level;

    LOG_DBG("CHESS", "GET %s", puzzlesUrl.c_str());
    string filename = activity->storage.puzzleFilename(level);
    auto result = HttpDownloader::downloadToFile(puzzlesUrl, filename, nullptr);

    if (result != HttpDownloader::OK) {
      LOG_ERR("CHESS", "Download failed %i", result);
      activity->infoText = "Could not download puzzles";
      return;
    }

    start(false);
  });
}

//////////////// PuzzleRightState //////////////

PuzzleRightState::PuzzleRightState(ChessActivity* activity) : PuzzleState(activity) { activity->infoText = "Go on"; }

ChessState* PuzzleRightState::move(Move move) {
  int result = activity->puzzle.propose(move);
  activity->last = activity->puzzle.last;

  if (result == Puzzle::RIGHT) {
    delete this;
    return new MoveStartState(activity, new PuzzleRightState(activity));

  } else if (result == Puzzle::SOLVED) {
    int index = activity->storage.loadPuzzleIndex(activity->level);
    activity->storage.savePuzzleIndex(activity->level, index + 1);
    delete this;
    return new PuzzleSolvedState(activity);

  } else {
    delete this;
    return new PuzzleWrongState(activity);
  }
}

////////////// PuzzleWrongState ///////////////

PuzzleWrongState::PuzzleWrongState(ChessActivity* activity) : PuzzleState(activity) {
  activity->infoText = "Not quite";
  activity->btnUp = "Undo";
  activity->btnDown = "";
}

ChessState* PuzzleWrongState::up() {
  activity->puzzle.undo();
  activity->move = activity->last;
  activity->last = Move{};
  return new MoveFromState(activity, new PuzzleCorrectionState(activity));
}

///////////// PuzzleCorrectionState ///////////////

PuzzleCorrectionState::PuzzleCorrectionState(ChessActivity* activity) : PuzzleRightState(activity) {
  activity->infoText = "Try again";
}

///////////// PuzzleSolvedState ///////////////

PuzzleSolvedState::PuzzleSolvedState(ChessActivity* activity) : PuzzleState(activity) {
  activity->infoText = "Good job!";
  activity->btnUp = "Again";
  activity->btnDown = "Next Puzzle";

  if (activity->game.isOver() == Game::CHECKMATE) {
    activity->statusText = "CHECKMATE!";
  }
}

ChessState* PuzzleSolvedState::up() {
  int index = activity->storage.loadPuzzleIndex(activity->level);
  activity->storage.savePuzzleIndex(activity->level, index - 1);

  delete this;
  return new MoveStartState(activity, new PuzzleStartState(activity));
}

ChessState* PuzzleSolvedState::down() {
  delete this;
  return new MoveStartState(activity, new PuzzleStartState(activity));
}