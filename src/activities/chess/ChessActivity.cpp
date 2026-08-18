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

#include "./ChessModeSelectionActivity.h"
#include "activities/network/WifiSelectionActivity.h"
#include "components/UITheme.h"
#include "fontIds.h"
#include "network/HttpDownloader.h"
#include "states/Move.h"
#include "states/Otb.h"

void ChessActivity::onEnter() {
  Activity::onEnter();

  // config = loadConfig();
  // mode = loadMode();
  calculateLayoutParams();
  onModeSelected(ChessMode{ChessModeSelectionActivity::OTB});

  savedOrientation = renderer.getOrientation();
  renderer.setOrientation(GfxRenderer::Orientation::Portrait);
  requestUpdate();
}

void ChessActivity::onExit() {
  Activity::onExit();
  renderer.setOrientation(savedOrientation);
  delete state;
}

void ChessActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    onGoHome();

  } else if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    activityManager.pushActivity(std::make_unique<ChessModeSelectionActivity>(
        renderer, mappedInput, [this](ChessMode mode) { onModeSelected(mode); }));

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
  // saveMode(mode);

  // if (mode.id == ChessMode::PUZZLE_MIX) {
  // state = PuzzleMixStart(this, level);

  // } else if (mode == "Daily Puzzle") {
  //   state = DailyPuzzleStart(this);

  // } else if (mode == "Play vs Engine") {
  //   state = PlayEngineStart(this, level);

  // } else
  if (mode.id == ChessModeSelectionActivity::OTB) {
    state = new MoveStartState(this, new OtbStartState(this));
  }
}

void ChessActivity::calculateLayoutParams() {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  const int contentTop = metrics.topPadding + metrics.headerHeight + 4;
  const int contentBot = pageHeight - metrics.buttonHintsHeight - 4;

  cellSize = (pageWidth - 10) / 8;
  boardSize = cellSize * 8;

  boardX = (pageWidth - boardSize) / 2;
  boardY = contentTop + (contentBot - contentTop - boardSize - 24) / 2;
}

void ChessActivity::render(RenderLock&&) {
  LOG_DBG("CHESS", "Render start");
  renderer.clearScreen();

  renderHeader();
  renderSideButtons();
  renderStatus();
  renderBoard();
  renderPieces();
  renderMove();
  renderLastMove();
  renderMoves();
  renderInfo();
  renderButtons();

  renderer.displayBuffer();
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

void ChessActivity::renderBoard() {
  const int right = boardX + boardSize;
  const int bottom = boardY + boardSize;

  for (int r = 0; r < 8; r++) {
    for (int c = 0; c < 8; c++) {
      XY square = squareXY(c, r);

      bool darkSquare = (r + c) % 2 == 1;
      if (darkSquare) {
        int h = cellSize / 4;
        for (int d = h; d <= cellSize; d += h) {
          renderer.drawLine(square.x + d, square.y, square.x, square.y + d);
        }
        for (int d = h; d < cellSize - 1; d += h) {
          renderer.drawLine(square.x + cellSize - d, square.y + cellSize, square.x + cellSize, square.y + cellSize - d);
        }
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

void ChessActivity::renderPieces() {
  for (int i = 0; i < 64; i++) {
    int piece = game.pieces[i];
    if (piece == Game::EMPTY) continue;

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

    XY square = squareXY(i);
    int px = square.x + (cellSize - tW) / 2;
    int py = square.y + (cellSize - tH) / 2 - tH / 3;

    renderer.drawText(font, px, py, pieceStr, true);
  }
}

void ChessActivity::renderMove() {
  if (move.from > -1) {
    XY square = squareXY(move.from);
    renderer.drawRect(square.x, square.y, cellSize, cellSize, 4, true);
  }

  if (move.to > -1) {
    XY square = squareXY(move.to);
    renderer.drawRoundedRect(square.x, square.y, cellSize, cellSize, 4, cellSize / 2, true);
  }
}

void ChessActivity::renderLastMove() {
  if (last.from < 0) return;

  XY from = squareXY(last.from);
  XY to = squareXY(last.to);
  renderer.drawRect(from.x, from.y, cellSize, cellSize, 2, true);
  renderer.drawRoundedRect(to.x, to.y, cellSize, cellSize, 2, cellSize / 2, true);
}

void ChessActivity::renderMoves() {
  int size = cellSize / 4;
  int s = 2;

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
  int y = boardY + boardSize + 15;
  renderer.drawCenteredText(UI_10_FONT_ID, y, infoText.c_str());
}

void ChessActivity::renderButtons() {
  const auto labels = mappedInput.mapLabels("Quit", "Mode", btnLeft.c_str(), btnRight.c_str());
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}