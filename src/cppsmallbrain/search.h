#pragma once
#include "board.h"
#include <atomic>

extern Board* board;
extern unsigned int time_given;
extern std::atomic<bool> stopped;

void searcher(int search_depth);

void iterative_search(int search_depth);

int alpha_beta(int alpha, int beta, int player, int depth, int ply);

std::string print_move(Move move);

std::string get_pv_line();

std::string get_bestmove();