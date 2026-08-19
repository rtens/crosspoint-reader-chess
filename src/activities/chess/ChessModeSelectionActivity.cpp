#include "./ChessModeSelectionActivity.h"

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
#include <map>
#include <sstream>
#include <string>
using namespace std;

#include "activities/network/WifiSelectionActivity.h"
#include "components/UITheme.h"
#include "fontIds.h"
#include "network/HttpDownloader.h"

vector<vector<string>> modes = {{"Solve Puzzles", "normal", "easier", "harder", "easiest", "hardest"},
                                {"Solve Daily Puzzle"},
                                {"Play vs Engine", "random"},
                                {"Play vs Friend"}};

void ChessModeSelectionActivity::onEnter() {
  Activity::onEnter();
  state = SELECT_MODE;
  selectedMode = 0;
  selectedLevel = 0;
  requestUpdate();
}

void ChessModeSelectionActivity::onExit() { Activity::onExit(); }

void ChessModeSelectionActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    finish();
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    LOG_DBG("CHESS", "Confirm state %i mode %i level %i", state, selectedMode, selectedLevel);
    if (modes[selectedMode].size() == 1 || state == SELECT_LEVEL) {
      finish();
      activityManager.requestUpdateAndWait();
      string level = "";
      if (selectedLevel + 1 < modes[selectedMode].size()) {
        level = modes[selectedMode][selectedLevel + 1];
      }
      onModeSelected(ChessMode{selectedMode, level});
    } else {
      state = SELECT_LEVEL;
      requestUpdate();
    }
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Left) ||
      mappedInput.wasReleased(MappedInputManager::Button::Up)) {
    if (state == SELECT_MODE && selectedMode > 0) {
      selectedMode--;
      requestUpdate();
    }
    if (state == SELECT_LEVEL && selectedLevel > 0) {
      selectedLevel--;
      requestUpdate();
    }
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Right) ||
      mappedInput.wasReleased(MappedInputManager::Button::Down)) {
    if (state == SELECT_MODE && selectedMode < modes.size() - 1) {
      selectedMode++;
      requestUpdate();
    }
    if (state == SELECT_LEVEL && selectedLevel < modes[selectedMode].size() - 2) {
      selectedLevel++;
      requestUpdate();
    }
    return;
  }
}

void ChessModeSelectionActivity::render(RenderLock&&) {
  renderer.clearScreen();
  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, "Chess");

  int centerY = pageHeight / 2;
  int selectionX = pageWidth / 3;

  string label = "Mode:";
  int selected = selectedMode;
  vector<string> selection;

  if (state == SELECT_MODE) {
    for (auto mode : modes) {
      selection.push_back(mode[0]);
    }
  } else if (state == SELECT_LEVEL) {
    label = "Level:";
    selected = selectedLevel;
    for (int i = 1; i < modes[selectedMode].size(); i++) {
      selection.push_back(modes[selectedMode][i]);
    }
  }

  renderer.drawCenteredText(NOTOSANS_18_FONT_ID, centerY, selection[selected].c_str());

  int cy = centerY;
  for (int i = selected - 1; i >= 0; i--) {
    cy -= 50;
    renderer.drawCenteredText(NOTOSANS_12_FONT_ID, cy, selection[i].c_str());
  }
  cy = centerY + 70;
  for (int i = selected + 1; i < selection.size(); i++) {
    renderer.drawCenteredText(NOTOSANS_12_FONT_ID, cy, selection[i].c_str());
    cy += 50;
  }

  GUI.drawButtonHints(renderer, "Back", "Select", "Up", "Down");

  renderer.displayBuffer();
}