#include "search.h"
#include "evaluation.h"

bool Search::exit_early() {
    if (stopped) return true;
    if (nodes & 2047 && searchtime != 0) {
        auto end = std::chrono::high_resolution_clock::now();
        auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        if (searchtime - time_diff <= 0) return true;
    }
    return false;
}

int Search::mmlva(Move move) {
    static constexpr int mvvlva[7][7] = { {0, 0, 0, 0, 0, 0, 0},
    {0, 205, 204, 203, 202, 201, 200},
    {0, 305, 304, 303, 302, 301, 300},
    {0, 405, 404, 403, 402, 401, 400},
    {0, 505, 504, 503, 502, 501, 500},
    {0, 605, 604, 603, 602, 601, 600},
    {0, 705, 704, 703, 702, 701, 700} };
    int attacker = board.getPiece(move.source()) % 6 + 1;
    int victim = board.getPiece(move.target()) % 6 + 1;
    if (victim == -1) {
        victim = 0;
    }
    return mvvlva[victim][attacker];
}

bool Search::compare_moves(Move& m1, Move& m2) {
    return m1.source() == m2.source() && m1.target() == m2.target()
        && m1.piece() == m2.piece() && m1.promoted() == m2.promoted();
}

int Search::score_move(Move move) {
    int IsWhite = board.sideToMove ? 0 : 1;
    if (compare_moves(move, pv_table[0][current_ply])) {
        return 100000;
    }
    else if (move.promoted()) {
        return 80000;
    }
    else if (board.getPiece(move.target()) != None) {
        return mmlva(move) * 100;
    }
    else if (compare_moves(killerMoves[0][current_ply], move)) {
        return 5000;
    }
    else if (compare_moves(killerMoves[1][current_ply], move)) {
        return 4000;
    }
    else if (history_table[IsWhite][move.source()][move.target()]) {
        return history_table[IsWhite][move.source()][move.target()];
    }
    else {
        return 0;
    }
}

template <Color color>
int Search::qsearch(int depth, int alpha, int beta, int player, int ply){
    if (exit_early()) return 0;
    int stand_pat = evaluation(board) * player;
    Square king_sq = board.KingSq<color>();
    bool inCheck = board.isSquareAttacked<~color>(king_sq);
    if (inCheck){
        stand_pat = -100000+ply;
        Moves moveList = board.generateLegalMoves<color>();
        if (moveList.count == 0) return -100000+ply;
    }else{
        if (stand_pat >= beta) return beta;
        if (alpha < stand_pat) alpha = stand_pat;
    }
    if (depth == 0) return stand_pat;

    Moves moveList = board.generateLegalMoves<color>();
    std::sort(std::begin(moveList.moves), moveList.moves + moveList.count, [&](const Move& m1, const Move& m2)
        {return mmlva(m1) > mmlva(m2); });
	
    for (int i = 0; i < (int)moveList.count; i++){
        Move move = moveList.moves[i];
        if (board.board[move.target()] == None && !inCheck) continue;
        nodes++;
        board.makemove<color>(move);
        int score = -qsearch<~color>(depth - 1, -beta, -alpha, -player, ply + 1);
        board.unmakemove<color>(move);
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

template <Color color>
int Search::absearch(int depth, int alpha, int beta, int player, int ply, bool null){
    if (exit_early()) return 0;

    if (ply == 0) {
        if (board.isRepetition()) return (0 + (2 * (nodes & 1) - 1));
        if (board.halfmoves >= 100) {
            Moves moveList = board.generateLegalMoves<color>();
			if (moveList.count == 0) return -100000+ply;
            return 0;
        }
    }
	
    bool inCheck = board.isSquareAttacked<~color>(board.KingSq<color>());
    if (inCheck && depth <= 0)
        depth++;
	
    if (depth <= 0) {
        return qsearch<color>(10, alpha, beta, player, ply);
    }
    pv_length[ply] = ply;
    // Seldepth
    if (ply > heighest_depth) {
        heighest_depth = ply;
    }
	
	int staticEval = evaluation(board) * player;
	bool pvNode = beta - alpha > 1;
	
    if (depth == 1 && (staticEval + 150) < alpha && !inCheck && !pvNode)
        return qsearch<color>(10, alpha, beta, player, ply);
	
    if (board.allPieces<color>() >= 5 && !null && depth >= 3 && !inCheck && !pvNode) {
        int r = depth > 6 ? 3 : 2;

        board.makenull();

		int score = -absearch<~color>(depth - 1 - r, -beta, -beta + 1, -player, ply + 1, true);

        board.unmakenull();

        if (score >= beta) {
            return beta;
        }
    }
	//	
    if (std::abs(beta) < -100000 - max_ply && !inCheck && !pvNode)
        if (staticEval - 150 * depth >= beta) return beta;
		
    int bestValue = -100000;
    Moves moveList = board.generateLegalMoves<color>();
    std::sort(std::begin(moveList.moves), moveList.moves + moveList.count, [&](const Move& m1, const Move& m2)
        {return score_move(m1) > score_move(m2); });
	
    uint8_t legal_moves = 0;
    int score = 0;
    for (int i = 0; i < (int)moveList.count; i++) {
        Move move = moveList.moves[i];
        nodes++;
        board.makemove<color>(move);
        if (legal_moves == 1) {
            int score = -absearch<~color>(depth - 1, -beta, -alpha, -player, ply + 1, false);
        }
        else {
			if (depth >= 3 && !pvNode && !inCheck && legal_moves > 3 + 2 * (ply == 0))
                int score = -absearch<~color>(depth - 2, -beta, -alpha, -player, ply + 1, false);
            else {
                score = alpha + 1;
            }
            if (score > alpha) {
                score = -absearch<~color>(depth - 1, -alpha - 1, -alpha, -player, ply + 1, false);
                if (score > alpha && score < beta)
                    score = -absearch<~color>(depth - 1, -beta, -alpha, -player, ply + 1, false);
            }
        }
        board.unmakemove<color>(move);
        if(score > bestValue){
            bestValue = score;
            if (depth == searchDepth){
                    bestMove = move;
            }
			
            // Save bestmove (PV)
            pv_table[ply][ply] = move;
            if (ply + 1 == 60)
                break;
            for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++) {
                pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
            }
            pv_length[ply] = pv_length[ply + 1];

            if (score > alpha) {
                alpha = score;
                if (board.getPiece(move.target()) == None) {
                    history_table[~color][move.source()][move.target()] += depth * depth;
                }
                // Beta cut-off
                if (score >= beta) {
                    // Killer move heuristic
                    if (board.getPiece(move.target()) == None) {
                        killerMoves[1][ply] = killerMoves[0][ply];
                        killerMoves[0][ply] = move;
                    }
                    break;
                }
            }
        }
    }
    if (moveList.count == 0){
        return inCheck ? -100000+ply : 0;
    }
    return bestValue;
}

template <Color color>
int Search::aspiration_search(int player, int depth, int prev_eval){
    int alpha = -100000;
    int beta = 100000;
    int result = 0;
    int ply = 0;
    if (depth == 1){
        result = absearch<color>(1, alpha, beta, player, ply, false); 
    }
    else{
        alpha = prev_eval - 100;
        beta = prev_eval + 100;
        result = absearch<color>(depth, alpha, beta, player, ply, false);
    }
    if (result <= alpha || result >= beta){
        return absearch<color>(depth, -100000, 100000, player, ply, false);
    }
    return result;
}

int Search::iterative_deepening(int search_depth, bool bench, unsigned long long time){
    int result = 0;
    Color color = board.sideToMove;
    auto t1 = std::chrono::high_resolution_clock::now();
    Move prev_bestmove;
    stopped = false;
    if (bench) {
        for (int depth = 1; depth <= 5; depth++) {
            searchDepth = depth;
            if (color == White) {
                result = aspiration_search<White>(1, depth, result);
            }
            else {
                result = aspiration_search<Black>(-1, depth, result);
            }
            if (exit_early()) break;
            prev_bestmove = bestMove;
            std::string move = board.printUciMove(bestMove);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        std::cout << nodes << " nodes " << signed((nodes / (ms + 1)) * 1000) << " nps " << std::endl;
        return 0;
    }
    searchtime = time;
    for (int depth = 1; depth <= search_depth; depth++){
        searchDepth = depth;
        if (color == White){
            result = aspiration_search<White>(1, depth, result);
        }
        else{
            result = aspiration_search<Black>(-1, depth, result);
        }
        if (exit_early()){
            std::string move = board.printUciMove(bestMove);
            std::cout << "bestmove " << move << std::endl;
            return 0;
        }
        prev_bestmove = bestMove;
        std::string move = board.printUciMove(bestMove);
        auto t2 = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        std::cout <<"info depth " << signed(depth) << " score cp " 
                  << result       << " pv "        << move 
                  << " nodes "    << nodes         << " nps " 
                  << signed((nodes/(ms+1))*1000)   << std::endl;
    }
    return 0;
}