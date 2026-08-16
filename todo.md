# Changes for Chess

Goal: Play chess against the computer with variable strength using only the side buttons.

Flash: pio run --target upload
Debug: python scripts/debugging_monitor.py COM11

## Current


## Planned

- Show puzzle hint and next move

- Change mode and level
  - OTB
  - Puzzles

- Solve daily puzzle

- Engine level random


## Ideas

- Build 32 pt emoji font (https://github.com/zakerytclarke/crosspoint-reader-apps/blob/c623c224da1d1a0f8a1b24228408f05026f76166/lib/EpdFont/scripts/convert-built)

- Add letter piece set (black and white)

- Say if solved without undos

- Redraw before puzzle response

- Use abstract piece symbols (via config)

- Lichess authentication


## Completed

- Flip board when playing black

- Change puzzle difficulty with config

- Put all files in .chess

- Labels for side buttons

- Menu item icon

- Keep selection at moves piece

- Use emoji piece symbols

- Solve puzzles

- Game over
  - Print if checkmate or stalemate
  - Allow no more moves

- Undo last move

- Use millipawn as engine
  - include as submodule

- Highlight own last move
  - to support OTB play passing back and forth

- Resume game
  - save FEN after every move
  - load FEN onEnter
  - start new game with button

- Promotion
  - Print selected move

- Switch between AI and OTB mode
  - Toggle using Confirm button

- Avoid illegal moves
  - Check legality of each generated move

- Apply engine move
  - Tell engine to make move
  - Print new board when done
  - Back to selecting own piece
  - Highlight from and to squares of last move

- Make move
  - Show legal moves
  - Move selector with right button
  - Restrict selector to legal target squares
  - Sort legal moves by to square
  - Cancel by selecting piece again
  - Make move with left button
  - Print whose turn it is

- Select piece
  - Move selector with right button
  - Restrict selector to own pieces
  - Select piece with left button
  - Unselect again with left button

- Draw pieces
  - Use Letter pieces
  - Query engine for board state
  - Use TinyChess as engine

- Draw board
  - Full width
  - Dark and light grey

- Add Chess Activity to the Menu
  - Add Menu entry
  - Open Activity
  - Quit Chess and go back Home