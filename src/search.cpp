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
	if (stopped) return true;
	if (nodes & 1023 and limit_time) {
		auto end = std::chrono::high_resolution_clock::now();
		auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
		if (time_given - time_diff < 0) return true;
	}
	return false;
}

int Searcher::iterative_search(int search_depth, int bench) {
	int result = 0;
	int player = board->side_to_move ? -1 : 1;

	std::string last_pv = "";
	nodes = 0;

	begin = std::chrono::high_resolution_clock::now();
	memset(pv_table, 0, sizeof(pv_table));
	memset(pv_length, 0, sizeof(pv_length));
	memset(history_table, 0, sizeof(history_table));
	heighest_depth = 0;
	start_age = board->full_moves;
	
	if (board->is_game_over()) {
		std::cout << "info depth 0 score mate 0" << std::endl;
		std::cout << "bestmove (none)" << std::endl;
		return 0;
	}
	if (bench) {
		make_bench();
		return 0;
	}
	for (int depth = 1; depth <= search_depth; depth++) {
		search_to_depth = depth;
		result = aspiration_search(player, depth, result);
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
			std::cout << std::fixed << "info depth " << unsigned(depth) << " seldepth " << unsigned(heighest_depth) << " score cp " << signed(result) << " nodes " << unsigned(nodes) << " nps " << unsigned(static_cast<int>(nodes / ((time_diff / static_cast<double>(1000)) + 0.01))) << " time " << unsigned(time_diff) << " pv " << get_pv_line() << std::endl;
		}
	}	
	std::vector<std::string> param = split_input(last_pv);
	std::string bm = param[0];
	std::cout << "bestmove " << bm << std::endl;
	return 0;
}

int Searcher::aspiration_search(int player, int depth, int prev_eval) {
	uint8_t ply = 0;
	int result = 0;
	int alpha = -INFINITE;
	int beta = INFINITE;
	if (depth == 1) {
		result = alpha_beta(alpha, beta, player, true, depth, ply, false);
	}
	else {
		alpha = prev_eval - 50;
		beta = prev_eval + 50;
		result = alpha_beta(alpha, beta, player, true, depth, ply, false);
	}
	if (result <= alpha or result >= beta) {
		result = alpha_beta(-INFINITE, INFINITE, player, true, depth, ply, false);
	}
	return result;
}

int Searcher::qsearch(int alpha, int beta, int player, uint8_t depth, int ply) {
	bool IsWhite = board->side_to_move ? 0 : 1;
	int8_t king_sq = _bitscanforward(board->King(IsWhite));
	bool in_check = board->is_square_attacked(IsWhite, king_sq);
	int stand_pat = 0;
	if (in_check) {
		 stand_pat = -MATE + ply;
		 if (board->is_checkmate(IsWhite)) return -MATE + ply;
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
	if (depth == 0) return alpha;

	MoveList n_moves = board->generate_capture_moves();
	int count = n_moves.size;
	current_ply = ply;
	std::sort(std::begin(n_moves.movelist), n_moves.movelist + count, [&](const Move& m1, const Move& m2) {return mmlva(m1) > mmlva(m2); });

	for (int i = 0; i < count; i++) {
		if (can_exit_early()) break;
		Move move = n_moves.movelist[i];
		nodes++;
		board->make_move(move);
		int score = -qsearch(-beta, -alpha, -player, depth - 1, ply + 1);
		board->unmake_move();
		if (score > stand_pat) {
			stand_pat = score;
			if (score > alpha) {
				alpha = score;
				if (score >= beta) break;
			}
		}
	}
	return stand_pat;
}

int Searcher::alpha_beta(int alpha, int beta, int player, bool root_node, uint8_t depth, int ply, bool null) {
	bool Is_White = board->side_to_move ? 0 : 1;
	int bestvalue = -INFINITE;
	int old_alpha = alpha;
	bool pv_node = (beta - alpha > 1);

	pv_length[ply] = ply;

	// Early exit
	if (can_exit_early()) return 0;

	// At not root node repetition detection for 2 times
	if (!root_node) {		
		if (board->is_threefold_rep()) return 0;
		if (board->half_moves >= 100) {
			int king_sq = _bitscanforward(board->King(Is_White));
			bool inCheck = board->is_square_attacked(Is_White, king_sq);
			if (inCheck) {
				MoveList moves = board->generate_legal_moves();
				if (moves.size == 0) return -MATE + ply;
			}
			return 0;
		}
		int am_pieces = popcount(board->Occ);
		if (am_pieces == 2) {
			return 0;
		}
		if (am_pieces == 3) {
			if (board->bitboards[board->WKNIGHT] || board->bitboards[board->WBISHOP] ||
				board->bitboards[board->BKNIGHT] || board->bitboards[board->BBISHOP]) return 0;
		}
		if (am_pieces == 4) {
			if (board->bitboards[board->BBISHOP] && board->bitboards[board->WBISHOP] &&
				get_square_color(_bitscanforward(board->bitboards[board->BBISHOP])) ==
				get_square_color(_bitscanforward(board->bitboards[board->WBISHOP]))) return 0;
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
			return qsearch(alpha, beta, player, 10, ply);
		}
	}
	// Seldepth
	if (ply > heighest_depth)
		heighest_depth = ply;
	
	// Check TT for entry
	U64 key = board->board_hash;
	U64 index = key % tt_size;
	bool u_move = false;
	if (TTable[index].key == key and TTable[index].depth >= depth and !root_node) {
		if (TTable[index].flag == EXACT) return TTable[index].score;
		else if (TTable[index].flag == LOWERBOUND) {
			alpha = std::max(alpha, TTable[index].score);
		}
		else if (TTable[index].flag == UPPERBOUND) {
			beta = std::min(beta, TTable[index].score);
		}
		if (alpha >= beta) return TTable[index].score;

		// use TT move
		u_move = true;
	}

	MoveList n_moves = board->generate_legal_moves();
	uint8_t count = n_moves.size;
	current_ply = ply;

	// Move ordering
	std::sort(std::begin(n_moves.movelist), n_moves.movelist + count, [&](const Move& m1, const Move& m2) {return score_move(m1, u_move) > score_move(m2, u_move); });
	
	bool inCheck = board->checkmask == 18446744073709551615ULL ? false : true; 
	
	// Game over ?
	if (!count) {
		if (inCheck) return -MATE + ply;
		return 0;
	}

	int reduction = 0;
	if (!inCheck && !pv_node) {
		int staticEval = evaluation() * player;
		// Razor
		if (depth <= 1 && (staticEval + 150) < alpha) {
			return qsearch(alpha, beta, player, 10, ply);
		}
		// Null move reduction
		if (popcount(board->Occ) >= 13 && !null && depth >= 3) {
			int r = depth > 6 ? 3 : 2;
			
			board->make_null_move();
			
			int score = -alpha_beta(-beta, -beta + 1, -player, false, depth - 1 - r, ply + 1, true);

			board->unmake_null_move();

			if (score >= beta) { 
				reduction = 2;
				if (depth - 3 <= 0) {
					return qsearch(alpha, beta, player, 10, ply);
				}
			}
		}
	}
	
	uint8_t tried_moves = 0;
	for (int i = 0; i < count; i++) {
		if (can_exit_early()) break;
		Move move = n_moves.movelist[i];
		
		// Passed pawn extension
		int new_depth = depth - 1;
		if (new_depth == 1 and move.piece == 0) {
			if (Is_White and square_rank(move.to_square) >= 5) {
				new_depth++;
			}
			if (!Is_White and square_rank(move.to_square) <= 2) {
				new_depth++;
			}
		}
		
		// Late move reduction
		if (tried_moves > 2 + 2 * root_node && depth >= 3 && !u_move 
			&& !inCheck && board->piece_at_square(move.to_square) == -1 && board->non_pawn_material(Is_White)) {
			new_depth -= 1;
		}
		 
		// Increase nodes
		nodes++;
		
		board->make_move(move);

		tried_moves++;

		// Apply reduction
		new_depth -= reduction;

		int score = -alpha_beta(-beta, -alpha, -player, false, new_depth, ply + 1, null);

		board->unmake_move();

		// Cut-off
		if (score > bestvalue) {
			bestvalue = score;
			
			// Save bestmove (PV)
			pv_table[ply][ply] = move;
			for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++) {
				pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
			}
			pv_length[ply] = pv_length[ply + 1];
			
			if (score > alpha) {
				alpha = score;		
				// Beta cut-off
				if (score >= beta) {
					// Killer move heuristic
					if (board->piece_at_square(move.to_square) == -1) {
						killerMoves[1][ply] = killerMoves[0][ply];
						killerMoves[0][ply] = move;
					}
					break;
				}
				// History heuristic
				if (board->piece_at_square(move.to_square) == -1) {
					history_table[Is_White][move.from_square][move.to_square] += depth * depth;
				}
			}
		}
	}

	// Store position in TT
	if (!can_exit_early() and !(bestvalue >= 19000) and !(bestvalue <= -19000) and 
		(TTable[index].depth < depth or TTable[index].age + 3 <= start_age)) {
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
		TTable[index].age = start_age;
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
	int IsWhite = board->side_to_move ? 0 : 1;
	if (compare_moves(move, pv_table[0][current_ply])) {
		return 10000;
	}
	else if (u_move && compare_moves(TTable[board->board_hash % tt_size].move, move)) {
		return 5000;
	}
	else if (move.promotion != -1) {
		return 700;
	}
	else if (board->piece_at_square(move.to_square) != -1) {
		return mmlva(move);
	}
	else if (compare_moves(killerMoves[0][current_ply], move)) {
		return 100;
	}
	else if (compare_moves(killerMoves[1][current_ply], move)) {
		return 75;
	}
	else if (history_table[IsWhite][move.from_square][move.to_square]) {
		return history_table[IsWhite][move.from_square][move.to_square];
	}
	else {
		return 0;
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
		victim = 0;
	}
	return mvvlva[victim][attacker];
}

bool Searcher::compare_moves(Move& m1, Move& m2) {
	return m1.from_square == m2.from_square && m1.to_square == m2.to_square
		   && m1.piece == m2.piece && m1.promotion == m2.promotion;
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

// A few positions from Eigenmanns Rapid Engine Test
int Searcher::make_bench() {
	int player = board->side_to_move ? -1 : 1;
	int result{};
	U64 nodes_searched = 0;
	board->apply_fen("r1bqk1r1/1p1p1n2/p1n2pN1/2p1b2Q/2P1Pp2/1PN5/PB4PP/R4RK1 w q - 0 1");
	heighest_depth = 0;
	nodes = 0;
	std::cout << "fen: r1bqk1r1/1p1p1n2/p1n2pN1/2p1b2Q/2P1Pp2/1PN5/PB4PP/R4RK1 w q - 0 1" << std::endl;
	for (int depth = 1; depth <= 7; depth++) {
		result = aspiration_search(player, depth, result);
		nodes_searched += nodes;
		std::cout << "info depth " << unsigned(depth) << " seldepth " << unsigned(heighest_depth) << " score cp " << signed(result) << " nodes " << unsigned(nodes) << std::endl;
	}
	std::cout << "" << std::endl;
	board->apply_fen("r1n2N1k/2n2K1p/3pp3/5Pp1/b5R1/8/1PPP4/8 w - - 0 1");
	heighest_depth = 0;
	nodes = 0;
	std::cout << "fen: r1n2N1k/2n2K1p/3pp3/5Pp1/b5R1/8/1PPP4/8 w - 0 1" << std::endl;
	for (int depth = 1; depth <= 7; depth++) {
		result = aspiration_search(player, depth, result);
		nodes_searched += nodes;
		std::cout << "info depth " << unsigned(depth) << " seldepth " << unsigned(heighest_depth) << " score cp " << signed(result) << " nodes " << unsigned(nodes) << std::endl;
	}
	std::cout << "" << std::endl;
	board->apply_fen("r1b1r1k1/1pqn1pbp/p2pp1p1/P7/1n1NPP1Q/2NBBR2/1PP3PP/R6K w - - 0 1");
	heighest_depth = 0;
	nodes = 0;
	std::cout << "fen: r1b1r1k1/1pqn1pbp/p2pp1p1/P7/1n1NPP1Q/2NBBR2/1PP3PP/R6K w - - 0 1" << std::endl;
	for (int depth = 1; depth <= 7; depth++) {
		result = aspiration_search(player, depth, result);
		nodes_searched += nodes;
		std::cout << "info depth " << unsigned(depth) << " seldepth " << unsigned(heighest_depth) << " score cp " << signed(result) << " nodes " << unsigned(nodes) << std::endl;
	}
	std::cout << "" << std::endl;
	board->apply_fen("5b2/p2k1p2/P3pP1p/n2pP1p1/1p1P2P1/1P1KBN2/7P/8 w - - 0 1");
	heighest_depth = 0;
	nodes = 0;
	std::cout << "fen: 5b2/p2k1p2/P3pP1p/n2pP1p1/1p1P2P1/1P1KBN2/7P/8 w - - 0 1" << std::endl;
	for (int depth = 1; depth <= 7; depth++) {
		result = aspiration_search(player, depth, result);
		nodes_searched += nodes;
		std::cout << "info depth " << unsigned(depth) << " seldepth " << unsigned(heighest_depth) << " score cp " << signed(result) << " nodes " << unsigned(nodes) << std::endl;
	}
	std::cout << "" << std::endl;
	board->apply_fen("r3kbnr/1b3ppp/pqn5/1pp1P3/3p4/1BN2N2/PP2QPPP/R1BR2K1 w kq - 0 1");
	heighest_depth = 0;
	nodes = 0;
	std::cout << "fen: r3kbnr/1b3ppp/pqn5/1pp1P3/3p4/1BN2N2/PP2QPPP/R1BR2K1 w kq - 0 1" << std::endl;
	for (int depth = 1; depth <= 6; depth++) {
		result = aspiration_search(player, depth, result);
		nodes_searched += nodes;
		std::cout << "info depth " << unsigned(depth) << " seldepth " << unsigned(heighest_depth) << " score cp " << signed(result) << " nodes " << unsigned(nodes) << std::endl;
	}	
	std::cout << "" << std::endl;
	board->apply_fen("5rk1/pp1b4/4pqp1/2Ppb2p/1P2p3/4Q2P/P3BPP1/1R3R1K b - - 0 1");
	heighest_depth = 0;
	nodes = 0;
	std::cout << "fen: 5rk1/pp1b4/4pqp1/2Ppb2p/1P2p3/4Q2P/P3BPP1/1R3R1K b - - 0 1" << std::endl;
	for (int depth = 1; depth <= 7; depth++) {
		result = aspiration_search(player, depth, result);
		nodes_searched += nodes;
		std::cout << "info depth " << unsigned(depth) << " seldepth " << unsigned(heighest_depth) << " score cp " << signed(result) << " nodes " << unsigned(nodes) << std::endl;
	}	
	std::cout << "" << std::endl;
	board->apply_fen("r3r2k/1bq1nppp/p2b4/1pn1p2P/2p1P1QN/2P1N1P1/PPBB1P1R/2KR4 w - - 0 1");
	heighest_depth = 0;
	nodes = 0;
	std::cout << "fen: r3r2k/1bq1nppp/p2b4/1pn1p2P/2p1P1QN/2P1N1P1/PPBB1P1R/2KR4 w - - 0 1" << std::endl;
	for (int depth = 1; depth <= 6; depth++) {
		result = aspiration_search(player, depth, result);
		nodes_searched += nodes;
		std::cout << "info depth " << unsigned(depth) << " seldepth " << unsigned(heighest_depth) << " score cp " << signed(result) << " nodes " << unsigned(nodes) << std::endl;
	}
	std::cout << "" << std::endl;
	board->apply_fen("r1b2rk1/p2nqppp/1ppbpn2/3p4/2P5/1PN1PN2/PBQPBPPP/R4RK1 w - - 0 1");
	heighest_depth = 0;
	nodes = 0;
	std::cout << "fen: r1b2rk1/p2nqppp/1ppbpn2/3p4/2P5/1PN1PN2/PBQPBPPP/R4RK1 w - - 0 1" << std::endl;
	for (int depth = 1; depth <= 6; depth++) {
		result = aspiration_search(player, depth, result);
		nodes_searched += nodes;
		std::cout << "info depth " << unsigned(depth) << " seldepth " << unsigned(heighest_depth) << " score cp " << signed(result) << " nodes " << unsigned(nodes) << std::endl;
	}	
	std::cout << "" << std::endl;
	board->apply_fen("r4r1k/p1p3bp/2pp2p1/4nb2/N1P4q/1P5P/PBNQ1PP1/R4RK1 b - - 0 1");
	heighest_depth = 0;
	nodes = 0;
	std::cout << "fen: r4r1k/p1p3bp/2pp2p1/4nb2/N1P4q/1P5P/PBNQ1PP1/R4RK1 b - - 0 1" << std::endl;
	for (int depth = 1; depth <= 6; depth++) {
		result = aspiration_search(player, depth, result);
		nodes_searched += nodes;
		std::cout << "info depth " << unsigned(depth) << " seldepth " << unsigned(heighest_depth) << " score cp " << signed(result) << " nodes " << unsigned(nodes) << std::endl;
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
	std::cout << std::fixed << "time " << time_diff << std::endl;
	std::cout << "\n---------------------------" << std::endl;
	std::cout << unsigned(nodes_searched) << " nodes " << unsigned(static_cast<int>(nodes_searched / ((time_diff / static_cast<double>(1000)) + 0.01))) << " nps" << std::endl;
	return 0;
}