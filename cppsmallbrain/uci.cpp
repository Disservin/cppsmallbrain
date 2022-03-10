#pragma once
#include <chrono>
#include <iostream>
#include "board.h"

// g++ -Ofast -march=native -mavx2 .\board.cpp .\uci.cpp -w
// g++ -Ofast -march=native -m64 .\board.cpp .\uci.cpp 
int main() {
	board board;
	std::cout << "Commands: " << std::endl;
	std::cout << "position fen ..." << std::endl;
	std::cout << "go perft ..." << std::endl;
	std::cout << "test perft" << std::endl;
	std::cout << "speed test" << std::endl;
	std::string fen = "rnbq1bnr/ppppkpRp/4p3/8/8/1P6/P1PPPPPP/RN1QKBNR w KQ - 0 1";
	board.apply_fen(fen);
	while (true) {
		std::string input;
		std::cout << "\n";
		std::getline(std::cin, input);
		if (input.find("position fen") != std::string::npos) {
			std::size_t start_index = input.find("fen");
			fen = input.substr(start_index + 4);
			board.apply_fen(fen);
		}
		if (input.find("go perft") != std::string::npos) {
			std::size_t start_index = input.find("perft");
			std::string depth_str = input.substr(start_index + 6);
			int depth = std::stoi(depth_str);
			auto begin = std::chrono::high_resolution_clock::now();
			Perft perft(board);
			U64 x = perft.bulk_perft(depth, depth);
			auto end = std::chrono::high_resolution_clock::now();
			auto time_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
			std::cout << "fen " << fen << " nodes " << x << " nps " << x / (time_diff / 1000000000.0f) << " time " << time_diff / 1000000000.0f << " seconds" << std::endl;
		}
		if (input.find("test perft") != std::string::npos) {
			std::cout << "\nTest started" << std::endl;
			test();
		}
		if (input.find("speed test") != std::string::npos) {
			std::cout << "\nTest started" << std::endl;
			int depth = 5;
			auto begin = std::chrono::high_resolution_clock::now();
			Perft perft(board);
			U64 x = perft.speed_test_perft(depth);
			auto end = std::chrono::high_resolution_clock::now();
			auto time_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
			std::cout << "startpos " << " nodes " << x << " nps " << x / (time_diff / 1000000000.0f) << " time " << time_diff / 1000000000.0f << " seconds" << std::endl;
		}
	}
}