#include <chrono>
#include <algorithm>
#include <signal.h>
#include <iostream>

#include "board.h"
#include "search.h"
#include "uci.h"
#include "timecontroller.h"
#include "evaluation.h"
#include "thread_manager.h"
#include "tt.h"

ThreadManager threads;
Board* board = new Board();
U64 tt_size = 524287;
TEntry* TTable{};

std::atomic<bool> stopped;

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

int main(int argc, char** argv) {
	std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	board->apply_fen(fen);
	std::thread searchThread;
	bool thread_started = false;
	signal(SIGINT, signal_callback_handler);
	TEntry* oldbuffer;
	if ((TTable = (TEntry*)malloc(tt_size * sizeof(TEntry))) == NULL) {
		std::cout << "Error: Could not allocate memory for TT\n";
		exit(1);
	}
	
	while (true) {
		if (argc > 1) {
			if (argv[1] == std::string("bench")) {
				Searcher searcher_class = Searcher(board, 7, -1);
				std::thread thread = std::thread(&Searcher::iterative_search, searcher_class, 7, 1);
				thread.join();
				break;
			}
		}

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
			U64 elements = (static_cast<unsigned long long>(std::stoi(size_str)) * 1000000) / sizeof(TEntry);
			oldbuffer = TTable;
			if ((TTable = (TEntry*)realloc(TTable, elements * sizeof(TEntry))) == NULL)
			{
				std::cout << "Error: Could not allocate memory for TT\n";
				free(oldbuffer);
				exit(1);
			}
			tt_size = elements;
		}
		if (input == "ucinewgame") {
			for (U64 i = 0; i < tt_size; i++) {
				TTable[i] = {};
			}
			board->apply_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		}
		if (input.find("quit") != std::string::npos) {
			threads.stop();
			free(TTable);
			delete board;
			exit(0);
		}
		if (input.find("position fen") != std::string::npos) {
			std::size_t start_index = input.find("fen");
			fen = input.substr(start_index + 4);
			board->apply_fen(fen);
			board->half_moves = 0;
			board->full_moves = 2;
			//board->repetition_table.clear();		
			memset(board->gameHistory, 0 , sizeof(board->gameHistory));
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
			board->full_moves = 2;
			board->half_moves = 0;
			board->apply_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
			//board->repetition_table.clear();
			memset(board->gameHistory, 0, sizeof(board->gameHistory));
			
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
			U64 result = perft.bulk_test_perft(depth, depth);
			auto end = std::chrono::high_resolution_clock::now();
			auto time_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
			std::cout << std::fixed << "startpos " << " nodes " << unsigned(result) << " nps " << unsigned(result / (time_diff / 1000000000.0f)) << " time " << (time_diff / 1000000000.0f) << " seconds" << std::endl;
		}
		if (input.find("test perft") != std::string::npos) {
			std::cout << "\nTest started" << std::endl;
			Perft perft(board);
			perft.test();
			board->apply_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		}
		if (input.find("speedtest") != std::string::npos) {
			std::cout << "\nTest started" << std::endl;
			int depth = 6;
			auto begin = std::chrono::high_resolution_clock::now();
			Perft perft(board);
			U64 x = perft.speed_test_perft(depth, depth);
			auto end = std::chrono::high_resolution_clock::now();
			auto time_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
			std::cout <<std::fixed<< "startpos " << " nodes " << unsigned(x) << " nps " << unsigned(x / (time_diff / 1000000000.0f)) << " time " << unsigned(time_diff / 1000000000.0f) << " seconds" << std::endl;
		}
		if (input.find("go depth") != std::string::npos and not thread_started) {
			std::size_t start_index = input.find("depth");
			std::string depth_str = input.substr(start_index + 6);
			int depth = std::stoi(depth_str);
			threads.begin(depth);
		}
		if ((input == "go" || input == "go infinite") && not thread_started) {
			threads.begin(60);
		}
		if (input.find("go movetime") != std::string::npos) {
			std::size_t start_index = input.find("movetime");
			std::string movetime_str = input.substr(start_index + 6);
			int movetime = std::stoi(movetime_str);
			threads.begin(60, movetime);
		}
		if (input.find("go wtime") != std::string::npos) {
			std::vector<std::string> param = split_input(input);
			int movetime = board->side_to_move ? std::stoi(param[4]) : std::stoi(param[2]);
			int inc = -1;
			int movestogo = 0;
			if (param.size() > 5) {
				inc = board->side_to_move ? std::stoi(param[8]) : std::stoi(param[6]); 
			}
			if (param.size() > 9) {
				movestogo = std::stoi(param[10]);
			}
			unsigned long long time_given = time_left(movetime, inc, movestogo);
			threads.begin(60, time_given);
		}
		if (input == "b") {
			board->print_board();
			std::cout << "fen:         " << board->get_fen() << std::endl;
			std::cout << "eval:        " << signed(evaluation()) << std::endl;
			MoveList moves = board->generate_legal_moves();
			std::cout << "move count:  " << unsigned(moves.size) << std::endl;
			std::cout << "zobrist key: " << unsigned(board->board_hash) << std::endl;
			if (board->en_passant_square == 64)
				std::cout << "en passant square: " << "-" << std::endl;
			else {
				std::cout << "en passant square: " << square_to_coordinates[board->en_passant_square] << std::endl;;
			}
			std::cout << "halfmove: " << unsigned(board->half_moves) << std::endl;
			std::cout << "fullmove: " << unsigned(board->full_moves/2) << std::endl;	
			std::cout << " threefold " << signed(board->is_threefold_rep()) << std::endl;
		}
		if (input == "captures") {
			MoveList n_moves = board->generate_non_quite_moves();
			int count = n_moves.size;
			for (int i = 0; i < count; i++) {
				Move move = n_moves.movelist[i];
				std::cout << print_move(move) << std::endl;
			}
			std::cout << "count " << unsigned(n_moves.size) << std::endl;
		}
		if (input == "moves") {
			MoveList n_moves = board->generate_legal_moves();
			int count = n_moves.size;
			for (int i = 0; i < count; i++) {
				Move move = n_moves.movelist[i];
				std::cout << print_move(move) << std::endl;
			}
			std::cout << "count " << unsigned(n_moves.size) << std::endl;
		}

		if (input == "bench") {
			threads.begin(7, -1, 1);
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
		std::cout << unsigned(from_index) << " " << unsigned(to_index);
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