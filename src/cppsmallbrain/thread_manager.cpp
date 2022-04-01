#include <thread>

#include "thread_manager.h"


void ThreadManager::begin(int depth, int tg, int bench) {
	if (is_searching()) {
		stop();
	}
	stopped = false;
	Searcher searcher_class = Searcher(board, depth, tg);
	threads = std::thread(&Searcher::iterative_search, searcher_class, depth, bench);
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