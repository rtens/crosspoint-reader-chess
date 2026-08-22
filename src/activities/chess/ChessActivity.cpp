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
using namespace std;

#include "ChessModeSelectionActivity.h"
#include "ChessState.h"
#include "activities/ActivityResult.h"
#include "activities/network/WifiSelectionActivity.h"
#include "components/UITheme.h"
#include "fontIds.h"
#include "modes/EngineStates.h"
#include "modes/MoveStates.h"
#include "modes/OtbStates.h"
#include "modes/PuzzleStates.h"
#include "network/HttpDownloader.h"

void ChessActivity::onEnter() {
  Activity::onEnter();

  config = storage.loadConfig();
  calculateLayoutParams();

  onModeSelected(storage.loadMode());
  if (config.pieceSet != "default") {
    pieceSet = storage.loadPieceSet(config.pieceSet);
  }

  savedOrientation = renderer.getOrientation();
  renderer.setOrientation(GfxRenderer::Orientation::Portrait);
  requestUpdate();
}

void ChessActivity::onExit() {
  Activity::onExit();
  if (state) delete state;
  if (pieceSet) delete pieceSet;
  renderer.setOrientation(savedOrientation);
}

void ChessActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    onGoHome();

  } else if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    startActivityForResult(make_unique<ChessModeSelectionActivity>(renderer, mappedInput),
                           [this](const ActivityResult& result) {
                             if (result.isCancelled) return;
                             ChessModeResult modeResult = get<ChessModeResult>(result.data);
                             ChessMode mode{modeResult.id, modeResult.level};
                             onModeSelected(mode);
                           });

  } else if (mappedInput.wasReleased(MappedInputManager::Button::Left)) {
    state = state->left();
    requestUpdate();

  } else if (mappedInput.wasReleased(MappedInputManager::Button::Right)) {
    state = state->right();
    requestUpdate();

  } else if (mappedInput.wasReleased(MappedInputManager::Button::Up)) {
    state = state->up();
    requestUpdate();

  } else if (mappedInput.wasReleased(MappedInputManager::Button::Down)) {
    state = state->down();
    requestUpdate();
  }
}

void ChessActivity::onModeSelected(ChessMode mode) {
  LOG_DBG("CHESS", "on Mode Selected %i %s", mode.id, mode.level.c_str());

  if (mode.id == ChessModeSelectionActivity::PUZZLE_MIX) {
    level = mode.level;
    headerText = "Chess Puzzles";
    if (level != "normal") {
      headerText = "Chess: " + level + " Puzzles";
    }
    storage.saveMode(mode);
    if (state) delete state;
    state = new MoveStartState(this, new PuzzleStartState(this));

  } else if (mode.id == ChessModeSelectionActivity::DAILY_PUZZLE) {
    level = "daily";
    headerText = "Chess: Daily Puzzle";
    storage.saveMode(mode);
    if (state) delete state;
    state = new MoveStartState(this, new PuzzleStartState(this));

  } else if (mode.id == ChessModeSelectionActivity::ENGINE) {
    level = mode.level;
    headerText = "Chess vs " + mode.level;
    storage.saveMode(mode);
    if (state) delete state;
    state = new MoveStartState(this, new EngineRunningState(this));

  } else if (mode.id == ChessModeSelectionActivity::OTB) {
    level = "";
    headerText = "Chess";
    storage.saveMode(mode);
    if (state) delete state;
    state = new MoveStartState(this, new OtbStartState(this));

  } else if (mode.id == ChessModeSelectionActivity::PIECE_SET) {
    config.pieceSet = mode.level;
    storage.saveConfig(config);

    if (pieceSet) delete pieceSet;
    pieceSet = 0;

    if (config.pieceSet != "default") {
      pieceSet = storage.loadPieceSet(config.pieceSet);
    }
  }

  requestUpdate();
}

void ChessActivity::calculateLayoutParams() {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  const int contentTop = metrics.topPadding + metrics.headerHeight + 4;
  const int contentBot = pageHeight - metrics.buttonHintsHeight - 4;

  cellSize = (pageWidth - 10) / 8;
  boardSize = cellSize * 8;
  LOG_DBG("CHESS", "cellSize %i", cellSize);

  boardX = (pageWidth - boardSize) / 2;
  boardY = contentTop + (contentBot - contentTop - boardSize - 24) / 2;
}

void ChessActivity::render(RenderLock&&) {
  LOG_DBG("CHESS", "Render start");
  renderer.clearScreen();

  renderHeader();
  renderSideButtons();
  renderStatus();
  renderPieces();
  renderBoard();
  renderMove();
  renderLastMove();
  renderMoves();
  renderInfo();
  renderButtons();

  if (movesSinceRefresh >= config.movesUntilRefresh) {
    movesSinceRefresh = 0;
    renderer.displayBuffer(HalDisplay::HALF_REFRESH);
  } else {
    renderer.displayBuffer();
  }

  LOG_DBG("CHESS", "Render done");
}

void ChessActivity::renderHeader() {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto rect = Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight};
  GUI.drawHeader(renderer, rect, headerText.c_str());
}

void ChessActivity::renderSideButtons() {
  renderSideButton(btnUp);
  renderSideButton(btnDown, false);
}

void ChessActivity::renderSideButton(string text, bool left) {
  if (text == "") return;

  int p = 7;
  int tw = renderer.getTextWidth(SMALL_FONT_ID, text.c_str());
  int th = renderer.getTextHeight(SMALL_FONT_ID) + 5;
  int x = left ? 0 : (renderer.getScreenWidth() - tw - p * 4);
  int y = boardY - 52;

  renderer.drawText(SMALL_FONT_ID, x + 2 * p, y + p, text.c_str());
  renderer.drawRect(x, y, tw + p * 4, th + p * 2, 1);
}

void ChessActivity::renderStatus() {
  int y = boardY - 45;
  renderer.drawCenteredText(UI_10_FONT_ID, y, statusText.c_str());
}

void ChessActivity::renderPieces() {
  LOG_DBG("CHESS", "render Pieces");
  for (int i = 0; i < 64; i++) {
    int piece = game.pieces[i];
    if (piece == Game::EMPTY) continue;

    XY square = squareXY(i);

    if (config.pieceSet == "default") {
      renderDefaultPiece(piece, square);
    } else {
      renderPieceFromSet(piece, square);
    }
  }
}

void ChessActivity::renderDefaultPiece(uint8_t piece, XY sq) {
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

  int tW = renderer.getTextWidth(font, pieceStr);
  int tH = renderer.getTextHeight(font);

  int px = sq.x + (cellSize - tW) / 2;
  int py = sq.y + (cellSize - tH) / 2 - tH / 3;

  renderer.drawText(font, px, py, pieceStr, true);
}

void ChessActivity::renderPieceFromSet(uint8_t piece, XY sq) {
  if (!pieceSet) {
    renderDefaultPiece(piece, sq);
    return;
  }

  int size = static_cast<uint8_t>(pieceSet[0]);
  int d = size * size / 8;
  int offset = 0;

  if (piece == (Game::BLACK | Game::BISHOP)) offset = d * 0;
  if (piece == (Game::BLACK | Game::KING)) offset = d * 1;
  if (piece == (Game::BLACK | Game::KNIGHT)) offset = d * 2;
  if (piece == (Game::BLACK | Game::PAWN)) offset = d * 3;
  if (piece == (Game::BLACK | Game::QUEEN)) offset = d * 4;
  if (piece == (Game::BLACK | Game::ROOK)) offset = d * 5;
  if (piece == (Game::WHITE | Game::BISHOP)) offset = d * 6;
  if (piece == (Game::WHITE | Game::KING)) offset = d * 7;
  if (piece == (Game::WHITE | Game::KNIGHT)) offset = d * 8;
  if (piece == (Game::WHITE | Game::PAWN)) offset = d * 9;
  if (piece == (Game::WHITE | Game::QUEEN)) offset = d * 10;
  if (piece == (Game::WHITE | Game::ROOK)) offset = d * 11;

  int x = sq.x + (cellSize - size) / 2;
  int y = sq.y + (cellSize - size) / 2;
  renderer.drawImage(pieceSet + 1 + offset, x, y, size, size);
}

void ChessActivity::renderBoard() {
  LOG_DBG("CHESS", "render Board");
  const int right = boardX + boardSize;
  const int bottom = boardY + boardSize;

  for (int r = 0; r < 8; r++) {
    for (int c = 0; c < 8; c++) {
      if ((r + c) % 2) {
        renderDarkSquare(c, r);
      }
    }

    int off = cellSize * r;
    int offY = boardY + off;
    int offX = boardX + off;

    renderer.drawLine(boardX, offY, right, offY);
    renderer.drawLine(offX, boardY, offX, bottom);
  }

  renderer.drawLine(boardX, bottom, right, bottom);
  renderer.drawLine(right, boardY, right, bottom);
}

void ChessActivity::renderDarkSquare(int c, int r) {
  XY s = squareXY(c, r);
  int cs = cellSize;
  int h = cs / 4;

  renderer.drawLine(s.x, s.y + cs, s.x + cs, s.y);
  for (int d = h; d < cs - h / 2; d += h) {
    renderer.drawLine(s.x, s.y + cs - d, s.x + cs - d, s.y);
    renderer.drawLine(s.x + d, s.y + cs, s.x + cs, s.y + d);
  }
}

void ChessActivity::renderMove() {
  LOG_DBG("CHESS", "render Move");
  if (move.from != Game::NOWHERE) {
    XY square = squareXY(move.from);
    renderer.drawRect(square.x, square.y, cellSize, cellSize, 4, true);
  }

  if (move.to != Game::NOWHERE) {
    XY square = squareXY(move.to);
    renderer.drawRoundedRect(square.x, square.y, cellSize, cellSize, 4, cellSize / 2, true);
  }
}

void ChessActivity::renderLastMove() {
  LOG_DBG("CHESS", "render last Move");
  if (last.from == Game::NOWHERE) return;

  XY from = squareXY(last.from);
  XY to = squareXY(last.to);
  renderer.drawRect(from.x, from.y, cellSize, cellSize, 2, true);
  renderer.drawRoundedRect(to.x, to.y, cellSize, cellSize, 2, cellSize / 2, true);
}

void ChessActivity::renderMoves() {
  LOG_DBG("CHESS", "render Moves");
  int size = cellSize / 4;
  int s = 3;

  for (int m = 0; m < moves.size(); m++) {
    XY square = squareXY(moves[m].to);

    int cx = square.x + (cellSize - size) / 2;
    int cy = square.y + (cellSize - size) / 2;

    if (game.turn == Game::WHITE) {
      renderer.fillRoundedRect(cx - s, cy - s, size + s * 2, size + s * 2, (size + s * 2) / 2, Color::Black);
      renderer.fillRoundedRect(cx, cy, size, size, size / 2, Color::White);
    } else {
      renderer.fillRoundedRect(cx, cy, size, size, size / 2, Color::Black);
    }
  }
}

void ChessActivity::renderInfo() {
  LOG_DBG("CHESS", "render Info");
  int y = boardY + boardSize + 15;
  renderer.drawCenteredText(UI_10_FONT_ID, y, infoText.c_str());
}

void ChessActivity::renderButtons() {
  const auto labels = mappedInput.mapLabels("Quit", "Mode", btnLeft.c_str(), btnRight.c_str());
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}

ChessActivity::XY ChessActivity::squareXY(int c, int r) {
  const int right = boardX + boardSize;
  const int bottom = boardY + boardSize;

  int x = boardX + c * cellSize;
  int y = boardY + r * cellSize;

  if (pov == Game::BLACK) {
    x = right - cellSize - (x - boardX);
    y = bottom - cellSize - (y - boardY);
  }

  return XY{x, y};
}

ChessActivity::XY ChessActivity::squareXY(int i) { return squareXY(i % 8, i / 8); }

void ChessActivity::startWifi(function<void()> then) {
  WiFi.mode(WIFI_STA);
  startActivityForResult(make_unique<WifiSelectionActivity>(renderer, mappedInput),
                         [this, then](const ActivityResult& wifi) {
                           if (wifi.isCancelled) {
                             infoText = "Could not connect to WiFi";
                             return;
                           }

                           then();
                         });
}