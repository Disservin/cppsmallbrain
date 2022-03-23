#include <chrono>
#include <unordered_map>
#include "board.h"
#include "general.h"
#include "search.h"
#include "evaluation.h"
#include "tt.h"
#include "zobrist.h"

extern TEntry* TTable;
extern U64 tt_size;

bool Searcher::can_exit_early() {
	if (stopped) {
		return true;
	}
	if (nodes & 1023 and limit_time) {
		auto end = std::chrono::high_resolution_clock::now();
		auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
		if (time_given - time_diff < 0) {
			return true;
		}
	}
	return false;
}
void Searcher::iterative_search(int search_depth) {
	int result = 0;
	int lowerbounds = -99999;
	int upperbounds = 99999;
	int player = board->side_to_move ? -1 : 1;
	int ply = 0;
	
	std::string last_pv = "";
	nodes = 0;
	
	begin = std::chrono::high_resolution_clock::now();
	memset(pv_table, 0, sizeof(pv_table));
	memset(pv_length, 0, sizeof(pv_length));
	
	for (int i = 1; i <= search_depth; i++) {
		search_to_depth = i;
		bestmove = {};
		result = alpha_beta(lowerbounds, upperbounds, player, i, 0);
		auto end = std::chrono::high_resolution_clock::now();
		auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
		if (can_exit_early()) {
			if (last_pv != "") {
				std::vector<std::string> param = split_input(last_pv);
				std::string bm = param[0];
				std::cout << "bestmove " << bm << std::endl;
				break;
			}
			std::cout << "bestmove " << get_bestmove() << std::endl;
			break;
		}
		else {
			last_pv = get_pv_line();
			std::cout << std::fixed << "info depth " << i << " score cp " << result << " nodes " << nodes << " nps " << static_cast<int>(nodes / ((time_diff / static_cast<double>(1000)) + 0.01)) << " time " << time_diff << " pv " << get_pv_line() << std::endl;
		}
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
	if (not(stopped or (time_given - time_diff < 0 and limit_time))) {
		std::vector<std::string> param = split_input(last_pv);
		std::string bm = param[0];
		std::cout << "bestmove " << bm << std::endl;
	}
}

int Searcher::qsearch(int alpha, int beta, int player, int depth, int ply) {
	bool IsWhite = board->side_to_move ? 0 : 1;
	int king_sq = _bitscanforward(board->King(IsWhite));
	bool in_check = board->is_square_attacked(IsWhite, king_sq); 
	nodes++;
	if (in_check) {
		int stand_pat = -20000;
	}
	else {
		int stand_pat = evaluation() * player;
		if (stand_pat >= beta) {
			return beta;
		}
		if (alpha < stand_pat) {
			alpha = stand_pat;
		}
	}
	if (depth == 0) {
		return alpha;
	}
	//MoveList n_moves = board->generate_capture_moves();
	MoveList n_moves = board->generate_legal_moves();
	int count = n_moves.e;
	if (count == 0) {
		int game_result = board->is_game_over(IsWhite);
		if (game_result == 0 or game_result == 1) {
			return (- 20000 - ply)* game_result;
		}
	}
	for (int i = 0; i < count; i++) {
		if (stopped) {
			break;
		}
		Move move = n_moves.movelist[i];
		if (board->piece_at(move.to_square) != -1 or move.promotion or (move.piece == board->PAWN and (move.to_square == 7 or move.to_square == 0))) {
			board->make_move(move);
			int score = -qsearch(-beta, -alpha, -player, depth - 1, ply + 1);
			board->unmake_move();
			if (score >= beta) {
				return score;
			}
			if (score > alpha) {
				alpha = score;
			}
		}
	}
	return alpha;
}

//"position fen 1k6/6R1/7P/5K2/8/8/8/8 b - - 0 2";
int Searcher::alpha_beta(int alpha, int beta, int player, int depth, int ply) {
	Move null_move;
	bool is_white = board->side_to_move ? 0 : 1;
	int bestvalue = -100000;
	int old_alpha = alpha;

	null_move.to_square = -1;
	null_move.from_square = -1;
	null_move.piece = -1;
	null_move.null = 1;

	pv_table[ply][ply] = null_move;
	pv_length[ply] = ply;

	int game_result = board->is_game_over(is_white);
	
	if (can_exit_early()) {
		return 0;
	}
	int king_sq = _bitscanforward(board->King(is_white));
	if (depth == 0) {
		if ((board->is_square_attacked(is_white, king_sq))) {
			depth++;
		}
		else {
			//
			return qsearch(alpha, beta, player, 10, ply);//evaluation() * player;// //  //
		}
	}
	U64 key = generate_zhash(board);
	U64 index = key % tt_size;
	TEntry ttentry = TTable[index];
	if (ttentry.key = key) {
		if (ttentry.depth >= depth) {
			if (ttentry.flag == 0) {
				alpha = ttentry.score;
			}
			else if(ttentry.flag == -1) {
				alpha = std::max(alpha, ttentry.score);
			}
			else {
				beta = std::min(beta, ttentry.score);
			}
		}
	}
	MoveList n_moves = board->generate_legal_moves();
	int count = n_moves.e;
	if (count == 0) {
		if (board->is_square_attacked(is_white, king_sq)) {
			return -20000 - depth;
		}
		else {
			return 0;
		}
	}
    for (int i = 0; i < count; i++) {
		if (stopped) {
			break;
		}
        Move move = n_moves.movelist[i];
		//std::cout << print_move(move) << std::endl;;
        board->make_move(move);
        int score = -alpha_beta(-beta, -alpha, -player, depth-1, ply + 1);
        board->unmake_move();
		if (score >= beta) {
			return score;
		}
		if (score > bestvalue) {
			bestvalue = score;
			if (depth == search_to_depth) {
				bestmove = move;
			}
			pv_table[ply][ply] = move;
			for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++) {
				pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
			}
			pv_length[ply] = pv_length[ply + 1];
			if (score > alpha) {
				alpha = score;
			}
		}
	}
	if (!can_exit_early()) {
		//Upperbound
		if (bestvalue <= old_alpha) {
			ttentry.flag = 1;
		}
		//lowerbound
		else if (bestvalue >= beta) {
			ttentry.flag = -1;
		}
		//exact
		else {
			ttentry.flag = 0;
		}
		ttentry.depth = depth;
		ttentry.score = bestvalue;
		ttentry.age = ply;
		ttentry.key = key;
		TTable[index] = ttentry;
	}
	return bestvalue;
}

std::string Searcher::get_pv_line() {
	std::string output = "";
	for (int i = 0; i < pv_length[0]; i++) {
		output += print_move(pv_table[0][i]);
		output += " ";
	}
	return output;
}

std::string Searcher::get_bestmove() {
	return print_move(pv_table[0][0]);
}

std::string Searcher::print_move(Move move) {
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