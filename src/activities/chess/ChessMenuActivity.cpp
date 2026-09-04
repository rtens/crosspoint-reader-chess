#include "./ChessMenuActivity.h"

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

#include "ChessStorage.h"
#include "activities/ActivityResult.h"
#include "activities/network/WifiSelectionActivity.h"
#include "components/UITheme.h"
#include "fontIds.h"
#include "network/HttpDownloader.h"

void ChessMenuActivity::onEnter() {
  Activity::onEnter();
  readPieceSets();
  state = SELECT_ITEM;
  selectedItem = 0;
  selectedOption = 0;
  requestUpdate();
}

void ChessMenuActivity::readPieceSets() {
  ChessStorage storage;
  for (String s : storage.listPieceSets()) {
    get<2>(items[PIECE_SET]).push_back(string(s.substring(0, s.length() - 4).c_str()));
  }
}

void ChessMenuActivity::onExit() { Activity::onExit(); }

void ChessMenuActivity::loop() {
  auto options = get<2>(items[selectedItem]);

  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    ActivityResult result;
    result.isCancelled = true;
    setResult(std::move(result));
    finish();

  } else if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    LOG_DBG("CHESS", "Confirm state %i item %i option %i", state, selectedItem, selectedOption);

    if (state == SELECT_OPTION || !options.size()) {
      string option = "";
      if (selectedOption < options.size()) {
        option = options[selectedOption];
      }

      setResult(ChessMenuResult{get<0>(items[selectedItem]), option});
      finish();

    } else {
      state = SELECT_OPTION;
      requestUpdate();
    }

  } else if (mappedInput.wasReleased(MappedInputManager::Button::Left) ||
             mappedInput.wasReleased(MappedInputManager::Button::Up)) {
    if (state == SELECT_ITEM) {
      selectedItem += items.size() - 1;
      selectedItem %= items.size();
    }
    if (state == SELECT_OPTION) {
      selectedOption += options.size() - 1;
      selectedOption %= options.size();
    }
    requestUpdate();

  } else if (mappedInput.wasReleased(MappedInputManager::Button::Right) ||
             mappedInput.wasReleased(MappedInputManager::Button::Down)) {
    if (state == SELECT_ITEM) {
      selectedItem++;
      selectedItem %= items.size();
    }
    if (state == SELECT_OPTION) {
      selectedOption++;
      selectedOption %= options.size();
    }
    requestUpdate();
  }
}

void ChessMenuActivity::render(RenderLock&&) {
  renderer.clearScreen();
  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, "Chess");

  int centerY = pageHeight / 2;
  int selectionX = pageWidth / 3;

  int selected = selectedItem;
  vector<string> selection;

  if (state == SELECT_ITEM) {
    for (auto item : items) {
      selection.push_back(get<1>(item));
    }

  } else if (state == SELECT_OPTION) {
    selected = selectedOption;
    for (auto option : get<2>(items[selectedItem])) {
      selection.push_back(option);
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