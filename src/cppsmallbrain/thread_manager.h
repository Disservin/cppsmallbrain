#pragma once
#include <thread>

#include "board.h"
#include "search.h"

std::atomic<bool> stopped;

class ThreadManager {
public:
	void begin(int depth, int tg = -1) {
		if (is_searching()) {
			stop();
		}
		stopped = false;
		Searcher searcher_class = Searcher(board, depth, tg);
		threads = std::thread(&Searcher::iterative_search, searcher_class, depth);
	}
	void stop() {
		stopped = true;
		if (threads.joinable()) {
			threads.join();
		}
	}
	bool is_searching() {
		return threads.joinable();
	}
private:
	std::thread threads;
};