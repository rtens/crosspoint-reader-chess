#include "ChessActivity.h"

#include <Arduino.h>
#include <GfxRenderer.h>
#include <I18n.h>

#include <cstdlib>
#include <cstring>

#include "components/UITheme.h"
#include "fontIds.h"

void ChessActivity::onEnter() {
  Activity::onEnter();

  engine.newGame();

  savedOrientation = renderer.getOrientation();
  renderer.setOrientation(GfxRenderer::Orientation::Portrait);
  requestUpdate();
}

void ChessActivity::onExit() {
  Activity::onExit();
  // Restore orientation so the next activity (reader) starts clean
  renderer.setOrientation(savedOrientation);
}

void ChessActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    onGoHome();
    return;
  }

  bool changed = false;

  if (mappedInput.wasReleased(MappedInputManager::Button::Down)) {
    selected++;
    changed = true;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Up)) {
    if (state == SELECT_PIECE) {
      state = SELECT_MOVE;
      selected_piece = selected;
      selected = 0;
    } else if (state == SELECT_MOVE) {
      selected %= 64;
      // if (selected == 0) {
      state = SELECT_PIECE;
      selected = selected_piece;
      // }
    }
    changed = true;
  }

  if (changed) requestUpdate();
}

void ChessActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();
  const int CELL = pageWidth / BOARD;

  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, "Chess");

  const int boardSize = CELL * BOARD;
  const int boardX = (pageWidth - boardSize) / 2;
  const int contentTop = metrics.topPadding + metrics.headerHeight + 4;
  const int contentBot = pageHeight - metrics.buttonHintsHeight - 4;
  const int boardY = contentTop + (contentBot - contentTop - boardSize - 24) / 2;
  const int right = boardX + boardSize;
  const int bottom = boardY + boardSize;

  auto pieces = engine.pieces();
  auto mine = engine.myPieces();

  int selected_square = -1;
  int piece_square = -1;

  if (state == SELECT_PIECE) {
    selected %= mine.size();
    selected_square = mine[selected];
  } else if (state == SELECT_MOVE) {
    selected %= 64;
    selected_square = selected;
    piece_square = mine[selected_piece];
  }

  // Draw board squares and pieces
  int i = -1;
  for (int r = 0; r < BOARD; r++) {
    int off = CELL * r;
    int offY = boardY + off;
    int offX = boardX + off;

    renderer.drawLine(boardX, offY, right, offY);
    renderer.drawLine(offX, boardY, offX, bottom);

    for (int c = 0; c < BOARD; c++) {
      i++;
      int cx = boardX + c * CELL;
      int cy = boardY + r * CELL;
      bool darkSquare = (r + c) % 2 == 1;

      if (darkSquare) {
        for (int d = CELL / 5; d < CELL; d += CELL / 5) {
          renderer.drawLine(cx + d, cy, cx, cy + d);
        }
        for (int d = CELL / 5; d < CELL - 1; d += CELL / 5) {
          renderer.drawLine(cx + CELL - d, cy + CELL, cx + CELL, cy + CELL - d);
        }
      }

      if (selected_square == i) {
        renderer.drawRect(cx, cy, CELL, CELL, 3, true);
      }

      if (piece_square == i) {
        renderer.drawRoundedRect(cx, cy, CELL, CELL, 3, CELL / 2, true);
      }

      int piece = pieces[i];
      if (piece != 0) {
        const char* pieceStr = "";
        switch (piece) {
          case ChessEngine::WHITE_PAWN:
            pieceStr = "P";
            break;
          case ChessEngine::WHITE_KING:
            pieceStr = "K";
            break;
          case ChessEngine::WHITE_QUEEN:
            pieceStr = "Q";
            break;
          case ChessEngine::WHITE_BISHOP:
            pieceStr = "B";
            break;
          case ChessEngine::WHITE_KNIGHT:
            pieceStr = "N";
            break;
          case ChessEngine::WHITE_ROOK:
            pieceStr = "R";
            break;
          case ChessEngine::BLACK_PAWN:
            pieceStr = "p";
            break;
          case ChessEngine::BLACK_KING:
            pieceStr = "k";
            break;
          case ChessEngine::BLACK_QUEEN:
            pieceStr = "q";
            break;
          case ChessEngine::BLACK_BISHOP:
            pieceStr = "b";
            break;
          case ChessEngine::BLACK_KNIGHT:
            pieceStr = "n";
            break;
          case ChessEngine::BLACK_ROOK:
            pieceStr = "r";
            break;
        }

        int tW = renderer.getTextWidth(NOTOSANS_16_FONT_ID, pieceStr);
        int tH = renderer.getTextHeight(NOTOSANS_16_FONT_ID);
        int px = cx + (CELL - tW) / 2;
        int py = cy + (CELL - tH) / 2;
        renderer.drawText(NOTOSANS_16_FONT_ID, px, py, pieceStr, true);
      }
    }
  }

  renderer.drawLine(boardX, bottom, right, bottom);
  renderer.drawLine(right, boardY, right, bottom);

  // Button hints
  const char* btn1 = "Quit";
  const char* btn2 = "";
  const char* btn3 = "";
  const char* btn4 = "";

  const auto labels = mappedInput.mapLabels(btn1, btn2, btn3, btn4);
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}