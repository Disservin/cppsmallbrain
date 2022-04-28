#pragma once
#include <thread>
#include <atomic>

#include "chess.h"
#include "search.h"

extern std::atomic<bool> stopped;

class ThreadManager {
public:
	void begin(Board& board, int depth, bool bench=false, unsigned long long time = 0);
	void stop();
	bool is_searching();
private:
	std::thread threads;
};