# cppsmallbrain

Compile it like this 

    g++ -flto -O3 -march=native -mavx2 .\board.cpp .\uci.cpp .\search.cpp .\evaluation.cpp .\timecontroller.cpp -std=c++17 -lpthread -w
