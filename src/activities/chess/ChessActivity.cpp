#include "ChessActivity.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <GfxRenderer.h>
#include <I18n.h>
#include <Logging.h>
#include <WiFi.h>
#include <ctype.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>

#include "activities/network/WifiSelectionActivity.h"
#include "components/UITheme.h"
#include "fontIds.h"
#include "network/HttpDownloader.h"

using namespace std;

void ChessActivity::onEnter() {
  Activity::onEnter();

  loadMode();
  startPuzzle();

  savedOrientation = renderer.getOrientation();
  renderer.setOrientation(GfxRenderer::Orientation::Portrait);
  requestUpdate();
}

string puzzleDifficulty(int level) {
  switch (level) {
    default:
      return "any";
  }
}

void ChessActivity::onExit() {
  Activity::onExit();
  renderer.setOrientation(savedOrientation);
}

void ChessActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    onGoHome();
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    // Open mode selection activity
  }

  if (mode == PUZZLES) {
    if (mappedInput.wasReleased(MappedInputManager::Button::Left)) {
      if (puzzleState == Puzzle::WRONG) {
        puzzle.undo();
        copyPieces();
        info = "Try again";
        puzzleState = Puzzle::RIGHT;
        state = SELECT_PIECE;
        selected = selected_piece;
      } else if (puzzleState == Puzzle::SOLVED) {
        startPuzzle();
      }
      last = Move{};
      requestUpdate();
    }

    if (mappedInput.wasReleased(MappedInputManager::Button::Right)) {
      int index = loadPuzzleIndex();
      savePuzzleIndex(index + 1);
      startPuzzle();
      requestUpdate();
    }
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Down)) {
    selected++;
    requestUpdate();
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Up)) {
    if (state == SELECT_PIECE) {
      if (!mine.size()) return;

      selected %= mine.size();
      selected_piece = selected;
      auto from = mine[selected_piece];
      moves = game.moves(from);
      sort(moves.begin(), moves.end(), [](Move a, Move b) { return a.to < b.to; });
      moves.insert(moves.begin(), Move{from, from});

      state = SELECT_MOVE;
      selected = 0;

    } else if (state == SELECT_MOVE) {
      if (selected == 0) {
        state = SELECT_PIECE;
        selected = selected_piece;

      } else {
        selected %= moves.size();
        last = moves[selected];
        make(last);
        copyPieces();

        over = game.isOver();
        if (over) {
          state = GAME_OVER;
        }
      }
    }

    requestUpdate();
  }
}

void ChessActivity::make(Move move) {
  if (mode == PUZZLES) {
    puzzleState = puzzle.propose(move);
    last = puzzle.last;
    copyPieces();

    if (puzzleState == Puzzle::RIGHT) {
      state = SELECT_PIECE;
      selected = 0;
      info = "Go on";
      btnL = "";
      btnR = "Hint";
    } else if (puzzleState == Puzzle::WRONG) {
      state = IDLE;
      info = "Not quite";
      btnL = "Undo";
      btnR = "";
    } else if (puzzleState == Puzzle::SOLVED) {
      state = IDLE;
      info = "Good job!";
      btnL = "Again";
      btnR = "Next";
    }

  } else if (mode == OTB) {
    game.make(move);
    savePosition();
    state = SELECT_PIECE;
    selected = 0;
  }
}

void ChessActivity::copyPieces() {
  pieces = {};
  mine = {};

  for (int i = 0; i < 64; i++) {
    int piece = game.pieces[i];
    pieces.push_back(piece);

    if (piece & game.turn) {
      mine.push_back(i);
    }
  }
}

void ChessActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();
  const int boardSize = pageWidth - 10;
  const int cellSize = boardSize / 8;

  renderer.clearScreen();

  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, header.c_str());

  const int boardX = (pageWidth - boardSize) / 2;
  const int contentTop = metrics.topPadding + metrics.headerHeight + 4;
  const int contentBot = pageHeight - metrics.buttonHintsHeight - 4;
  const int boardY = contentTop + (contentBot - contentTop - boardSize - 24) / 2;
  const int right = boardX + boardSize;
  const int bottom = boardY + boardSize;

  renderer.drawCenteredText(SMALL_FONT_ID, boardY - 25, info.c_str());

  int selected_square = -1;
  int piece_square = -1;

  if (mine.size() && state == SELECT_PIECE) {
    selected %= mine.size();
    selected_square = mine[selected];

  } else if (state == SELECT_MOVE) {
    piece_square = mine[selected_piece];
    selected %= moves.size();
    selected_square = moves[selected].to;

    // Draw legal moves
    for (int m = 1; m < moves.size(); m++) {
      int to = moves[m].to;
      int size = cellSize / 5;
      int cx = (to % 8) * cellSize + boardX + (cellSize - size) / 2;
      int cy = (to / 8) * cellSize + boardY + (cellSize - size) / 2;
      renderer.fillRoundedRect(cx, cy, size, size, size / 2, Color::Black);
    }
  }

  // Draw board squares and pieces
  for (int r = 0; r < 8; r++) {
    int off = cellSize * r;
    int offY = boardY + off;
    int offX = boardX + off;

    for (int c = 0; c < 8; c++) {
      int i = (r * 8) + c;
      int cx = boardX + c * cellSize;
      int cy = boardY + r * cellSize;
      bool darkSquare = (r + c) % 2 == 1;

      if (darkSquare) {
        int h = cellSize / 4;
        for (int d = h; d <= cellSize; d += h) {
          renderer.drawLine(cx + d, cy, cx, cy + d);
        }
        for (int d = h; d < cellSize - 1; d += h) {
          renderer.drawLine(cx + cellSize - d, cy + cellSize, cx + cellSize, cy + cellSize - d);
        }
      }

      if (selected_square == i) {
        renderer.drawRect(cx, cy, cellSize, cellSize, 3, true);
      }

      if (piece_square == i) {
        renderer.drawRoundedRect(cx, cy, cellSize, cellSize, 3, cellSize / 2, true);
      }

      if (last.from == i || last.to == i) {
        renderer.drawRect(cx, cy, cellSize, cellSize, 2, true);
      }

      if (i < pieces.size() && pieces[i] != Game::EMPTY) {
        int piece = pieces[i];
        int type = piece & Game::TYPE;
        int color = piece & Game::COLOR;

        const char* pieceStr = "?";
        if (type == Game::PAWN) pieceStr = "p";
        if (type == Game::ROOK) pieceStr = "r";
        if (type == Game::KNIGHT) pieceStr = "n";
        if (type == Game::BISHOP) pieceStr = "b";
        if (type == Game::QUEEN) pieceStr = "q";
        if (type == Game::KING) pieceStr = "k";

        auto font = NOTOSERIF_16_FONT_ID;
        if (color == Game::WHITE) {
          char upper = pieceStr[0] - 'a' + 'A';
          pieceStr = string{upper}.c_str();
          font = NOTOSANS_16_FONT_ID;
        }

        int tW = renderer.getTextWidth(font, pieceStr);
        int tH = renderer.getTextHeight(font);
        int px = cx + (cellSize - tW) / 2;
        int py = cy + (cellSize - tH) / 2;
        renderer.drawText(font, px, py, pieceStr, true);
      }
    }

    renderer.drawLine(boardX, offY, right, offY);
    renderer.drawLine(offX, boardY, offX, bottom);
  }

  renderer.drawLine(boardX, bottom, right, bottom);
  renderer.drawLine(right, boardY, right, bottom);

  // Status line
  int statusY = boardY + boardSize + 6;

  string status = "";
  if (state == SELECT_PIECE) {
    status = (selected_square > -1) ? Game::print(selected_square) : "";
  } else if (state == SELECT_MOVE) {
    status = selected ? Game::print(moves[selected]) : "Cancel";
  } else if (state == GAME_OVER) {
    status = (over == Game::CHECKMATE) ? "CHECKMATE!" : "Stalemate =|";
  } else if (state == WAIT) {
    status = "Wait...";
  }
  renderer.drawCenteredText(SMALL_FONT_ID, statusY, status.c_str());

  // Buttons
  const auto labels = mappedInput.mapLabels("Quit", "Mode", btnL.c_str(), btnR.c_str());
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}

void ChessActivity::loadMode() {
  HalFile file = Storage.open("/chess_mode.json");
  if (!file) return;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();
  if (err) return;

  mode = doc["mode"];
  level = doc["level"];
}

void ChessActivity::saveMode() {
  auto file = Storage.open("/chess_mode.json", O_WRITE | O_CREAT | O_TRUNC);
  if (!file) return;

  JsonDocument doc;
  doc["mode"] = mode;
  doc["level"] = level;

  serializeJson(doc, file);
  file.close();
}

void ChessActivity::savePosition() {
  auto file = Storage.open("/chess_position.fen", O_WRITE | O_CREAT | O_TRUNC);
  if (!file) return;

  string position = game.fen();
  file.write(position.c_str(), position.size());
  file.close();
}

string ChessActivity::loadPosition() {
  HalFile file = Storage.open("/chess_position.fen");
  if (!file) return Game::STARTPOS;

  char buffer[file.size()];
  file.read(buffer, file.size());
  file.close();

  return string(buffer);
}

int ChessActivity::loadPuzzleIndex() {
  HalFile file = Storage.open("/chess_puzzle_index.json");
  if (!file) return 0;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();
  if (err) return 0;

  string difficulty = puzzleDifficulty(level);
  if (!doc[difficulty].is<int>()) return 0;
  return doc[difficulty];
}

void ChessActivity::savePuzzleIndex(int index) {
  HalFile file = Storage.open("/chess_puzzle_index.json", O_READ | O_WRITE | O_CREAT | O_TRUNC);
  if (!file) return;

  JsonDocument doc;
  deserializeJson(doc, file);

  string difficulty = puzzleDifficulty(level);
  doc[difficulty] = index;
  serializeJson(doc, file);

  file.close();
}

void ChessActivity::startPuzzle() {
  string difficulty = puzzleDifficulty(level);
  string filename = "/chess_puzzles_" + difficulty + ".json";
  HalFile file;

  if (!Storage.openFileForRead("CHESS", filename, file)) {
    downloadPuzzles(filename);
    return;
  }

  int index = loadPuzzleIndex();
  LOG_DBG("CHESS", "Index %i", index);

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();

  if (err) {
    LOG_ERR("CHESS", "Parsing failed %s", err.c_str());
    info = "Could not load puzzles";
    return;
  }

  if (index >= doc["puzzles"].size()) {
    downloadPuzzles(filename);
    return;
  }

  JsonObject jsonPuzzle = doc["puzzles"][index];
  string pgn = jsonPuzzle["game"]["pgn"];
  JsonArray jsonSolution = jsonPuzzle["puzzle"]["solution"];

  vector<string> solution;
  for (const char* step : jsonSolution) {
    solution.push_back(string(step));
  }

  LOG_DBG("CHESS", "Start puzzle %s", pgn.c_str());
  puzzle.start(pgn, solution);
  puzzleState = Puzzle::RIGHT;
  copyPieces();
  state = SELECT_PIECE;
  selected = 0;

  stringstream infos;
  infos << "Puzzle " << (index + 1) << "/" << doc["puzzles"].size();
  info = infos.str();
}

void ChessActivity::downloadPuzzles(string filename) {
  WiFi.mode(WIFI_STA);
  startActivityForResult(std::make_unique<WifiSelectionActivity>(renderer, mappedInput),
                         [this, filename](const ActivityResult& wifi) {
                           if (wifi.isCancelled) {
                             LOG_DBG("CHESS", "No wifi");
                             info = "Could not connect to WiFi";
                             return;
                           }

                           string puzzlesUrl = "https://lichess.org/api/puzzle/batch/mix?nb=50";

                           string difficulty = puzzleDifficulty(level);
                           if (difficulty != "any") {
                             puzzlesUrl += "&difficulty=" + difficulty;
                           }

                           LOG_DBG("CHESS", "GET %s", puzzlesUrl.c_str());
                           auto result = HttpDownloader::downloadToFile(puzzlesUrl, filename, nullptr);

                           if (result != HttpDownloader::OK) {
                             LOG_ERR("CHESS", "Download failed %i", result);
                             info = "Could not download puzzles";
                             return;
                           }

                           LOG_DBG("CHESS", "Start puzzle %s", difficulty.c_str());
                           startPuzzle();
                         });
}