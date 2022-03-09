#include "board.cpp"
#include "rays.h"
#include <assert.h>

int perft_test(std::string fen, int depth) {
	board board;
	board.apply_fen(fen);
	board.split_fen(fen);
	auto begin = std::chrono::high_resolution_clock::now();
	U64 x = perft(board, depth, depth);
	auto end = std::chrono::high_resolution_clock::now();
	auto time_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
	//std::cout << "nodes " << x << " nps " << x / (time_diff / 1000000000.0f) << " time " << time_diff / 1000000000.0f << " seconds" << std::endl;
	return x;
}

std::string fen1 = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
std::string fen2 = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
std::string fen3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
std::string fen4 = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
std::string fen5 = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
std::string fen6 = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ";

int test() {
	assert(perft_test(fen1, 5) == 4865609);
	assert(perft_test(fen2, 4) == 4085603);
	assert(perft_test(fen3, 5) == 674624);
	assert(perft_test(fen4, 4) == 422333);
	assert(perft_test(fen5, 4) == 2103487);
	assert(perft_test(fen6, 3) == 3894594);
	return 0;
}