#include "board.h"
#include "general.h"
#include "evaluation.h"
#include <map>

//std::map <int, int> convert_to_score = {
//   {-1,  0},
//   {0, 100},
//   {1, 320},
//   {2, 330},
//   {3, 500},
//   {4, 900},
//   {5, 10000},
//   {6, 100},
//   {7, 320},
//   {8, 330},
//   {9, 500},
//   {10, 900},
//   {11, 10000},
//};

const int pawnValue = 100;
const int knightValue = 320;
const int bishopValue = 330;
const int rookValue = 500;
const int queenValue = 900;

int evaluation() {
	int eval = 0;
	int material = 0;
	U64 pieces_white = board->White;
	U64 pieces_black = board->Black;
	material += popcount(board->WPawn) * pawnValue;
	material += popcount(board->WBishop) * knightValue;
	material += popcount(board->WKnight) * bishopValue;
	material += popcount(board->WRook) * rookValue;
	material += popcount(board->WQueen) * queenValue;

	material -= popcount(board->BPawn) * pawnValue;
	material -= popcount(board->BBishop) * knightValue;
	material -= popcount(board->BKnight) * bishopValue;
	material -= popcount(board->BRook) * rookValue;
	material -= popcount(board->BQueen) * queenValue;
	
	eval += material;
	return eval;
}