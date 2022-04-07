# cppsmallbrain

## Compile

Compile it using the Makefile in ./src <br>
```
make
.\smallbrain.exe bench
```
compare the Bench with the Bench in the commit messages, they should be <br>
the same, unless I screwed up.

## Elo 
       # PLAYER            : RATING  ERROR   POINTS  PLAYED    (%)
       1 Smallbrain        : 1791.3   49.1    155.5     185   84.1%
       2 Stockfish 1500    : 1500.0   ----     29.5     185   15.9%
Using commit `897d77b81ce90d6f91620f65d038450b88f7d4f1` <br>
TC = 40/60+0.6<br>
Calculated with ordo -s 8000
