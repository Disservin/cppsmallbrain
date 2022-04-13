#pragma once
#include <thread>

#include "board.h"
#include "search.h"

extern std::atomic<bool> stopped;

class ThreadManager {
public:
	void begin(int depth, long long tg = -1, int bench = 0);
	void stop();
	bool is_searching();
private:
	std::thread threads;
};