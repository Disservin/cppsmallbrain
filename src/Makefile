
smallbrain: board.o uci.o search.o evaluation.o timecontroller.o thread_manager.o
	g++ -flto -O3 -march=native -std=c++17 -static-libgcc -static-libstdc++ -static -lpthread board.o uci.o search.o evaluation.o timecontroller.o thread_manager.o -o smallbrain

board.o: board.cpp board.h
	g++ -c -flto -O3 -march=native -std=c++17 -static-libgcc -static-libstdc++ -static -lpthread board.cpp

uci.o: uci.cpp uci.h
	g++ -c -flto -O3 -march=native -std=c++17 -static-libgcc -static-libstdc++ -static -lpthread uci.cpp

search.o: search.cpp search.h
	g++ -c -flto -O3 -march=native -std=c++17 -static-libgcc -static-libstdc++ -static -lpthread search.cpp

evaluation.o: evaluation.cpp evaluation.h
	g++ -c -flto -O3 -march=native -std=c++17 -static-libgcc -static-libstdc++ -static -lpthread evaluation.cpp

thread_manager.o: thread_manager.cpp thread_manager.h
	g++ -c -flto -O3 -march=native -std=c++17 -static-libgcc -static-libstdc++ -static -lpthread thread_manager.cpp

clean:
	rm *.o smallbrain