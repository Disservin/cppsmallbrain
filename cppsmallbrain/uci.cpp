#pragma once
#include <chrono>
#include <iostream>
#include "board.h"

int main() {
	board board;
	while (true) {
		std::string input;
		std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
		board.apply_fen(fen);
		std::getline(std::cin, input);
		if (input.find("position fen") != std::string::npos) {
			std::size_t start_index = input.find("fen");
			std::string fen = input.substr(start_index + 4);
			board.apply_fen(fen);
		}
		if (input.find("go perft") != std::string::npos) {
			std::size_t start_index = input.find("perft");
			std::string depth_str = input.substr(start_index + 6);
			int depth = std::stoi(depth_str);
			auto begin = std::chrono::high_resolution_clock::now();
			U64 x = perft(board, depth, depth);
			auto end = std::chrono::high_resolution_clock::now();
			auto time_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
			std::cout << "fen " << fen << " nodes " << x << " nps " << x / (time_diff / 1000000000.0f) << " time " << time_diff / 1000000000.0f << " seconds" << std::endl;
		}
		if (input.find("test perft") != std::string::npos) {
			std::cout << "Test started" << std::endl;
			test();
		}
	}
}