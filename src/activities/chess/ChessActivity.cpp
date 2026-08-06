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

  int board[8][8] = {{-6, -5, -4, -3, -2, -4, -5, -6}, {-1, -1, -1, -1, -1, -1, -1, -1}, {0, 0, 0, 0, 0, 0, 0, 0},
                     {0, 0, 0, 0, 0, 0, 0, 0},         {0, 0, 0, 0, 0, 0, 0, 0},         {0, 0, 0, 0, 0, 0, 0, 0},
                     {1, 1, 1, 1, 1, 1, 1, 1},         {6, 5, 4, 3, 2, 4, 5, 6}};

  // Draw board squares and pieces
  for (int r = 0; r < BOARD; r++) {
    int off = CELL * r;
    int offY = boardY + off;
    int offX = boardX + off;

    renderer.drawLine(boardX, offY, right, offY);
    renderer.drawLine(offX, boardY, offX, bottom);

    for (int c = 0; c < BOARD; c++) {
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

      int piece = board[r][c];
      if (piece != 0) {
        const char* pieceStr = "";
        switch (piece) {
          case 1:  // White Pawn
            pieceStr = "P";
            break;
          case 2:  // White King
            pieceStr = "K";
            break;
          case 3:  // White Queen
            pieceStr = "Q";
            break;
          case 4:  // White Bishop
            pieceStr = "B";
            break;
          case 5:  // White Knight
            pieceStr = "N";
            break;
          case 6:  // White Rook
            pieceStr = "R";
            break;
          case -1:  // Black Pawn
            pieceStr = "p";
            break;
          case -2:  // Black King
            pieceStr = "k";
            break;
          case -3:  // Black Queen
            pieceStr = "q";
            break;
          case -4:  // Black Bishop
            pieceStr = "b";
            break;
          case -5:  // Black Knight
            pieceStr = "n";
            break;
          case -6:  // Black Rook
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