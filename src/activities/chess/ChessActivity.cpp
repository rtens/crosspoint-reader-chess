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

string BASE_PATH = "./.config";

void ChessActivity::onEnter() {
  Activity::onEnter();

  if (!Storage.exists(BASE_PATH.c_str())) {
    Storage.mkdir(BASE_PATH.c_str());
  }

  loadConfig();
  loadMode();
  onModeSelected(mode, level);

  savedOrientation = renderer.getOrientation();
  renderer.setOrientation(GfxRenderer::Orientation::Portrait);
  requestUpdate();
}

string puzzleDifficulty(int level) {
  switch (level) {
    case -2:
      return "easiest";
    case -1:
      return "easier";
    case 1:
      return "harder";
    case 2:
      return "hardest";
    default:
      return "normal";
  }
}

void ChessActivity::onModeSelected(Mode mode, int level) {
  this->mode = mode;
  this->level = level;

  header = "Chess";

  if (mode == PUZZLES) {
    header += " Puzzles";
    if (level) header += " (" + puzzleDifficulty(level) + ")";
    startPuzzle();
  }
  saveMode();
}

void ChessActivity::onExit() {
  Activity::onExit();
  renderer.setOrientation(savedOrientation);
}

void ChessActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    onGoHome();
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    // Open mode selection activity
    return;
  }

  if (mode == PUZZLES) {
    if (puzzleState == Puzzle::WRONG) {
      if (mappedInput.wasReleased(MappedInputManager::Button::Up)) {
        puzzle.undo();
        copyPieces();
        info = "Try again";
        puzzleState = Puzzle::RIGHT;
        state = SELECT_PIECE;
        btnU = "Select";
        btnD = "Next Piece";
        selected = selected_piece;
        last = Move{};
        requestUpdate();
        return;
      }

    } else if (puzzleState == Puzzle::SOLVED) {
      if (mappedInput.wasReleased(MappedInputManager::Button::Up)) {
        startPuzzle();
        requestUpdate();
        return;
      }

      if (mappedInput.wasReleased(MappedInputManager::Button::Down)) {
        if (puzzleState == Puzzle::SOLVED) {
          int index = loadPuzzleIndex();
          savePuzzleIndex(index + 1);
          startPuzzle();
          requestUpdate();
          return;
        }
      }
    }
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Down)) {
    selected++;
    if (state == SELECT_MOVE) {
      selected %= moves.size();
      if (selected) {
        btnU = "Make Move";
      } else {
        btnU = "Cancel";
      }
    }
    requestUpdate();
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Up)) {
    if (state == SELECT_PIECE) {
      if (!mine.size()) return;

      selected %= mine.size();
      selected_piece = selected;
      auto from = mine[selected_piece];
      moves = game.moves(from);

      if (pov == Game::WHITE) {
        sort(moves.begin(), moves.end(), [](Move a, Move b) { return a.to < b.to; });
      } else {
        sort(moves.begin(), moves.end(), [](Move a, Move b) { return a.to > b.to; });
      }
      moves.insert(moves.begin(), Move{from, from});

      state = SELECT_MOVE;
      selected = 0;
      btnU = "Cancel";
      btnD = "Next Move";

    } else if (state == SELECT_MOVE) {
      if (selected == 0) {
        state = SELECT_PIECE;
        selected = selected_piece;
        btnU = "Select";
        btnD = "Next Piece";

      } else {
        selected %= moves.size();
        last = moves[selected];
        make(last);
        copyPieces();
      }
    }

    requestUpdate();
    return;
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
      for (int i = 0; i < mine.size(); i++) {
        if (mine[i] == move.to) {
          selected = i;
          break;
        }
      }
      info = "Go on";
      btnU = "Select";
      btnD = "Next Piece";
      btnL = "";
      btnR = "";

    } else if (puzzleState == Puzzle::WRONG) {
      state = IDLE;
      info = "Not quite";
      btnU = "Undo";
      btnD = "";
      btnL = "";
      btnR = "";

    } else if (puzzleState == Puzzle::SOLVED) {
      state = IDLE;
      info = "Good job!";
      btnU = "Again";
      btnD = "Next Puzzle";
      btnL = "";
      btnR = "";
    }

    over = game.isOver();
    if (over) {
      state = GAME_OVER;
    }

  } else if (mode == OTB) {
    game.make(move);
    savePosition();
    state = SELECT_PIECE;
    selected = 0;
    btnU = "Select";
    btnD = "Next Piece";
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

  if (pov == Game::WHITE) {
    sort(mine.begin(), mine.end(), [](int a, int b) { return a < b; });
  } else {
    sort(mine.begin(), mine.end(), [](int a, int b) { return a > b; });
  }
}

void drawSideButton(const GfxRenderer& renderer, string text, int y, bool left = true) {
  if (text == "") return;

  int p = 7;
  int tw = renderer.getTextWidth(SMALL_FONT_ID, text.c_str());
  int th = renderer.getTextHeight(SMALL_FONT_ID) + 5;
  int x = left ? 0 : (renderer.getScreenWidth() - tw - p * 4);
  renderer.drawText(SMALL_FONT_ID, x + 2 * p, y + p, text.c_str());
  renderer.drawRect(x, y, tw + p * 4, th + p * 2, 1);
}

void ChessActivity::render(RenderLock&&) {
  LOG_DBG("CHESS", "Render start");
  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();
  const int cellSize = (pageWidth - 10) / 8;
  const int boardSize = cellSize * 8;

  renderer.clearScreen();

  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, header.c_str());

  const int boardX = (pageWidth - boardSize) / 2;
  const int contentTop = metrics.topPadding + metrics.headerHeight + 4;
  const int contentBot = pageHeight - metrics.buttonHintsHeight - 4;
  const int boardY = contentTop + (contentBot - contentTop - boardSize - 24) / 2;
  const int right = boardX + boardSize;
  const int bottom = boardY + boardSize;

  int selected_square = -1;
  int piece_square = -1;

  LOG_DBG("CHESS", "Selected stuff");
  if (mine.size() && state == SELECT_PIECE) {
    selected %= mine.size();
    selected_square = mine[selected];

  } else if (state == SELECT_MOVE) {
    piece_square = mine[selected_piece];
    selected %= moves.size();
    selected_square = moves[selected].to;
  }

  const int statusY = boardY - 45;
  string status = "";
  if (state == SELECT_PIECE) {
    status = (selected_square > -1) ? Game::print(selected_square) : "";
  } else if (state == SELECT_MOVE) {
    status = selected ? Game::print(moves[selected]) : Game::print(selected_square);
  } else if (state == GAME_OVER) {
    status = (over == Game::CHECKMATE) ? "CHECKMATE!" : "Stalemate =|";
  } else if (state == WAIT) {
    status = "Wait...";
  }
  renderer.drawCenteredText(UI_10_FONT_ID, statusY, status.c_str());

  drawSideButton(renderer, btnU, statusY - 7);
  drawSideButton(renderer, btnD, statusY - 7, false);

  // Draw board squares and pieces
  LOG_DBG("CHESS", "Render board and pieces");
  for (int r = 0; r < 8; r++) {
    int off = cellSize * r;
    int offY = boardY + off;
    int offX = boardX + off;

    for (int c = 0; c < 8; c++) {
      int i = (r * 8) + c;
      int cx = boardX + c * cellSize;
      int cy = boardY + r * cellSize;

      if (pov == Game::BLACK) {
        cx = right - cellSize - (cx - boardX);
        cy = bottom - cellSize - (cy - boardY);
      }

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

        const char* pieceStr = "?";
        if (piece == (Game::WHITE | Game::PAWN)) pieceStr = "\u2659";
        if (piece == (Game::WHITE | Game::KNIGHT)) pieceStr = "\u2658";
        if (piece == (Game::WHITE | Game::BISHOP)) pieceStr = "\u2657";
        if (piece == (Game::WHITE | Game::ROOK)) pieceStr = "\u2656";
        if (piece == (Game::WHITE | Game::QUEEN)) pieceStr = "\u2655";
        if (piece == (Game::WHITE | Game::KING)) pieceStr = "\u2654";
        if (piece == (Game::BLACK | Game::PAWN)) pieceStr = "\u265F";
        if (piece == (Game::BLACK | Game::KNIGHT)) pieceStr = "\u265E";
        if (piece == (Game::BLACK | Game::BISHOP)) pieceStr = "\u265D";
        if (piece == (Game::BLACK | Game::ROOK)) pieceStr = "\u265C";
        if (piece == (Game::BLACK | Game::QUEEN)) pieceStr = "\u265B";
        if (piece == (Game::BLACK | Game::KING)) pieceStr = "\u265A";

        auto font = NOTOSANS_32_EMOJI_FONT_ID;
        if (config.pieceSet == "emoji48") {
          font = NOTOSANS_48_EMOJI_FONT_ID;
        } else if (config.pieceSet == "emoji16") {
          font = NOTOSANS_16_EMOJI_FONT_ID;
        }

        int tW = renderer.getTextWidth(font, pieceStr);
        int tH = renderer.getTextHeight(font);
        int px = cx + (cellSize - tW) / 2;
        int py = cy + (cellSize - tH) / 2 - tH / 3;
        renderer.drawText(font, px, py, pieceStr, true);
      }
    }

    renderer.drawLine(boardX, offY, right, offY);
    renderer.drawLine(offX, boardY, offX, bottom);
  }

  renderer.drawLine(boardX, bottom, right, bottom);
  renderer.drawLine(right, boardY, right, bottom);

  LOG_DBG("CHESS", "Render legal moves");
  // Draw legal moves
  if (state == SELECT_MOVE) {
    int size = cellSize / 4;
    int s = 2;
    for (int m = 1; m < moves.size(); m++) {
      int to = moves[m].to;
      int cx = boardX + (to % 8) * cellSize;
      int cy = boardY + (to / 8) * cellSize;

      if (pov == Game::BLACK) {
        cx = right - cellSize - (cx - boardX);
        cy = bottom - cellSize - (cy - boardY);
      }

      cx += (cellSize - size) / 2;
      cy += (cellSize - size) / 2;

      if (game.turn == Game::WHITE) {
        renderer.fillRoundedRect(cx - s, cy - s, size + s * 2, size + s * 2, (size + s * 2) / 2, Color::Black);
        renderer.fillRoundedRect(cx, cy, size, size, size / 2, Color::White);
      } else {
        renderer.fillRoundedRect(cx, cy, size, size, size / 2, Color::Black);
      }
    }
  }

  // Info line
  int infoY = boardY + boardSize + 15;
  renderer.drawCenteredText(UI_10_FONT_ID, infoY, info.c_str());

  // Buttons
  const auto labels = mappedInput.mapLabels("Quit", "", btnL.c_str(), btnR.c_str());
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
  LOG_DBG("CHESS", "Render done");
}

void ChessActivity::loadConfig() {
  HalFile file = Storage.open("/.chess/config.json");
  if (!file) return;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();
  if (err) return;

  if (doc["pieceSet"].is<string>()) {
    config.pieceSet = doc["pieceSet"].as<string>();
  }
}

void ChessActivity::loadMode() {
  HalFile file = Storage.open("/.chess/mode.json");
  if (!file) return;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();
  if (err) return;

  if (doc["mode"].is<string>()) {
    string modeString = doc["mode"];
    if (modeString == "puzzles") {
      mode = PUZZLES;
    } else if (modeString == "otb") {
      mode = OTB;
    } else if (modeString == "engine") {
      mode = ENGINE;
    }
  }
  if (doc["level"].is<int>()) {
    level = doc["level"];
  }
}

void ChessActivity::saveMode() {
  auto file = Storage.open("/.chess/mode.json", O_WRITE | O_CREAT | O_TRUNC);
  if (!file) return;

  JsonDocument doc;

  if (mode == OTB) {
    doc["mode"] = "otb";
  } else if (mode == PUZZLES) {
    doc["mode"] = "puzzles";
  } else if (mode == ENGINE) {
    doc["mode"] = "engine";
  }
  doc["level"] = level;

  serializeJson(doc, file);
  file.close();
}

void ChessActivity::savePosition() {
  auto file = Storage.open("/.chess/position.fen", O_WRITE | O_CREAT | O_TRUNC);
  if (!file) return;

  string position = game.fen();
  file.write(position.c_str(), position.size());
  file.close();
}

string ChessActivity::loadPosition() {
  HalFile file = Storage.open("/.chess/position.fen");
  if (!file) return Game::STARTPOS;

  char buffer[file.size()];
  file.read(buffer, file.size());
  file.close();

  return string(buffer);
}

int ChessActivity::loadPuzzleIndex() {
  HalFile file = Storage.open("/.chess/puzzle_index.json");
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
  HalFile reading = Storage.open("/.chess/puzzle_index.json");
  if (!reading) return;

  JsonDocument doc;
  DeserializationError readingErr = deserializeJson(doc, reading);
  reading.close();
  if (readingErr) return;

  HalFile file = Storage.open("/.chess/puzzle_index.json", O_WRITE | O_CREAT | O_TRUNC);
  if (!file) return;

  string difficulty = puzzleDifficulty(level);
  doc[difficulty] = index;
  serializeJson(doc, file);

  file.close();
}

void ChessActivity::startPuzzle() {
  string difficulty = puzzleDifficulty(level);
  string filename = "/.chess/puzzles_" + difficulty + ".json";
  HalFile file;

  if (!Storage.openFileForRead("CHESS", filename, file)) {
    LOG_DBG("CHESS", "Cannot open %s - download puzzles", filename.c_str());
    downloadPuzzles(filename);
    return;
  }

  int index = loadPuzzleIndex();
  LOG_DBG("CHESS", "Index %i", index);

  LOG_DBG("CHESS", "Reading puzzles");
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();

  if (err) {
    LOG_ERR("CHESS", "Parsing failed %s", err.c_str());
    info = "Could not load puzzles";
    return;
  }

  if (index >= doc["puzzles"].size()) {
    LOG_DBG("CHESS", "End of batch - download puzzles");
    downloadPuzzles(filename);
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
  puzzle.start(pgn, solution);
  LOG_DBG("CHESS", "Puzzle started");
  puzzleState = Puzzle::RIGHT;
  pov = game.turn;
  copyPieces();
  last = Move{};
  state = SELECT_PIECE;
  selected = 0;
  btnU = "Select";
  btnD = "Next Piece";
  btnL = "";
  btnR = "";

  stringstream infos;
  infos << "Puzzle " << (index + 1) << "/" << doc["puzzles"].size();
  info = infos.str();
  LOG_DBG("CHESS", "Done starting puzzle");
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

                           savePuzzleIndex(0);

                           string puzzlesUrl = "https://lichess.org/api/puzzle/batch/mix?nb=50";

                           string difficulty = puzzleDifficulty(level);
                           if (level) {
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