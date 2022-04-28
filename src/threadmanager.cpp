#include <thread>

#include "threadmanager.h"


void ThreadManager::begin(Board& board, int depth, bool bench) {
	if (is_searching()) {
		stop();
	}
	stopped = false;
	Search searcher_class = Search(board);
	threads = std::thread(&Search::iterative_deepening, searcher_class, depth, bench);
}
void ThreadManager::stop() {
	stopped = true;
	if (threads.joinable()) {
		threads.join();
	}
}
bool ThreadManager::is_searching() {
	return threads.joinable();
}