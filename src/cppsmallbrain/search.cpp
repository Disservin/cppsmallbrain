#include <chrono>
#include <algorithm> 

#include "board.h"
#include "general.h"
#include "search.h"
#include "evaluation.h"
#include "tt.h"

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

int Searcher::iterative_search(int search_depth, int bench) {
	int result = 0;
	int lowerbounds = -INFINITE;
	int upperbounds = INFINITE;
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
		result = alpha_beta(lowerbounds, upperbounds, player, true, i, ply, false);
		auto end = std::chrono::high_resolution_clock::now();
		auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
		if (can_exit_early()) {
			if (last_pv != "") {
				std::vector<std::string> param = split_input(last_pv);
				std::string bm = param[0];
				std::cout << "bestmove " << bm << std::endl;
				return 0;
			}
			std::cout << "bestmove " << get_bestmove() << std::endl;
			return 0;
		}
		else {
			last_pv = get_pv_line();
			std::cout << std::fixed << "info depth " << i << " score cp " << result << " nodes " << nodes << " nps " << static_cast<int>(nodes / ((time_diff / static_cast<double>(1000)) + 0.01)) << " time " << time_diff << " pv " << get_pv_line() << std::endl;
		}
	}	
	std::vector<std::string> param = split_input(last_pv);
	std::string bm = param[0];
	std::cout << "bestmove " << bm << std::endl;
	if (bench) {
		auto end = std::chrono::high_resolution_clock::now();
		auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
		std::cout << "\n---------------------------" << std::endl;
		std::cout << nodes << " nodes "<< static_cast<int>(nodes / ((time_diff / static_cast<double>(1000)) + 0.01))<< " nps" << std::endl;
	}
	return 0;
}

int Searcher::qsearch(int alpha, int beta, int player, int depth, int ply) {
	bool IsWhite = board->side_to_move ? 0 : 1;
	int king_sq = _bitscanforward(board->King(IsWhite));
	bool in_check = board->is_square_attacked(IsWhite, king_sq);
	nodes++;
	int stand_pat = 0;
	if (in_check) {
		 stand_pat = -MATE + ply;
		 if (board->is_checkmate(IsWhite)) {
			 return -MATE + ply;
		}
	}
	else {
		stand_pat = evaluation() * player;
		// Delta Pruning
		if (stand_pat < alpha - 500) return alpha;
		if (stand_pat >= beta) return beta;
		if (alpha < stand_pat) {
			alpha = stand_pat;
		}
	}
	if (depth == 0) {
		return alpha;
	}

	MoveList n_moves = board->generate_legal_moves();
	int count = n_moves.size;
	current_ply = ply;
	std::sort(std::begin(n_moves.movelist), n_moves.movelist + count, [&](const Move& m1, const Move& m2) {return mmlva(m1) > mmlva(m2); });

	for (int i = 0; i < count; i++) {
		if (can_exit_early()) {
			break;
		}
		Move move = n_moves.movelist[i];
		if (board->piece_at(move.to_square) != -1 or move.promotion != -1 or (move.piece == board->PAWN and (move.to_square == 7 or move.to_square == 0))) {
			//if ((stand_pat < alpha - delta_pruning(move)) && popcount(board->Occ) >= 13 && move.promotion == -1 && board->piece_at(move.to_square) != -1) {
			//	return alpha;
			//}
			board->make_move(move);
			int score = -qsearch(-beta, -alpha, -player, depth - 1, ply + 1);
			board->unmake_move();
			if (score > stand_pat) {
				stand_pat = score;
				if (score > alpha) {
					alpha = score;
					if (score >= beta) {
						break;
					}
				}
			}
		}
	}
	return stand_pat;
}

//"position fen 1k6/6R1/7P/5K2/8/8/8/8 b - - 0 2";
int Searcher::alpha_beta(int alpha, int beta, int player, bool root_node, int depth, int ply, bool null) {
	bool Is_White = board->side_to_move ? 0 : 1;
	int bestvalue = -INFINITE;
	int old_alpha = alpha;

	pv_length[ply] = ply;
	// Early exit
	if (can_exit_early()) {
		return 0;
	}
	
	// At root node repetition detection for 3 times
	if (root_node && board->is_threefold_rep3()) {
		return 0;
	}
	// At not root node repetition detection for 2 times
	if (!root_node) {		
		if (board->is_threefold_rep()) {
			return 0;
		}
	}
	// Enter qsearch if not in check else increase depth
	if (depth <= 0) {
		int king_sq = _bitscanforward(board->King(Is_White));
		if ((board->is_square_attacked(Is_White, king_sq))) {
			depth++;
		}
		else {
			nodes++;
			int value = qsearch(alpha, beta, player, 10, ply); //  //;//evaluation() * player; //
			return value;
		}
	}

	// Check TT for entry
	U64 key = board->board_hash;
	U64 index = key % tt_size;
	bool u_move = false;
	if (TTable[index].key == key and TTable[index].depth >= depth and !root_node) {
		if (TTable[index].flag == EXACT) {
			return TTable[index].score;
		}
		else if (TTable[index].flag == LOWERBOUND) {
			alpha = std::max(alpha, TTable[index].score);
		}
		else if (TTable[index].flag == UPPERBOUND) {
			beta = std::min(beta, TTable[index].score);
		}
		if (alpha >= beta) {
			return TTable[index].score;
		}
		// use TT move
		u_move = true;
	}

	MoveList n_moves = board->generate_legal_moves();
	int count = n_moves.size;
	current_ply = ply;
	// Move ordering
	std::sort(std::begin(n_moves.movelist), n_moves.movelist + count, [&](const Move& m1, const Move& m2) {return score_move(m1, u_move) > score_move(m2, u_move); });
	
	int king_sq = _bitscanforward(board->King(Is_White));
	bool inCheck = board->is_square_attacked(Is_White, king_sq);
	
	// Game over ?
	if (count == 0) {
		if (inCheck) {
			return -MATE + ply;
		}
		return 0;
	}
	//if (!root_node and board->half_moves > 75) {
	//	return 0;
	//}

	bool onPv = (beta - alpha > 1);
	if (!inCheck && !onPv) {
		int staticEval = evaluation();
		// Razor
		if (depth <= 1 && (staticEval + 150) < alpha) {
			return qsearch(alpha, beta, player, 10, ply);
		}
		// Null move 
		//if (board->full_moves <= 40 && !null && depth >= 3) {
		//	int reduction = 2;//(depth >= 6) ? 3 : depth >= 3 ? 2 : 1;
		//	int old_ep = board->en_passant_square;
		//	board->side_to_move ^= 1;
		//	board->board_hash ^= RANDOM_ARRAY[780];
		//	board->en_passant_square = 64;
		//	board->full_moves++;
		//	int score = -alpha_beta(-beta, -beta + 1, -player, false, depth - 1 - reduction, ply + 1, true);
		//	board->side_to_move ^= 1;
		//	board->board_hash ^= RANDOM_ARRAY[780];
		//	board->en_passant_square = old_ep;
		//	board->full_moves--;
		//	if (score >= beta) return score;
		//}
	}
	for (int i = 0; i < count; i++) {
		if (can_exit_early()) {
			break;
		}
		Move move = n_moves.movelist[i];
		
		// Passed pawn extension
		int new_depth = depth - 1;
		if (new_depth == 1 and move.piece == 0) {
			if (Is_White and square_rank(move.to_square) >= 5) {
				new_depth++;
			}
			if (!Is_White and square_rank(move.to_square) >= 5) {
				new_depth++;
			}
		}
		board->make_move(move);
		int score = -alpha_beta(-beta, -alpha, -player, false, new_depth, ply + 1, null);
		board->unmake_move();

		// Cut-off
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
				// Beta cut-off
				if (score >= beta) {
					break;
				}
			}
		}
	}
	// Store position in TT
	if (!can_exit_early() and !(bestvalue >= 19000) and !(bestvalue <= -19000) and 
		(TTable[index].depth <= depth or TTable[index].age + 3 <= board->full_moves)) {
		TTable[index].flag = EXACT;
		// Upperbound
		if (bestvalue <= old_alpha) {
			TTable[index].flag = UPPERBOUND;
		}
		// Lowerbound
		else if (bestvalue >= beta) {
			TTable[index].flag = LOWERBOUND;
		}
		TTable[index].depth = depth;
		TTable[index].score = bestvalue;
		TTable[index].age = board->full_moves;
		TTable[index].key = key;
		TTable[index].move = pv_table[0][ply];
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

int Searcher::score_move(Move move, bool u_move) {
	if (is_pv_move(move, current_ply)) {
		return 10000;
	}
	else if (u_move && TTable[board->board_hash % tt_size].move.piece == move.piece &&
		TTable[board->board_hash % tt_size].move.from_square == move.from_square &&
		TTable[board->board_hash % tt_size].move.to_square == move.to_square &&
		TTable[board->board_hash % tt_size].move.promotion == move.promotion) {
		return 5000;
	}
	else if (board->piece_at(move.to_square) != -1 or (move.to_square == board->en_passant_square and move.piece == 0)) {
		return mmlva(move);
	}
	else if (move.piece == -1) {
		return 0;
	}
	else {
		return 50;
	}
}

int Searcher::mmlva(Move move) {
	static constexpr int mvvlva[7][7] = { {0, 0, 0, 0, 0, 0, 0},
	{0, 105, 104, 103, 102, 101, 100},
	{0, 205, 204, 203, 202, 201, 200},
	{0, 305, 304, 303, 302, 301, 300},
	{0, 405, 404, 403, 402, 401, 400},
	{0, 505, 504, 503, 502, 501, 500},
	{0, 605, 604, 603, 602, 601, 600} };
	int attacker = board->piece_type_at(move.from_square) + 1;
	int victim = board->piece_type_at(move.to_square) + 1;
	if (victim == -1) {
		victim = 1;
	}
	return mvvlva[victim][attacker];
}

int Searcher::delta_pruning(Move move) {
	std::map<int, int>piece_values = { { 0, 100 }, { 1 , 320 }, { 2 , 330 }, { 3 , 500 }, { 4 , 900 }, {5 , 0} };
	int piece = board->piece_to_piece_type((board->piece_at_square(move.to_square)));
	return piece_values[piece] + 200;
}
int Searcher::is_pv_move(Move move, int ply) {
	return pv_table[0][ply].from_square == move.from_square && pv_table[0][ply].to_square == move.to_square &&
		pv_table[0][ply].piece == move.piece && pv_table[0][ply].promotion == move.promotion &&
		pv_table[0][ply].null == move.null;
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
