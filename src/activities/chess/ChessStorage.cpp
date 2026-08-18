

// string BASE_PATH = "./.config";

// if (!Storage.exists(BASE_PATH.c_str())) {
//   Storage.mkdir(BASE_PATH.c_str());
// }

// void ChessActivity::loadConfig() {
//   HalFile file = Storage.open("/.chess/config.json");
//   if (!file) return;

//   JsonDocument doc;
//   DeserializationError err = deserializeJson(doc, file);
//   file.close();
//   if (err) return;

//   if (doc["pieceSet"].is<string>()) {
//     config.pieceSet = doc["pieceSet"].as<string>();
//   }
// }

// void ChessActivity::loadMode() {
//   HalFile file = Storage.open("/.chess/mode.json");
//   if (!file) return;

//   JsonDocument doc;
//   DeserializationError err = deserializeJson(doc, file);
//   file.close();
//   if (err) return;

//   if (doc["mode"].is<string>()) {
//     string modeString = doc["mode"];
//     if (modeString == "puzzles") {
//       mode = PUZZLES;
//     } else if (modeString == "otb") {
//       mode = OTB;
//     } else if (modeString == "engine") {
//       mode = ENGINE;
//     }
//   }
//   if (doc["level"].is<int>()) {
//     level = doc["level"];
//   }
// }

// void ChessActivity::saveMode() {
//   auto file = Storage.open("/.chess/mode.json", O_WRITE | O_CREAT | O_TRUNC);
//   if (!file) return;

//   JsonDocument doc;

//   if (mode == OTB) {
//     doc["mode"] = "otb";
//   } else if (mode == PUZZLES) {
//     doc["mode"] = "puzzles";
//   } else if (mode == ENGINE) {
//     doc["mode"] = "engine";
//   }
//   doc["level"] = level;

//   serializeJson(doc, file);
//   file.close();
// }

// void ChessActivity::savePosition() {
//   auto file = Storage.open("/.chess/position.fen", O_WRITE | O_CREAT | O_TRUNC);
//   if (!file) return;

//   string position = game.fen();
//   file.write(position.c_str(), position.size());
//   file.close();
// }

// string ChessActivity::loadPosition() {
//   HalFile file = Storage.open("/.chess/position.fen");
//   if (!file) return Game::STARTPOS;

//   char buffer[file.size()];
//   file.read(buffer, file.size());
//   file.close();

//   return string(buffer);
// }

// int ChessActivity::loadPuzzleIndex() {
//   HalFile file = Storage.open("/.chess/puzzle_index.json");
//   if (!file) return 0;

//   JsonDocument doc;
//   DeserializationError err = deserializeJson(doc, file);
//   file.close();
//   if (err) return 0;

//   string difficulty = puzzleDifficulty[level];
//   if (!doc[difficulty].is<int>()) return 0;
//   return doc[difficulty];
// }

// void ChessActivity::savePuzzleIndex(int index) {
//   HalFile reading = Storage.open("/.chess/puzzle_index.json");
//   if (!reading) return;

//   JsonDocument doc;
//   DeserializationError readingErr = deserializeJson(doc, reading);
//   reading.close();
//   if (readingErr) return;

//   HalFile file = Storage.open("/.chess/puzzle_index.json", O_WRITE | O_CREAT | O_TRUNC);
//   if (!file) return;

//   string difficulty = puzzleDifficulty[level];
//   doc[difficulty] = index;
//   serializeJson(doc, file);

//   file.close();
// }