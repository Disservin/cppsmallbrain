#pragma once
#include "board.h"
#include "search.h"
#include <map>

extern Board* board;
int evaluation();

U64 nortFill(U64 gen);
U64 soutFill(U64 gen);
U64 fileFill(U64 gen);

U64 halfopenoropenfile(U64 gen);

bool is_a_file(int square);

bool is_h_file(int square);

std::tuple<U64, U64> half_open_file(U64 White, U64 Black);

int isolated_pawn(int square, U64 hf_open);

std::tuple<int, int> doubled_pawns(U64 pawns, U64 White, U64 Black);

int supported_pawn(int square, U64 pawns, bool IsWhite);

int neighbour_pawns(int square, U64 pawns);

int connected_pawns(int square, U64 pawns, bool IsWhite);

int backwards_pawn(int square, U64 pawn_white, U64 pawn_black, bool IsWhite);

int rook_open_file(U64 hf_w, U64 hf_b, U64 White_rooks, U64 Black_rooks);
