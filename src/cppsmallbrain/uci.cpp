#pragma once
#include <chrono>
#include <iostream>
#include <thread>
#include <optional>
#include <algorithm>
#include <atomic>

#include "board.h"
#include "search.h"
#include "uci.h"
#include "timecontroller.h"
#include "evaluation.h"
#include "thread_manager.h"


//  g++ -Ofast -march=native -mavx2 .\board.cpp .\uci.cpp .\search.cpp .\evaluation.cpp .\timecontroller.cpp -w
ThreadManager threads;
unsigned int time_given;
//Board board;
Board* board = new Board();

void output(std::string str) {
	std::cout << str;
}
int main() {
	//std::cout << "Commands: " << std::endl;
	//std::cout << "position fen ..." << std::endl;
	//std::cout << "go perft ..." << std::endl;
	//std::cout << "test perft" << std::endl;
	//std::cout << "speed test" << std::endl;
	std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	board->apply_fen(fen);
	std::thread searchThread;
	bool thread_started = false;
	while (true) {
		std::string input;
		std::getline(std::cin, input);
		if (input == "uci") {
			output("\nid name Smallbrain\n");
			output("id author Max, aka Disservin\n");
			output("uciok\n");
		}
		if (input == "isready") {
			output("readyok\n");
		}
		if (input == "stop") {
			threads.stop();
		}
		if (input.find("quit") != std::string::npos) {
			threads.stop();
			break;
		}
		if (input.find("position fen") != std::string::npos) {
			std::size_t start_index = input.find("fen");
			fen = input.substr(start_index + 4);
			board->apply_fen(fen);
			if (input.find("moves") != std::string::npos) {
				std::vector<std::string> param = split_input(input);
				std::size_t index = std::find(param.begin(), param.end(), "moves") - param.begin();
				index ++;
				for (index; index < param.size(); index++) {
					Move move = convert_uci_to_Move(param[index]);
					board->make_move(move);
				}
				memset(board->move_stack, 0, sizeof(board->move_stack));
				board->move_stack_index = 0;
			}
		}
		if (input.find("position startpos") != std::string::npos) {
			board->apply_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
			if (input.find("moves") != std::string::npos) {
				std::vector<std::string> param = split_input(input);
				std::size_t index = std::find(param.begin(), param.end(), "moves") - param.begin();
				index++;
				for (index; index < param.size(); index++) {
					Move move = convert_uci_to_Move(param[index]);
					board->make_move(move);
				}
				memset(board->move_stack, 0, sizeof(board->move_stack));
				board->move_stack_index = 0;
			}
		}
		if (input.find("go perft") != std::string::npos) {
			std::size_t start_index = input.find("perft");
			std::string depth_str = input.substr(start_index + 6);
			int depth = std::stoi(depth_str);
			Perft perft(board);
			auto begin = std::chrono::high_resolution_clock::now();
			U64 result = perft.speed_test_perft(depth, depth);
			auto end = std::chrono::high_resolution_clock::now();
			auto time_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
			std::cout << "startpos " << " nodes " << result << " nps " << result / (time_diff / 1000000000.0f) << " time " << time_diff / 1000000000.0f << " seconds" << std::endl;
		}
		if (input.find("test perft") != std::string::npos) {
			std::cout << "\nTest started" << std::endl;
			Perft perft(board);
			perft.test();
		}
		if (input.find("speed test") != std::string::npos) {
			std::cout << "\nTest started" << std::endl;
			int depth = 6;
			auto begin = std::chrono::high_resolution_clock::now();
			Perft perft(board);
			U64 x = perft.speed_test_perft(depth, depth);
			auto end = std::chrono::high_resolution_clock::now();
			auto time_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
			std::cout << "startpos " << " nodes " << x << " nps " << x / (time_diff / 1000000000.0f) << " time " << time_diff / 1000000000.0f << " seconds" << std::endl;
		}
		if (input.find("go depth") != std::string::npos and not thread_started) {
			std::size_t start_index = input.find("depth");
			std::string depth_str = input.substr(start_index + 6);
			int depth = std::stoi(depth_str);
			threads.begin(depth);
		}
		if (input == "go" or input == "go infinite" and not thread_started) {
			threads.begin(256);
		}
		if (input.find("go movetime") != std::string::npos) {
			std::size_t start_index = input.find("movetime");
			std::string movetime_str = input.substr(start_index + 6);
			int movetime = std::stoi(movetime_str);
			time_given = time_left(movetime);
			threads.begin(256);
		}
		if (input == "b") {
			board->print_board();
			std::cout << "Turn: " << board->side_to_move << std::endl;
		}
		if (input == "eval") {
			std::cout << evaluation()<<"\n";
		}
	}
}