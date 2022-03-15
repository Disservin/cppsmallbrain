#include <chrono>
#include "board.h"
#include "general.h"
#include "search.h"
#include "evaluation.h"

const int max_ply = 60;
int pv_length[max_ply] = { 0 };
Move pv_table[max_ply][max_ply] = { };
int search_to_depth = 256;
U64 nodes = 0;
Move bestmove;

void searcher(int search_depth) {
	iterative_search(search_depth);
}

void iterative_search(int search_depth) {
	int result = 0;
	int lowerbounds = -99999;
	int upperbounds = 99999;
	int player = board->side_to_move ? -1 : 1;
	int ply = 0;
	
	std::string last_pv = "";
	for (int i = 1; i <= search_depth; i++) {
		search_to_depth = i;
		ply = 0;
		nodes = 0;
		auto begin = std::chrono::high_resolution_clock::now();
		result = alpha_beta(lowerbounds, upperbounds, player, i, ply);
		auto end = std::chrono::high_resolution_clock::now();
		auto time_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
		if (stopped) {
			std::string bm = print_move(bestmove);
			std::cout << "bestmove " << bm << std::endl;
			break;
		}
		else {
			last_pv = get_pv_line();
			std::cout << std::fixed << "info depth " << i << " score cp " << result << " nodes " << nodes << " nps " << static_cast<int>(nodes / ((time_diff / 1000000000) + 0.01)) << " time " << (time_diff / 1000000000) * 1000 << " pv " << last_pv << std::endl;
		}
	}
}
//"position fen 1k6/6R1/7P/5K2/8/8/8/8 b - - 0 2";
int alpha_beta(int alpha, int beta, int player, int depth, int ply) {
	Move null_move;
	bool is_white = board->side_to_move ? 0 : 1;
	int bestvalue = -100000;

	null_move.to_square = -1;
	null_move.from_square = -1;
	null_move.capture = -1;
	null_move.piece = -1;
	null_move.null = 1;

	pv_table[ply][ply] = null_move;
	pv_length[ply] = 0;
	int game_result = board->is_game_over(is_white);

	
	if (stopped) {
		return 0;
	}
	if ( game_result == 1 or game_result == 0) {
		nodes++;
		if (ply != 0) {
			return (-20000 - (1.0f / ply) * 100) * game_result;
		}
		return (-20000) * game_result;
		
	}
	if (depth == 0) {
		nodes++;
		return evaluation() * player;
	}
	MoveList n_moves = board->generate_legal_moves();
	int count = board->count;
    for (int i = 0; i < count; i++) {
		if (stopped) {
			break;
		}
        Move move = n_moves.movelist[i];
        board->make_move(move);
        int score = -alpha_beta(-beta, -alpha, -player, depth-1, ply + 1);
        board->unmake_move();
		if (score >= beta) {
			return score;
		}
		if (score > bestvalue) {
			bestvalue = score;
			pv_table[ply][ply] = move;
			for (int next_ply = 0; next_ply < pv_length[ply + 1]; next_ply++) {
				pv_table[ply][ply + 1 + next_ply] = pv_table[ply + 1][ply + 1 + next_ply];
			}
			pv_length[ply] = 1 + pv_length[ply + 1];
			if (score > alpha) {
				alpha = score;
			}
		}
	}
	return bestvalue;
}

std::string square_to_coordinates[64] = {
"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
};

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
		"","N", "B", "R", "Q"
	};
	if (move.promotion != -1) {
		std::cout << pieces[move.promotion] << std::endl;
		std::string prom = pieces[move.promotion];
		str_move += prom;
	}
	return str_move;
}

std::string get_pv_line() {
	std::string output = "";
	for (int i = 0; i < pv_length[0]; i++) {
		output += print_move(pv_table[0][i]);
		output += " ";
	}
	return output;
}

std::string get_bestmove() {
	return print_move(pv_table[0][0]);
}