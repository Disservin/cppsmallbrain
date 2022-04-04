#include <map>
#include <tuple>

#include "board.h"
#include "general.h"
#include "evaluation.h"

// Values were taken from Stockfish https://github.com/official-stockfish/Stockfish/blob/master/src/psqt.cpp
// Released under GNU General Public License v3.0 https://github.com/official-stockfish/Stockfish/blob/master/Copying.txt
int w_pawn_mg[64] = { 0, 0, 0, 0, 0, 0, 0, 0, 2, 4, 11, 18, 16, 21, 9, -3, -9, -15, 11, 15, 31, 23, 6, -20, -3, -20, 8, 19, 39, 17, 2, -5, 11, -4, -11, 2, 11, 0, -12, 5, 3, -11, -6, 22, -8, -5, -14, -11, -7, 6, -2, -11, 4, -14, 10, -9, 0, 0, 0, 0, 0, 0, 0, 0 };
int b_pawn_mg[64] = { 0, 0, 0, 0, 0, 0, 0, 0, -7, 6, -2, -11, 4, -14, 10, -9, 3, -11, -6, 22, -8, -5, -14, -11, 11, -4, -11, 2, 11, 0, -12, 5, -3, -20, 8, 19, 39, 17, 2, -5, -9, -15, 11, 15, 31, 23, 6, -20, 2, 4, 11, 18, 16, 21, 9, -3, 0, 0, 0, 0, 0, 0, 0, 0 };
int w_knight_mg[64] = { -175, -92, -74, -73, -73, -74, -92, -175, -77, -41, -27, -15, -15, -27, -41, -77, -61, -17, 6, 12, 12, 6, -17, -61, -35, 8, 40, 49, 49, 40, 8, -35, -34, 13, 44, 51, 51, 44, 13, -34, -9, 22, 58, 53, 53, 58, 22, -9, -67, -27, 4, 37, 37, 4, -27, -67, -201, -83, -56, -26, -26, -56, -83, -201 };
int b_knight_mg[64] = { -201, -83, -56, -26, -26, -56, -83, -201, -67, -27, 4, 37, 37, 4, -27, -67, -9, 22, 58, 53, 53, 58, 22, -9, -34, 13, 44, 51, 51, 44, 13, -34, -35, 8, 40, 49, 49, 40, 8, -35, -61, -17, 6, 12, 12, 6, -17, -61, -77, -41, -27, -15, -15, -27, -41, -77, -175, -92, -74, -73, -73, -74, -92, -175 };
int w_bishop_mg[64] = { -37, -4, -6, -16, -16, -6, -4, -37, -11, 6, 13, 3, 3, 13, 6, -11, -5, 15, -4, 12, 12, -4, 15, -5, -4, 8, 18, 27, 27, 18, 8, -4, -8, 20, 15, 22, 22, 15, 20, -8, -11, 4, 1, 8, 8, 1, 4, -11, -12, -10, 4, 0, 0, 4, -10, -12, -34, 1, -10, -16, -16, -10, 1, -34 };
int b_bishop_mg[64] = { -34, 1, -10, -16, -16, -10, 1, -34, -12, -10, 4, 0, 0, 4, -10, -12, -11, 4, 1, 8, 8, 1, 4, -11, -8, 20, 15, 22, 22, 15, 20, -8, -4, 8, 18, 27, 27, 18, 8, -4, -5, 15, -4, 12, 12, -4, 15, -5, -11, 6, 13, 3, 3, 13, 6, -11, -37, -4, -6, -16, -16, -6, -4, -37 };
int w_rook_mg[64] = { -31, -20, -14, -5, -5, -14, -20, -31, -21, -13, -8, 6, 6, -8, -13, -21, -25, -11, -1, 3, 3, -1, -11, -25, -13, -5, -4, -6, -6, -4, -5, -13, -27, -15, -4, 3, 3, -4, -15, -27, -22, -2, 6, 12, 12, 6, 2, -22, -2, 12, 16, 18, 18, 16, 12, -2, -17, -19, -1, 9, 9, -1, -19, -17 };
int b_rook_mg[64] = { -17, -19, -1, 9, 9, -1, -19, -17, -2, 12, 16, 18, 18, 16, 12, -2, -22, -2, 6, 12, 12, 6, -2, -22, -27, -15, -4, 3, 3, -4, -15, -27, -13, -5, -4, -6, -6, -4, -5, -13, -25, -11, -1, 3, 3, -1, -11, -25, -21, -13, -8, 6, 6, -8, -13, -21, -31, -20, -14, -5, -5, -14, -20, -31 };
int w_queen_mg[64] = { 3, -5, -5, 4, 4, -5, -5, 3, -3, 5, 8, 12, 12, 8, 5, -3, -3, 6, 13, 7, 7, 13, 6, -3, 4, 5, 9, 8, 8, 9, 5, 4, 0, 14, 12, 5, 5, 12, 14, 0, -4, 10, 6, 8, 8, 6, 10, -4, -5, 6, 10, 8, 8, 10, 6, -5, -2, -2, 1, -2, -2, 1, -2, -2 };
int b_queen_mg[64] = { -2, -2, 1, -2, -2, 1, -2, -2, -5, 6, 10, 8, 8, 10, 6, -5, -4, 10, 6, 8, 8, 6, 10, -4, 0, 14, 12, 5, 5, 12, 14, 0, 4, 5, 9, 8, 8, 9, 5, 4, -3, 6, 13, 7, 7, 13, 6, -3, -3, 5, 8, 12, 12, 8, 5, -3, 3, -5, -5, 4, 4, -5, -5, 3 };
int w_king_mg[64] = { 271, 327, 271, 198, 198, 271, 327, 271, 278, 303, 234, 179, 179, 234, 303, 278, 195, 258, 169, 120, 120, 169, 258, 195, 164, 190, 138, 98, 98, 138, 190, 164, 154, 179, 105, 70, 70, 105, 179, 154, 123, 145, 81, 31, 31, 81, 145, 123, 88, 120, 65, 33, 33, 65, 120, 88, 59, 89, 45, -1, -1, 45, 89, 59 };
int b_king_mg[64] = { 59, 89, 45, -1, -1, 45, 89, 59, 88, 120, 65, 33, 33, 65, 120, 88, 123, 145, 81, 31, 31, 81, 145, 123, 154, 179, 105, 70, 70, 105, 179, 154, 164, 190, 138, 98, 98, 138, 190, 164, 195, 258, 169, 120, 120, 169, 258, 195, 278, 303, 234, 179, 179, 234, 303, 278, 271, 327, 271, 198, 198, 271, 327, 271 };

int w_pawn_eg[64] = { 0, 0, 0, 0, 0, 0, 0, 0, -8, -6, 9, 5, 16, 6, -6, -18, -9, -7, -10, 5, 2, 3, -8, -5, 7, 1, -8, -2, -14, -13, -11, -6, 12, 6, 2, -6, -5, -4, 14, 9, 27, 18, 19, 29, 30, 9, 8, 14, -1, -14, 13, 22, 24, 17, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0 };
int b_pawn_eg[64] = { 0, 0, 0, 0, 0, 0, 0, 0, -1, -14, 13, 22, 24, 17, 7, 7, 27, 18, 19, 29, 30, 9, 8, 14, 12, 6, 2, -6, -5, -4, 14, 9, 7, 1, -8, -2, -14, -13, -11, -6, -9, -7, -10, 5, 2, 3, -8, -5, -8, -6, 9, 5, 16, 6, -6, -18, 0, 0, 0, 0, 0, 0, 0, 0 };
int w_knight_eg[64] = { -96, -65, -49, -21, -21, -49, -65, -96, -67, -54, -18, 8, 8, -18, -54, -67, -40, -27, -8, 29, 29, -8, -27, -40, -35, -2, 13, 28, 28, 13, -2, -35, -45, -16, 9, 39, 39, 9, -16, -45, -51, -44, -16, 17, 17, -16, -44, -51, -69, -50, -51, 12, 12, -51, -50, -69, -100, -88, -56, -17, -17, -56, -88, -100 };
int b_knight_eg[64] = { -100, -88, -56, -17, -17, -56, -88, -100, -69, -50, -51, 12, 12, -51, -50, -69, -51, -44, -16, 17, 17, -16, -44, -51, -45, -16, 9, 39, 39, 9, -16, -45, -35, -2, 13, 28, 28, 13, -2, -35, -40, -27, -8, 29, 29, -8, -27, -40, -67, -54, -18, 8, 8, -18, -54, -67, -96, -65, -49, -21, -21, -49, -65, -96 };
int w_bishop_eg[64] = { -40, -21, -26, -8, -8, -26, -21, -40, -26, -9, -12, 1, 1, -12, -9, -26, -11, -1, -1, 7, 7, -1, -1, -11, -14, -4, 0, 12, 12, 0, -4, -14, -12, -1, -10, 11, 11, -10, -1, -12, -21, 4, 3, 4, 4, 3, 4, -21, -22, -14, -1, 1, 1, -1, -14, -22, -32, -29, -26, -17, -17, -26, -29, -32 };
int b_bishop_eg[64] = { -32, -29, -26, -17, -17, -26, -29, -32, -22, -14, -1, 1, 1, -1, -14, -22, -21, 4, 3, 4, 4, 3, 4, -21, -12, -1, -10, 11, 11, -10, -1, -12, -14, -4, 0, 12, 12, 0, -4, -14, -11, -1, -1, 7, 7, -1, -1, -11, -26, -9, -12, 1, 1, -12, -9, -26, -40, -21, -26, -8, -8, -26, -21, -40 };
int w_rook_eg[64] = { -9, -13, -10, -9, -9, -10, -13, -9, -12, -9, -1, -2, -2, -1, -9, -12, 6, -8, -2, -6, -6, -2, -8, 6, -6, 1, -9, 7, 7, -9, 1, -6, -5, 8, 7, -6, -6, 7, 8, -5, 6, 1, -7, 10, 10, -7, 1, 6, 4, 5, 20, -5, -5, 20, 5, 4, 18, 0, 19, 13, 13, 19, 0, 18 };
int b_rook_eg[64] = { 18, 0, 19, 13, 13, 19, 0, 18, 4, 5, 20, -5, -5, 20, 5, 4, 6, 1, -7, 10, 10, -7, 1, 6, -5, 8, 7, -6, -6, 7, 8, -5, -6, 1, -9, 7, 7, -9, 1, -6, 6, -8, -2, -6, -6, -2, -8, 6, -12, -9, -1, -2, -2, -1, -9, -12, -9, -13, -10, -9, -9, -10, -13, -9 };
int w_queen_eg[64] = { -69, -57, -47, -26, -26, -47, -57, -69, -54, -31, -22, -4, -4, -22, -31, -54, -39, -18, -9, 3, 3, -9, -18, -39, -23, -3, 13, 24, 24, 13, -3, -23, -29, -6, 9, 21, 21, 9, -6, -29, -38, -18, -11, 1, 1, -11, -18, -38, -50, -27, -24, -8, -8, -24, -27, -50, -74, -52, -43, -34, -34, -43, -52, -74 };
int b_queen_eg[64] = { -74, -52, -43, -34, -34, -43, -52, -74, -50, -27, -24, -8, -8, -24, -27, -50, -38, -18, -11, 1, 1, -11, -18, -38, -29, -6, 9, 21, 21, 9, -6, -29, -23, -3, 13, 24, 24, 13, -3, -23, -39, -18, -9, 3, 3, -9, -18, -39, -54, -31, -22, -4, -4, -22, -31, -54, -69, -57, -47, -26, -26, -47, -57, -69 };
int w_king_eg[64] = { 1, 45, 85, 76, 76, 85, 45, 1, 53, 100, 133, 135, 135, 133, 100, 53, 88, 130, 169, 175, 175, 169, 130, 88, 103, 156, 172, 172, 172, 172, 156, 103, 96, 166, 199, 199, 199, 199, 166, 96, 92, 172, 184, 191, 191, 184, 172, 92, 47, 121, 116, 131, 131, 116, 121, 47, 11, 59, 73, 78, 78, 73, 59, 11 };
int b_king_eg[64] = { 11, 59, 73, 78, 78, 73, 59, 11, 47, 121, 116, 131, 131, 116, 121, 47, 92, 172, 184, 191, 191, 184, 172, 92, 96, 166, 199, 199, 199, 199, 166, 96, 103, 156, 172, 172, 172, 172, 156, 103, 88, 130, 169, 175, 175, 169, 130, 88, 53, 100, 133, 135, 135, 133, 100, 53, 1, 45, 85, 76, 76, 85, 45, 1 };

const int pawnValue = 100;
const int knightValue = 320;
const int bishopValue = 330;
const int rookValue = 500;
const int queenValue = 900;
static std::map<int, int*> piece_to_mg =
	{
	{ 0,   w_pawn_mg},
	{ 1,   w_knight_mg},
	{ 2,   w_bishop_mg},
	{ 3,   w_rook_mg },
	{ 4,   w_queen_mg },
	{ 5,   w_king_mg },
	{ 6,   b_pawn_mg},
	{ 7,   b_knight_mg},
	{ 8,   b_bishop_mg },
	{ 9,   b_rook_mg },
	{ 10,  b_queen_mg },
	{ 11,  b_king_mg },
	};
static std::map<int, int*> piece_to_eg =
	{
	{ 0,   w_pawn_eg},
	{ 1,   w_knight_eg},
	{ 2,   w_bishop_eg},
	{ 3,   w_rook_eg },
	{ 4,   w_queen_eg },
	{ 5,   w_king_eg },
	{ 6,   b_pawn_eg},
	{ 7,   b_knight_eg},
	{ 8,   b_bishop_eg },
	{ 9,   b_rook_eg },
	{ 10,  b_queen_eg },
	{ 11,  b_king_eg },
	};

int evaluation() {
	int eval_mg = 0;
	int eval_eg = 0;
	int material = 0;
	int phase = 0;
	
	int wpawns = popcount(board->bitboards[0]);
	int wknight = popcount(board->bitboards[1]);
	int wbishop = popcount(board->bitboards[2]);
	int wrook = popcount(board->bitboards[3]);
	int wqueen = popcount(board->bitboards[4]);

	int bpawns = popcount(board->bitboards[6]);
	int bknight = popcount(board->bitboards[7]);
	int bbishop = popcount(board->bitboards[8]);
	int brook = popcount(board->bitboards[9]);
	int bqueen = popcount(board->bitboards[10]);
	
	phase += wknight + bknight;
	phase += wbishop + bbishop;
	phase += (wrook + brook) * 2;
	phase += (wqueen + bqueen) * 4;
	
	material += (wpawns - bpawns) * pawnValue;
	material += (wknight - bknight) * knightValue;
	material += (wbishop - bbishop) * bishopValue;
	material += (wrook - brook) * rookValue;
	material += (wqueen - bqueen) * queenValue;

	eval_mg += material;
	eval_eg += material;
	
	U64 pieces_white = board->White;
	U64 pieces_black = board->Black;

	while (pieces_white) {
		int square = pop_lsb(&pieces_white);
		int piece = board->piece_at_square(square);
		eval_mg += piece_to_mg[piece][square];
		eval_eg += piece_to_eg[piece][square];
	}
	while (pieces_black) {
		int square = pop_lsb(&pieces_black);
		int piece = board->piece_at_square(square);
		eval_mg -= piece_to_mg[piece][square];
		eval_eg -= piece_to_eg[piece][square];
	}
	
	//King Safety
	int king_sq_white = _bitscanforward(board->King(true));
	int king_sq_black = _bitscanforward(board->King(false));
	if (king_sq_white == 6) {
		if (!_test_bit(board->bitboards[board->WPAWN], 14) ||
			!_test_bit(board->bitboards[board->WPAWN], 15) || 
			!_test_bit(board->bitboards[board->WPAWN], 23)) {
			eval_mg -= 50;
			eval_eg -= 15;
		}
	}
	if (king_sq_black == 62) {
		if (!_test_bit(board->bitboards[board->BPAWN], 54) ||
			!_test_bit(board->bitboards[board->BPAWN], 55) ||
			!_test_bit(board->bitboards[board->BPAWN], 47)) {
			eval_mg += 50;
			eval_eg += 15;
		}
	}
	U64 hf_w, hf_b;
	std::tie(hf_w, hf_b) = half_open_file(board->bitboards[board->WPAWN], board->bitboards[board->BPAWN]);
	int open_rooks = rook_open_file(hf_w, hf_b, board->bitboards[board->WROOK], board->bitboards[board->BROOK]);
	eval_mg += open_rooks * 44;
	eval_eg += open_rooks * 5;
	
	phase = 24 - phase;
	phase = (phase * 256 + (24 / 2)) / 24;
	return ((eval_mg * (256 - phase)) + (eval_eg * phase))/256;
}

U64 nortFill(U64 gen) {
	gen |= (gen << 8);
	gen |= (gen << 16);
	gen |= (gen << 32);
	return gen;
}
U64 soutFill(U64 gen) {
	gen |= (gen >> 8);
	gen |= (gen >> 16);
	gen |= (gen >> 32);
	return gen;
}
U64 fileFill(U64 gen) {
	return (nortFill(gen) | soutFill(gen));
} 
	
U64 halfopenoropenfile(U64 gen) {
	return ~fileFill(gen);
}

bool is_a_file(int square) {
	if ((square & 7) == 0) {
		return true;
	}
	return false;
}

bool is_h_file(int square) {
	if ((square & 7) == 7) {
		return true;
	}
	return false;
}	

std::tuple<U64, U64> half_open_file(U64 White, U64 Black) {

	U64 openfile = (~(fileFill(White)) & ~(fileFill(White)));

	return { halfopenoropenfile(Black), halfopenoropenfile(Black) };
}

int isolated_pawn(int square, U64 hf_open) {
	if ((not is_a_file(square) and _test_bit(hf_open, square - 1)) ||
		(not is_h_file(square) and _test_bit(hf_open, square + 1)) ||
		(_test_bit(hf_open, square + 1) && _test_bit(hf_open, square - 1))) {
		return 1;
	}
	return 0;
}

std::tuple<int, int> doubled_pawns(U64 pawns, U64 White, U64 Black) {
	U64 bb_pawns_white = pawns & White;
	U64 x = popcount(pawns & (pawns << 8));
	U64 bb_pawns_black = pawns & Black;
	U64 y = popcount(pawns & (pawns >> 8));
	return { x, y };
}

int supported_pawn(int square, U64 pawns, bool IsWhite) {
	if (IsWhite) {
		if (_test_bit(pawns, square - 7) && !is_h_file(square) || _test_bit(pawns, square - 9) && is_a_file(square)) {
			return 1;
		}
	}
	else {
		if (_test_bit(pawns, square + 7) && !is_a_file(square) || _test_bit(pawns, square + 9) && is_h_file(square)) {
			return 1;
		}
	}
	return 0;
}

int neighbour_pawns(int square, U64 pawns) {
	if (_test_bit(pawns, square + 1) && !is_h_file(square) || _test_bit(pawns, square - 1) && !is_a_file(square))
		return 1;
	return 0;
}

int connected_pawns(int square, U64 pawns, bool IsWhite) {
	if (neighbour_pawns(square, pawns) or supported_pawn(square, pawns, IsWhite))
		return 1;
	return 0;
}

int backwards_pawn(int square, U64 pawn_white, U64 pawn_black, bool IsWhite) {
	if (IsWhite) {
		if (neighbour_pawns(square, pawn_white))
			return 0;
		int y = 8;
		while (square-y >= 0) {
			if (_test_bit(pawn_white, square - (y - 1)) && !is_a_file(square) || (_test_bit(pawn_white, square - (y + 1)) && !is_h_file(square)))
				return 1;
			y += 8;
		}
		if (_test_bit(pawn_black, square + (9 * 2)) && !is_h_file(square) || (_test_bit(pawn_black, square + (7 * 2)) && !is_a_file(square)) || _test_bit(pawn_black, square + 8))
			return 1;
	}
	else {
		if (neighbour_pawns(square, pawn_black))
			return 0;
		int y = 8;
		while (square + y < 64) {
			if (_test_bit(pawn_black, square + (y - 1)) && !is_a_file(square) || (_test_bit(pawn_black, square + (y + 1)) && !is_h_file(square)))
				return 1;
			y += 8;
		}
		if (_test_bit(pawn_white, square - (9 * 2)) && !is_h_file(square) || (_test_bit(pawn_white, square - (7 * 2)) && !is_a_file(square)) || _test_bit(pawn_white, square - 8))
			return 1;
	}
	return 0;
}

int rook_open_file(U64 hf_w, U64 hf_b, U64 White_rooks, U64 Black_rooks) {
	int x = popcount(White_rooks & hf_w);
	int y = popcount(Black_rooks & hf_b);
	return x-y;
}