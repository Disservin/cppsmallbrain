# cppsmallbrain

## Compile

Compile it using the Makefile in ./src or use the VS Solution<br>
```
make
.\smallbrain.exe bench
```
compare the Bench with the Bench in the commit messages, they should be <br>
the same, unless I screwed up.

## Features
* Engine
  * Bitboard representation
  * Zobrist hashing
* Search
  * Negamax framework
  * Aspiration Window
  * Alpha-Beta pruning fail-soft
  * Quiescence search
  * Repetition detection
  * Killer moves
  * History heuristic
  * MVV-LVA move ordering
  * Null-move pruning
  * Razoring
  * Late-move reduction
* Evaluation
  * Material evaluation
  * Piece square table
  * Open Rook
  * King safety 

## Elo 
       # PLAYER            : RATING  ERROR   POINTS  PLAYED    (%)
       1 Smallbrain        : 1847.2   52.4    351.5     400   87.9%
       2 Stockfish 1500    : 1500.0   ----     48.5     400   12.1%

       White advantage = -6.17 +/- 26.87
       Draw rate (equal opponents) = 1.94 % +/- 0.87
       
Using commit `2489526408811d25ccfadd26dccbc898144ab92d` <br>
TC = 40/60+0.6<br>
Calculated with ordo -s 8000
