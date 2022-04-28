#include "chess.h"
#include "search.h"
#include <sstream>
#include <map>

using namespace Chess;

std::vector<std::string> split_input(std::string fen);

Move convert_uci_to_Move(std::string input, Board& board);

int main(int argc, char** argv);