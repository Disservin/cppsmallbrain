#pragma once
#include <thread>

#include "board.h"
#include "search.h"

std::atomic<bool> stopped;

class ThreadManager {
public:
	void begin(int depth) {
		if (is_searching()) {
			stop();
		}
		stopped = false;
		threads = std::thread(searcher, depth);
		threads.detach();
	}
	void stop() {
		stopped = true;
		if (threads.joinable()) {
			threads.detach();
		}
		std::cout << "successful stop\n"<< std::endl;
	}
	bool is_searching() {
		return threads.joinable();
	}
private:
	std::thread threads;

};