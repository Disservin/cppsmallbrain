# cppsmallbrain

Compile it like this 

    g++ -flto -O3 -march=native -mavx2 -std=c++17 -static-libgcc -static-libstdc++ -static -lpthread .\board.cpp .\uci.cpp .\search.cpp .\evaluation.cpp        .\timecontroller.cpp  -w -o master

or just use the make file in ./src

## Elo 
       # PLAYER            : RATING  ERROR   POINTS  PLAYED    (%)
       1 Smallbrain        : 1730.3   50.1    113.5     144   78.8%
       2 Stockfish 1500    : 1500.0   ----     30.5     144   21.2%
TC = 60/0.6<br>
Ordo -s 8000
