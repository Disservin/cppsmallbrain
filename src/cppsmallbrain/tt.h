#pragma once
#include "general.h"

struct TEntry {
	U64 key = 0ULL;
	int depth = 0;
	int flag = 0;
	int score = 0;
	int age = 0;
	Move move{};
};