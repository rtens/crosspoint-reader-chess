# Tasks for the *CrossPoint Reader Chess* Project

## Current


## Planned

#### Remember daily puzzle
- show if already solved
- download next on button press


## Ideas

#### Download piece sets
- from github maybe

#### Undo vs engine
- allow via config

#### Win puzzles
- if solved without undos

#### Lichess authentication
- [usinga personal token](https://lichess.org/api#description/authentication)

#### Update puzzle rating
- [post won puzzles](https://lichess.org/api#tag/puzzles/POST/api/puzzle/batch/{angle})

#### PGN move notation in status text

#### Port to CrossInk
- [requested](https://www.reddit.com/r/xteinkereader/comments/1vr5o7t/comment/p4ghnvm/)


## Completed

#### Show last move when puzzle starts

#### Nice piece sets
- load from file

#### Refresh display after number of moves

#### Redraw before puzzle response

#### Play against engine
- with random moves by engine
- start with random color
- restart the game
- persist position

#### Solve daily puzzle

#### Show puzzle hint and next move

#### Add button to reset puzzle

#### Add button to go to previous puzzle

#### Add more info why puzzle download failed
- [see comment](https://www.reddit.com/r/xteinkereader/comments/1vr5o7t/comment/p4hyavx/)

#### Fix hatching alignment
- [reported](https://www.reddit.com/r/xteinkereader/comments/1vr5o7t/comment/p4hyavx/)

#### Solve puzzle start crashes
- Probably caused by memory issues
- Replaced all ints of millipawn to uint8s

#### Play over the board
- Use state machine for OTB and Puzzle Mix

#### Change mode and level

#### Build 32pt emoji font

#### Flip board when playing black

#### Change puzzle difficulty with config

#### Put all files in .chess

#### Labels for side buttons

#### Menu item icon

#### Keep selection at moves piece

#### Use emoji piece symbols

#### Solve puzzles

#### Game over
- Print if checkmate or stalemate
- Allow no more moves

#### Undo last move

#### Use millipawn as engine
- include as submodule

#### Highlight own last move
- to support OTB play passing back and forth

#### Resume game
- save FEN after every move
- load FEN onEnter
- start new game with button

#### Promotion
- Print selected move

#### Switch between AI and OTB mode
- Toggle using Confirm button

#### Avoid illegal moves
- Check legality of each generated move

#### Apply engine move
- Tell engine to make move
- Print new board when done
- Back to selecting own piece
- Highlight from and to squares of last move

#### Make move
- Show legal moves
- Move selector with right button
- Restrict selector to legal target squares
- Sort legal moves by to square
- Cancel by selecting piece again
- Make move with left button
- Print whose turn it is

#### Select piece
- Move selector with right button
- Restrict selector to own pieces
- Select piece with left button
- Unselect again with left button

#### Draw pieces
- Use Letter pieces
- Query engine for board state
- Use TinyChess as engine

#### Draw board
- Full width
- Dark and light grey

#### Add Chess Activity to the Menu
- Add Menu entry
- Open Activity
- Quit Chess and go back Home
