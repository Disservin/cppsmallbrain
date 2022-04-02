#pragma once
#include <atomic>
#include <iostream>
#include <chrono>

#include "board.h"

using namespace std::literals::chrono_literals;
extern Board* board;
extern std::atomic<bool> stopped;

class Searcher {
public: 
	static const int max_ply = 60;
	int pv_length[max_ply] = { 0 };
	Move pv_table[max_ply][max_ply] = { };
	int current_ply = 0;
	int search_to_depth = 60;
	U64 nodes = 0;
	Move bestmove;
	int time_given = -1;
	int limit_time = false;
	enum {
		UPPERBOUND, LOWERBOUND, EXACT
	};
	
	
	
	std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();

	Board* board;
	Searcher(Board* brd, int, int tg = -1) {
		board = brd;
		time_given = tg;
		limit_time = time_given != -1 ? true : false;
		begin = std::chrono::high_resolution_clock::now();
	}

	bool can_exit_early();

	int iterative_search(int search_depth, int bench = 0);

	int qsearch(int alpha, int beta, int player, int depth, int ply);

	int alpha_beta(int alpha, int beta, int player, bool root_node, int depth, int ply, bool null);
		
	std::string get_pv_line();

	std::string get_bestmove();

	std::string print_move(Move move);

	int score_move(Move move, bool u_move);

	int mmlva(Move move);

	int is_pv_move(Move move, int ply);
};
