#include "ChessActivity.h"

#include <Arduino.h>
#include <GfxRenderer.h>
#include <I18n.h>
#include <Logging.h>
#include <ctype.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <string>

#include "components/UITheme.h"
#include "fontIds.h"

using namespace std;

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

void ChessActivity::onEnter() {
  Activity::onEnter();

  game.restore(loadPosition());
  copyPieces();

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

  } else if (mappedInput.wasReleased(MappedInputManager::Button::Left)) {
    game.undo();
    copyPieces();
    last = Move{};
    state = SELECT_PIECE;
    selected = 0;
    requestUpdate();

  } else if (mappedInput.wasReleased(MappedInputManager::Button::Right)) {
    game.restore(Game::STARTPOS);
    copyPieces();
    last = Move{};
    state = SELECT_PIECE;
    selected = 0;
    requestUpdate();

  } else if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    mode = (mode + 1) % 3;

    requestUpdate();

  } else if (mappedInput.wasReleased(MappedInputManager::Button::Down)) {
    selected++;
    if (state == SELECT_PIECE) {
      selected %= mine.size();
    } else if (state == SELECT_MOVE) {
      selected %= moves.size();
    }

    requestUpdate();

  } else if (mappedInput.wasReleased(MappedInputManager::Button::Up)) {
    if (state == SELECT_PIECE) {
      if (!mine.size()) return;

      selected_piece = selected;
      auto from = mine[selected_piece];
      auto legals = game.moves(from);
      sort(legals.begin(), legals.end(), [](Move a, Move b) { return a.to < b.to; });

      moves = {Move{from, from}};
      moves.insert(moves.end(), legals.begin(), legals.end());

      state = SELECT_MOVE;
      selected = 0;

    } else if (state == SELECT_MOVE) {
      if (selected == 0) {
        state = SELECT_PIECE;
        selected = selected_piece;

      } else {
        last = moves[selected];

        game.make(last);
        savePosition();
        copyPieces();

        over = game.over();
        if (over) {
          state = GAME_OVER;
        } else if (mode == OTB) {
          state = SELECT_PIECE;
          selected = 0;
        } else {
          state = THINKING;
        }
      }
    }

    requestUpdate();

  } else if (state == THINKING) {
    // if (mode == COMPUTER) {
    //   last = engine.respond();
    // } else {
    //   auto theirs = engine.myPieces(engine.sideToMove());

    //   vector<Move> legals;
    //   while (legals.size() == 0) {
    //     auto piece = theirs[rand() % theirs.size()];

    //     legals = engine.legalMoves(piece);
    //   }

    //   last = legals[rand() % legals.size()];

    //   engine.makeMove(last);
    // }
    // savePosition();

    // copyPieces();

    state = SELECT_PIECE;
    selected = 0;

    requestUpdate();
  }
}

void ChessActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();
  const int CELL = (pageWidth - 10) / BOARD;

  renderer.clearScreen();

  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, "Chess");

  const int boardSize = CELL * BOARD;
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
    selected_square = mine[selected];

  } else if (state == SELECT_MOVE) {
    piece_square = mine[selected_piece];
    selected_square = moves[selected].to;

    for (int m = 1; m < moves.size(); m++) {
      int to = moves[m].to;
      int size = CELL / 5;
      int cx = boardX + (to % 8) * CELL + (CELL - size) / 2;
      int cy = boardY + (to / 8) * CELL + (CELL - size) / 2;
      renderer.fillRoundedRect(cx, cy, size, size, size / 2, Color::Black);
    }
  }

  // Draw board squares and pieces
  for (int r = 0; r < BOARD; r++) {
    int off = CELL * r;
    int offY = boardY + off;
    int offX = boardX + off;

    for (int c = 0; c < BOARD; c++) {
      int i = (r * 8) + c;
      int cx = boardX + c * CELL;
      int cy = boardY + r * CELL;
      bool darkSquare = (r + c) % 2 == 1;

      if (darkSquare) {
        int h = CELL / 4;
        for (int d = h; d <= CELL; d += h) {
          renderer.drawLine(cx + d, cy, cx, cy + d);
        }
        for (int d = h; d < CELL - 1; d += h) {
          renderer.drawLine(cx + CELL - d, cy + CELL, cx + CELL, cy + CELL - d);
        }
      }

      if (selected_square == i) {
        renderer.drawRect(cx, cy, CELL, CELL, 3, true);
      }

      if (piece_square == i) {
        renderer.drawRoundedRect(cx, cy, CELL, CELL, 3, CELL / 2, true);
      }

      if (last.from == i || last.to == i) {
        renderer.drawRect(cx, cy, CELL, CELL, 2, true);
      }

      int piece = pieces[i];
      if (piece != Game::EMPTY) {
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
        int px = cx + (CELL - tW) / 2;
        int py = cy + (CELL - tH) / 2;
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
  if (state == SELECT_PIECE) {
    renderer.drawCenteredText(SMALL_FONT_ID, statusY, "Select this piece");
  } else if (state == SELECT_MOVE) {
    if (selected == 0) {
      renderer.drawCenteredText(SMALL_FONT_ID, statusY, "Cancel");
    } else {
      renderer.drawCenteredText(SMALL_FONT_ID, statusY, Game::print(moves[selected]).c_str());
    }
  } else if (state == THINKING) {
    renderer.drawCenteredText(SMALL_FONT_ID, statusY, "Thinking...");
  } else if (state == GAME_OVER) {
    if (over == Game::CHECKMATE) {
      renderer.drawCenteredText(SMALL_FONT_ID, statusY, "Checkmate!");
    } else {
      renderer.drawCenteredText(SMALL_FONT_ID, statusY, "Stalemate =|");
    }
  }

  // Button hints
  const string btn1 = "Quit";
  string btn2 = "";
  if (mode == COMPUTER) {
    btn2 = "AI";
  } else if (mode == OTB) {
    btn2 = "OTB";
  } else {
    btn2 = "RND";
  }
  const string btn3 = "Undo";
  const string btn4 = "New";

  const auto labels = mappedInput.mapLabels(btn1.c_str(), btn2.c_str(), btn3.c_str(), btn4.c_str());
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}

void ChessActivity::savePosition() {
  auto file = Storage.open("/chess_position.txt", O_WRITE | O_CREAT | O_TRUNC);
  if (file) {
    string position = game.fen();
    file.write(position.c_str(), position.size());
    file.close();
    LOG_INF("CHESS", "Wrote position: %s", position.c_str());
  } else {
    LOG_ERR("CHESS", "Failed to open chess_position.txt for writing");
  }
}

string ChessActivity::loadPosition() {
  HalFile file = Storage.open("/chess_position.txt");
  if (file) {
    char buffer[file.size()];
    file.read(buffer, file.size());
    LOG_INF("CHESS", "Read position: %s", buffer);
    return string(buffer);
  } else {
    LOG_ERR("CHESS", "Failed to open chess_position.txt for reading");
    return Game::STARTPOS;
  }
}