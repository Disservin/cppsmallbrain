smallbrain: chess.o uci.o search.o evaluation.o threadmanager.o
	g++ -flto -O3 -march=native -std=c++20 -static-libgcc -static-libstdc++ -static -lpthread chess.o uci.o search.o evaluation.o threadmanager.o -o smallbrain

chess.o: chess.cpp chess.h
	g++ -c -flto -O3 -march=native -std=c++20 -static-libgcc -static-libstdc++ -static -lpthread chess.cpp

uci.o: uci.cpp uci.h
	g++ -c -flto -O3 -march=native -std=c++20 -static-libgcc -static-libstdc++ -static -lpthread uci.cpp

search.o: search.cpp search.h
	g++ -c -flto -O3 -march=native -std=c++20 -static-libgcc -static-libstdc++ -static -lpthread search.cpp

evaluation.o: evaluation.cpp evaluation.h
	g++ -c -flto -O3 -march=native -std=c++20 -static-libgcc -static-libstdc++ -static -lpthread evaluation.cpp

threadmanager.o: threadmanager.cpp threadmanager.h
	g++ -c -flto -O3 -march=native -std=c++20 -static-libgcc -static-libstdc++ -static -lpthread threadmanager.cpp

clean:
	rm *.o smallbrain