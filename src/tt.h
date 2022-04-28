#pragma once
#include "chess.h"
using namespace Chess;
struct TEntry {
	unsigned long long key = 0ULL;
	int depth = 0;
	int flag = 0;
	int score = 0;
	int age = 0;
	Moves move{};
};