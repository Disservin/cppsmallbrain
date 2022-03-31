#pragma once
#include <chrono>
#include <iostream>
#include <thread>
#include <optional>
#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <signal.h>

#include "board.h"
#include "search.h"
#include "uci.h"
#include "timecontroller.h"
#include "evaluation.h"
#include "thread_manager.h"
#include "tt.h"


ThreadManager threads;
Board* board = new Board();
U64 tt_size = 4294967*2;
TEntry* TTable = (TEntry*)malloc(tt_size * sizeof(TEntry));	//TEntry == 48 Bytes n = HASH_SIZE / (48/1000000) 

void signal_callback_handler(int signum) {
	threads.stop();
	free(TTable);
	delete[] board;
	exit(signum);
}

void output(std::string str) {
	std::cout << str;
}

template<typename K, typename V>
void print_map(std::unordered_map<K, V> const& m)
{
	for (auto const& pair : m) {
		std::cout << "{" << pair.first << ": " << pair.second << "}\n";
	}
}

int main() {
	// Commands
	// test perft
	// speedtest
	std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	board->apply_fen(fen);
	std::thread searchThread;
	bool thread_started = false;
	signal(SIGINT, signal_callback_handler);

	while (true) {
		std::string input;
		std::getline(std::cin, input);
		if (input == "uci") {
			output("\nid name Smallbrain\n");
			output("id author Max, aka Disservin\n");
			output("\noption name Hash type spin default 400 min 1 max 1000000\n");
			output("uciok\n");
		}
		if (input == "isready") {
			output("readyok\n");
		}
		if (input == "stop") {
			threads.stop();
		}
		if (input.find("setoption name Hash value") != std::string::npos) {
			std::size_t start_index = input.find("value");
			std::string size_str = input.substr(start_index + 6);
			U64 elements = (static_cast<unsigned long long>(std::stoi(size_str)) * 1000000)/48;
			TTable = (TEntry*)realloc(TTable, elements*48);
			tt_size = elements;
		}
		if (input == "ucinewgame") {
			memset(TTable, 0, tt_size * 48);
			board->apply_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		}
		if (input.find("quit") != std::string::npos) {
			threads.stop();
			free(TTable);
			delete[] board;
			break;
		}
		if (input.find("position fen") != std::string::npos) {
			std::size_t start_index = input.find("fen");
			fen = input.substr(start_index + 4);
			board->apply_fen(fen);
			board->repetition_table.clear();
			if (input.find("moves") != std::string::npos) {
				std::vector<std::string> param = split_input(input);
				std::size_t index = std::find(param.begin(), param.end(), "moves") - param.begin();
				index ++;
				for ( ; index < param.size(); index++) {
					Move move = convert_uci_to_Move(param[index]);
					board->make_move(move);
				}
			}
			while (!board->move_stack.empty()) {
				board->move_stack.pop();
			}
		}
		if (input.find("position startpos") != std::string::npos) {
			board->apply_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
			board->repetition_table.clear();
			if (input.find("moves") != std::string::npos) {
				std::vector<std::string> param = split_input(input);
				std::size_t index = std::find(param.begin(), param.end(), "moves") - param.begin();
				index++;
				for ( ; index < param.size(); index++) {
					Move move = convert_uci_to_Move(param[index]);
					board->make_move(move);				
				}
			}
			while (!board->move_stack.empty()) {
				board->move_stack.pop();
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
		if (input.find("speedtest") != std::string::npos) {
			std::cout << "\nTest started" << std::endl;
			int depth = 6;
			auto begin = std::chrono::high_resolution_clock::now();
			Perft perft(board);
			U64 x = perft.bulk_test_perft(depth, depth);
			auto end = std::chrono::high_resolution_clock::now();
			auto time_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
			std::cout <<std::fixed<< "startpos " << " nodes " << x << " nps " << x / (time_diff / 1000000000.0f) << " time " << time_diff / 1000000000.0f << " seconds" << std::endl;
		}
		if (input.find("go depth") != std::string::npos and not thread_started) {
			std::size_t start_index = input.find("depth");
			std::string depth_str = input.substr(start_index + 6);
			int depth = std::stoi(depth_str);
			threads.begin(depth);
		}
		if (input == "go" or input == "go infinite" and not thread_started) {
			threads.begin(60);
		}
		if (input.find("go movetime") != std::string::npos) {
			std::size_t start_index = input.find("movetime");
			std::string movetime_str = input.substr(start_index + 6);
			int movetime = std::stoi(movetime_str);
			int time_given = time_left(movetime);
			threads.begin(60, time_given);
		}
		if (input.find("go wtime") != std::string::npos) {
			std::vector<std::string> param = split_input(input);
			int movetime = board->side_to_move ? std::stoi(param[4]) :std::stoi(param[2]);
			int time_given = time_left(movetime);
			threads.begin(60, time_given);
		}
		if (input == "b") {
			board->print_board();
			std::cout << board->half_moves << " " << board->full_moves << std::endl;
		}
		if (input == "eval") {
			std::cout << evaluation()<<"\n";
			
		}
		if (input == "captures") {
			MoveList n_moves = board->generate_capture_moves();
			int count = n_moves.e;
			for (int i = 0; i < count; i++) {
				Move move = n_moves.movelist[i];
				std::cout << print_move(move) << std::endl;
			}
			std::cout << "count " << n_moves.e << std::endl;
		}
		if (input == "moves") {
			MoveList n_moves = board->generate_legal_moves();
			int count = n_moves.e;
			for (int i = 0; i < count; i++) {
				Move move = n_moves.movelist[i];
				std::cout << print_move(move) << std::endl;
			}
			std::cout << "count " << n_moves.e << std::endl;
		}
		if (input == "hash") {
			std::cout << board->generate_zhash()<<std::endl;
			std::cout << board->board_hash << std::endl;
		}
	}
}

std::string print_move(Move move) {
	std::string str_move = "";
	int from_index = move.from_square;
	int to_index = move.to_square;
	if (from_index >= 0 and to_index >= 0) {
		std::string from = square_to_coordinates[from_index];
		std::string to = square_to_coordinates[to_index];
		str_move = from + to;
	}
	else {
		std::cout << from_index << " " << to_index;
	}

	std::string pieces[5] = {
		"","n", "b", "r", "q"
	};
	if (move.promotion != -1) {
		std::string prom = pieces[move.promotion];
		str_move += prom;
	}
	return str_move;
}
