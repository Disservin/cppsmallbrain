# cppsmallbrain

Compile it like this 

    g++ -flto -O3 -march=native -mavx2 -std=c++17 -static-libgcc -static-libstdc++ -static -lpthread .\board.cpp .\uci.cpp .\search.cpp .\evaluation.cpp        .\timecontroller.cpp  -w -o master

## Elo 
       # PLAYER            : RATING  ERROR   POINTS  PLAYED    (%)
       1 Smallbrain        : 1737.5   61.4     79.5     100   79.5%
       2 Stockfish 1500    : 1500.0   ----     20.5     100   20.5%
TC = 60/0.6 for 40 moves <br>
Ordo -s 8000
