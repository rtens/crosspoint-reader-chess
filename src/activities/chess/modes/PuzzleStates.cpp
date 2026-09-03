#include "PuzzleStates.h"

#include <ArduinoJson.h>
#include <Chess/Parse.h>
#include <Chess/Print.h>
#include <Logging.h>
#include <WiFi.h>

#if defined(FREEINK_NET_WOLFSSL)
#include <SecureHttpClient.h>
#endif

#include <functional>
#include <sstream>
using namespace std;

#include "../ChessActivity.h"
#include "MoveStates.h"
#include "activities/network/WifiSelectionActivity.h"
#include "network/HttpDownloader.h"

//////////////// PuzzleState ////////////

PuzzleState::PuzzleState(ChessActivity* activity) : ChessState(activity) {}

////////////// PuzzleStartState //////////////

PuzzleStartState::PuzzleStartState(ChessActivity* activity) : PuzzleRightState(activity) {
  activity->move = Chess::Move{};
  activity->moves = {};
  activity->statusText = "";
  activity->btnUp = "";
  activity->btnDown = "";
  activity->btnLeft = "";

  if (!start()) {
    download([this]() { start(); });
  }
}

ChessState* PuzzleStartState::left() {
  int index = activity->storage.loadPuzzleIndex(activity->level);
  if (index == 0) return this;

  activity->storage.savePuzzleIndex(activity->level, index - 1);

  delete this;
  return new MoveStartState(activity, new PuzzleStartState(activity));
}

bool PuzzleStartState::start() {
  activity->board.reset();
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
  LOG_DBG("CHESS", "Index %i for %s", index, level.c_str());

  if (level == "daily") {
    return startDaily(doc, index);
  } else {
    return startMix(doc, index);
  }
}

bool PuzzleStartState::startDaily(JsonDocument& doc, int index) {
  LOG_DBG("CHESS", "Starting daily puzzle");

  if (index > 0) {
    LOG_DBG("CHESS", "Already solved (%i)", index);
    activity->infoText = "Already solved";
    return false;
  }

  LOG_DBG("CHESS", "Parsing puzzle");
  string pgn = doc["game"]["pgn"];
  JsonArray jsonSolution = doc["puzzle"]["solution"];
  string id = doc["puzzle"]["id"];

  LOG_DBG("CHESS", "Parsing solution");
  vector<Chess::Move> solution;
  for (const char* step : jsonSolution) {
    solution.push_back(Chess::Parse::move(string(step)));
  }

  doc.clear();
  startPuzzle(pgn, solution);

  stringstream infos;
  infos << "Today's Puzzle (" << id << ")";
  activity->infoText = infos.str();
  LOG_DBG("CHESS", "Done starting daily puzzle");

  return true;
}

bool PuzzleStartState::startMix(JsonDocument& doc, int index) {
  LOG_DBG("CHESS", "tarting puzzle mix");

  if (index >= doc["puzzles"].size()) {
    LOG_DBG("CHESS", "End of batch (%i >= %i)", index, doc["puzzles"].size());
    activity->infoText = "End of batch";
    return false;
  }

  if (index > 0) activity->btnLeft = "Prev";

  LOG_DBG("CHESS", "Parsing puzzle");
  JsonDocument jsonPuzzle = doc["puzzles"][index];
  string pgn = jsonPuzzle["game"]["pgn"];
  JsonArray jsonSolution = jsonPuzzle["puzzle"]["solution"];
  string id = jsonPuzzle["puzzle"]["id"];
  int total = doc["puzzles"].size();

  LOG_DBG("CHESS", "Parsing solution");
  vector<Chess::Move> solution;
  for (const char* step : jsonSolution) {
    solution.push_back(Chess::Parse::move(string(step)));
  }

  doc.clear();
  startPuzzle(pgn, solution);

  stringstream infos;
  infos << "Puzzle " << (index + 1) << "/" << total << " (" << id << ")";
  activity->infoText = infos.str();
  LOG_DBG("CHESS", "Done starting puzzle mix");

  return true;
}

void PuzzleStartState::startPuzzle(const string pgn, vector<Chess::Move> solution) {
  LOG_DBG("CHESS", "Start puzzle %s", pgn.c_str());
  Chess::Parse::fen(Chess::StartingPosition, activity->board);
  Chess::Parse::pgn(pgn, activity->board);
  if (activity->puzzle) delete activity->puzzle;
  activity->puzzle = new Chess::Puzzle(&activity->board, solution);
  activity->pov = activity->board.turn;
  LOG_DBG("CHESS", "Puzzle started");
}

void PuzzleStartState::download(function<void()> then) {
  activity->infoText = "Downloading puzzles...";
  activity->startWifi([this, then]() {
    string level = activity->level;
    activity->storage.savePuzzleIndex(level, 0);

    string puzzlesUrl = activity->config.puzzleUrls[level];

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

PuzzleRightState::PuzzleRightState(ChessActivity* activity) : PuzzleState(activity) {
  activity->btnLeft = "Reset";
  activity->btnRight = "Hint";
  activity->infoText = "Go on";
}

ChessState* PuzzleRightState::left() {
  delete this;
  return new MoveStartState(activity, new PuzzleStartState(activity));
}

ChessState* PuzzleRightState::right() {
  delete this;
  return new MoveFromState(activity, new PuzzleHintState(activity));
}

ChessState* PuzzleRightState::move(Chess::Move move) {
  activity->puzzle->propose(move);

  if (activity->puzzle->state == Chess::Puzzle::Correct) {
    activity->requestUpdateAndWait();
    activity->puzzle->respond();
    activity->movesSinceRefresh += 2;

    delete this;
    return new MoveStartState(activity, new PuzzleRightState(activity));

  } else if (activity->puzzle->state >= Chess::Puzzle::Solved) {
    int index = activity->storage.loadPuzzleIndex(activity->level);
    activity->storage.savePuzzleIndex(activity->level, index + 1);

    delete this;
    return new PuzzleSolvedState(activity);

  } else {
    delete this;
    return new PuzzleWrongState(activity);
  }
}

////////////// PuzzleHintState ////////////////

PuzzleHintState::PuzzleHintState(ChessActivity* activity) : PuzzleRightState(activity) {
  activity->btnRight = "Show";
  activity->infoText = "Try moving this piece";

  Chess::Move hint = activity->puzzle->hint();
  activity->move = Chess::Move{hint.from};
}

ChessState* PuzzleHintState::right() {
  delete this;
  return new PuzzleShowState(activity);
}

////////////// PuzzleShowState ////////////////

PuzzleShowState::PuzzleShowState(ChessActivity* activity) : PuzzleRightState(activity) {
  activity->move = activity->puzzle->hint();

  activity->btnUp = "";
  activity->btnDown = "";
  activity->btnRight = "Make";
  activity->infoText = "This is the best move";
}

ChessState* PuzzleShowState::right() {
  Chess::Move move = activity->move;
  activity->move = Chess::Move{};

  return this->move(move);
}

////////////// PuzzleWrongState ///////////////

PuzzleWrongState::PuzzleWrongState(ChessActivity* activity) : PuzzleState(activity) {
  activity->infoText = "Not quite";
  activity->btnUp = "Undo";
  activity->btnLeft = "Undo";
  activity->btnDown = "";
  activity->btnRight = "";
}

ChessState* PuzzleWrongState::up() {
  activity->move = activity->board.last;
  activity->puzzle->undo();

  delete this;
  return new MoveFromState(activity, new PuzzleCorrectionState(activity));
}

ChessState* PuzzleWrongState::left() { return up(); }

///////////// PuzzleCorrectionState ///////////////

PuzzleCorrectionState::PuzzleCorrectionState(ChessActivity* activity) : PuzzleRightState(activity) {
  activity->infoText = "Try again";
}

///////////// PuzzleSolvedState ///////////////

PuzzleSolvedState::PuzzleSolvedState(ChessActivity* activity) : PuzzleState(activity) {
  activity->infoText = "Good job!";
  activity->btnUp = "Again";
  activity->btnDown = "Next Puzzle";
  activity->btnRight = "Again";
  activity->btnRight = "Next";

  if (activity->game.result() == Chess::Game::Checkmate) {
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
  activity->movesSinceRefresh += 50;
  delete this;
  return new MoveStartState(activity, new PuzzleStartState(activity));
}

ChessState* PuzzleSolvedState::left() { return up(); }

ChessState* PuzzleSolvedState::right() { return down(); }