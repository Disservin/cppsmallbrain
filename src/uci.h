#pragma once
#include <map>
#include "general.h"
#include "tt.h"

Move convert_uci_to_Move(std::string input) {
    Move move;
    if (input.length() == 4) {
        std::string from = input.substr(0, 2);
        std::string to = input.substr(2);
        int from_index;
        int to_index;
        char letter;
        letter = from[0];
        int file = letter - 96;
        int rank = from[1] - 48;
        from_index = (rank - 1) * 8 + file - 1;
        move.from_square = from_index;
        letter = to[0];
        file = letter - 96;
        rank = to[1] - 48;
        to_index = (rank - 1) * 8 + file - 1;
        move.to_square = to_index;
        move.piece = board->piece_type_at(from_index);
        move.capture = board->piece_at_square(to_index);
        move.promotion = -1;
        return move;
    }
    if (input.length() == 5) {
        std::string from = input.substr(0, 2);
        std::string to = input.substr(2,2);
        char prom = input.at(4);
        int from_index;
        int to_index;
        char letter;
        letter = from[0];
        int file = letter - 96;
        int rank = from[1] - 48;
        from_index = (rank - 1) * 8 + file - 1;

        move.from_square = from_index;
        letter = to[0];
        file = letter - 96;
        rank = to[1] - 48;
        to_index = (rank - 1) * 8 + file - 1;
        move.to_square = to_index;
        std::map<char, int> piece_to_int = 
        {
        { 'n', 1 },
        { 'b', 2 },
        { 'r', 3 },
        { 'q', 4 }
        };
        move.promotion = piece_to_int[prom];
        move.piece = 0;
        move.capture = board->piece_at_square(to_index);
        return move;
    }
    else {
        std::cout << "FALSE INPUT" << std::endl;
        move.to_square = -1;
        move.from_square = -1;
        move.piece = -1;
        move.promotion = -1;
        move.capture = -1;
        return move;
    }
}
std::string print_move(Move move);