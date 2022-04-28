#pragma once
#include <atomic>
#include "chess.h"

extern std::atomic<bool> stopped;
using namespace Chess;

class Search{
    public: 
        uint64_t nodes;
        Board board;
        Move bestMove;
        static const uint8_t max_ply = 60;
        uint8_t pv_length[max_ply] = { 0 };
        Move pv_table[max_ply][max_ply]{};
        uint8_t current_ply = 0;
        uint8_t search_to_depth = 60;
        int history_table[2][64][64] = { {0},{0} };
        Move killerMoves[2][max_ply + 1]{};
        int8_t heighest_depth = 0;
        uint8_t searchDepth;
        unsigned long long searchtime = 0;
        std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		
        Search (Board brd){
            board = brd;
            begin = std::chrono::high_resolution_clock::now();
        }
		
        bool exit_early();
        int mmlva(Move move);
        bool compare_moves(Move& m1, Move& m2);
        int score_move(Move move);
        template <Color color> int qsearch(int depth, int alpha, int beta, int player, int ply);
        template <Color color> int absearch(int depth, int alpha, int beta, int player, int ply, bool null);
        template <Color color> int aspiration_search(int player, int depth, int prev_eval);
        int iterative_deepening(int depth, bool bench, unsigned long long time);
};