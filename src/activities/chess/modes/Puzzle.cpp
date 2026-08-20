#include "Puzzle.h"

#include <ArduinoJson.h>
#include <Game.h>
#include <Logging.h>
#include <WiFi.h>

#if defined(FREEINK_NET_WOLFSSL)
#include <SecureHttpClient.h>
#endif

#include <functional>
#include <sstream>
using namespace std;

#include "../ChessActivity.h"
#include "Move.h"
#include "activities/network/WifiSelectionActivity.h"
#include "network/HttpDownloader.h"

//////////////// PuzzleState ////////////

PuzzleState::PuzzleState(ChessActivity* activity) : ChessState(activity) {}

////////////// PuzzleStartState //////////////

PuzzleStartState::PuzzleStartState(ChessActivity* activity) : PuzzleRightState(activity) {
  activity->game.start("");
  activity->move = Move{};
  activity->last = Move{};
  activity->moves = {};
  activity->statusText = "";
  activity->btnUp = "";
  activity->btnDown = "";

  if (!start()) {
    download([this]() { start(); });
  }
}

bool PuzzleStartState::start() {
  activity->infoText = "Loading puzzle...";
  activity->requestUpdateAndWait();

  string level = activity->level;
  LOG_DBG("CHESS", "Start puzzle %s", level.c_str());

  JsonDocument doc;
  if (!activity->storage.loadPuzzles(level, doc)) {
    LOG_DBG("CHESS", "Could not load puzzles %s", level.c_str());
    activity->infoText = "No puzzles downloaded";
    return false;
  }

  int index = activity->storage.loadPuzzleIndex(level);
  LOG_DBG("CHESS", "Index %i", index);

  if (index >= doc["puzzles"].size()) {
    LOG_DBG("CHESS", "End of batch %s (%i >= %i)", level.c_str(), index, doc["puzzles"].size());
    activity->infoText = "End of batch";
    return false;
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

  return true;
}

void PuzzleStartState::download(function<void()> then) {
  activity->infoText = "Downloading puzzles...";
  activity->startWifi([this, then]() {
    string level = activity->level;
    activity->storage.savePuzzleIndex(level, 0);

    string puzzlesUrl = activity->config.puzzlesUrl + level;

    LOG_DBG("CHESS", "GET %s", puzzlesUrl.c_str());
    string filename = activity->storage.puzzleFilename(level);
    auto result = HttpDownloader::downloadToFile(puzzlesUrl, filename);

    if (result == HttpDownloader::FILE_ERROR) {
      activity->infoText = "Could not write puzzles file";
      return;
    }
    if (result == HttpDownloader::HTTP_ERROR) {
      LOG_ERR("CHESS", "Download failed %i", result);
      activity->infoText = "Download failed: " + getDownloadError(puzzlesUrl);
      return;
    }
    if (result != HttpDownloader::OK) {
      activity->infoText = "Download failed";
      return;
    }

    then();
  });
}

string PuzzleStartState::getDownloadError(string url) {
#if defined(FREEINK_NET_WOLFSSL)
  freeink::SecureHttpClient http;
  http.setUserAgent("CrossPoint-ESP32-" CROSSPOINT_VERSION);
  http.setTimeout(60000);
  http.setInsecure();

  if (!http.begin(url)) return "Bad URL: " + url;
  const int status = http.GET();
  if (http.aborted()) return "Aborted";
  return "Got status " + to_string(status);
#else
  return "Unknown error";
#endif
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