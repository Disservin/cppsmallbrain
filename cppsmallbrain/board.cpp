#pragma once
#include <iostream>
#include <list>
#include <bitset>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <iterator>
#include <vector>
#include <chrono>
#include <cmath>
#include <intrin.h>
#include <assert.h>
#include "board.h"
#include "rays.h"

std::vector<std::string> board::split_fen(std::string fen)
{
    std::stringstream fen_stream(fen);
    std::string segment;
    std::vector<std::string> seglist;

    while (std::getline(fen_stream, segment, ' '))
    {
        seglist.push_back(segment);
    }
    return seglist;
}

void board::apply_fen(std::string fen)
{
    
    for (int i = 0; i < 12; i++)
    {
        bitboards[i] = 0ULL;
    }
    std::string position = split_fen(fen)[0];
    std::string move_right = split_fen(fen)[1];
    std::string castling = split_fen(fen)[2];
    std::string en_passant = split_fen(fen)[3];
    std::string half_move_clock = split_fen(fen)[4];
    std::string full_move_counter = split_fen(fen)[5];

    //Side to move
    if (move_right == "w")
    {
        side_to_move = 0;
    }
    else
    {
        side_to_move = 1;
    }

    //Placing pieces on the board
    int pos = 0;
    int sq;
    char letter;
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            sq = rank * 8 + file;
            letter = position[pos];
            if (pos >= position.size())
            {
                break;
            }
            switch (letter)
            {
            case 'p':
                bitboards[6] |= (1ULL << sq);
                pos++;
                break;
            case 'r':
                bitboards[9] |= (1ULL << sq);
                pos++;
                break;
            case 'n':
                bitboards[7] |= (1ULL << sq);
                pos++;
                break;
            case 'b':
                bitboards[8] |= (1ULL << sq);
                pos++;
                break;
            case 'q':
                bitboards[10] |= (1ULL << sq);
                pos++;
                break;
            case 'k':
                bitboards[11] |= (1ULL << sq);
                pos++;
                break;
            case 'P':
                bitboards[0] |= (1ULL << sq);
                pos++;
                break;
            case 'R':
                bitboards[3] |= (1ULL << sq);
                pos++;
                break;
            case 'N':
                bitboards[1] |= (1ULL << sq);
                pos++;
                break;
            case 'B':
                bitboards[2] |= (1ULL << sq);
                pos++;
                break;
            case 'Q':
                bitboards[4] |= (1ULL << sq);
                pos++;
                break;
            case 'K':
                bitboards[5] |= (1ULL << sq);
                pos++;
                break;
            case '/':
                file--;
                pos++;
                break;
            case '1':
                pos += 1;
                break;
            case '2':
                file += 1;
                pos++;
                break;
            case '3':
                file += 2;
                pos++;
                break;
            case '4':
                file += 3;
                pos++;
                break;
            case '5':
                file += 4;
                pos++;
                break;
            case '6':
                file += 5;
                pos++;
                break;
            case '7':
                file += 6;
                pos++;
                break;
            case '8':
                rank -= 1;
                file--;
                pos++;
                break;
            default:
                break;
            }
        }
    }

    // Encodes castling rights
    for (int i = 0; i < castling.size(); i++) {
        if (castling[i] == 'K')
        {
            castling_rights |= wk;
        }
        if (castling[i] == 'Q')
        {
            castling_rights |= wq;
        }
        if (castling[i] == 'k')
        {
            castling_rights |= bk;
        }
        if (castling[i] == 'q')
        {
            castling_rights |= bq;
        }
    }

    //Changing en passant from FEN into en passant square
    if (en_passant == "-")
    {
        en_passant_square = no_sq;
    }
    else
    {
        letter = en_passant[0];
        int file = letter - 96;
        int rank = en_passant[1] - 48;
        en_passant_square = (rank - 1) * 8 + file - 1;
    }

    //half_move_clock
    half_moves = std::stoi(half_move_clock);

    //full_move_counter
    full_moves = std::stoi(full_move_counter);

    //Udates Bitboards
    update_occupancies();
}
void board::get_side_to_move() {
    std::cout << "get_move_right: " << side_to_move << std::endl;
}
void board::get_castling_rights() {
    std::cout << "castling: " << castling_rights << std::endl;
}
void board::get_en_passant_square() {
    std::cout << "en_passant: " << en_passant_square << std::endl;
}
void board::get_half_moves() {
    std::cout << "half_moves: " << half_moves << std::endl;
}
void board::get_full_moves() {
    std::cout << "full_moves: " << full_moves << std::endl;
}

inline void board::update_occupancies() {
    // White
    occupancies[0] = bitboards[0] | bitboards[1] | bitboards[2] | bitboards[3] | bitboards[4] | bitboards[5];
    // Black
    occupancies[1] = bitboards[6] | bitboards[7] | bitboards[8] | bitboards[9] | bitboards[10] | bitboards[11];
    both = occupancies[0] | occupancies[1];

    pawns   = bitboards[0] | bitboards[6];
    knights = bitboards[1] | bitboards[7];
    bishops = bitboards[2] | bitboards[8];
    rooks   = bitboards[3] | bitboards[9];
    queens  = bitboards[4] | bitboards[10];
    kings   = bitboards[5] | bitboards[11];
}

inline bool board::piece_color(int sq) {
    /* returns true if piece is white*/
    if (_test_bit(occupancies[0], sq)) {
        return true;
    }
    else {
        return false;
    }
    //if (std::bitset<64>(occupancies[0]).test(sq) == 1) {
    //    return true;
    //}
    //else {
    //    return false;
    //}
}
inline int board::piece_at(int sq, int given) {
    /* returns color specific int for piece*/
    bool white = false;
    if (given > -1) {
        white = given;
    }
    else {
        if (_test_bit(occupancies[0], sq)) {
            white = true;
        }
    }

    if (white) {
        if (_test_bit(bitboards[WPAWN], sq)) {
            return WPAWN;
        }
        if (_test_bit(bitboards[WKNIGHT], sq)) {
            return WKNIGHT;
        }
        if (_test_bit(bitboards[WBISHOP], sq)) {
            return WBISHOP;
        }
        if (_test_bit(bitboards[WROOK], sq)) {
            return WROOK;
        }
        if (_test_bit(bitboards[WQUEEN], sq)) {
            return WQUEEN;
        }
        if (_test_bit(bitboards[WKING], sq)) {
            return WKING;
        }
    }
    else {
        if (_test_bit(bitboards[BPAWN], sq)) {
            return BPAWN;
        }
        if (_test_bit(bitboards[BKNIGHT], sq)) {
            return BKNIGHT;
        }
        if (_test_bit(bitboards[BBISHOP], sq)) {
            return BBISHOP;
        }
        if (_test_bit(bitboards[BROOK], sq)) {
            return BROOK;
        }
        if (_test_bit(bitboards[BQUEEN], sq)) {
            return BQUEEN;
        }
        if (_test_bit(bitboards[BKING], sq)) {
            return BKING;
        }
    }
    return -1;
}
inline int board::piece_type_at(int sq) {
    /* returns color specific int for piece*/
    bool white = false;

    if (_test_bit(bitboards[WPAWN], sq) or _test_bit(bitboards[BPAWN], sq)) {
        return PAWN;
    }
    if (_test_bit(bitboards[WKNIGHT], sq) or _test_bit(bitboards[BKNIGHT], sq)) {
        return KNIGHT;
    }
    if (_test_bit(bitboards[WBISHOP], sq) or _test_bit(bitboards[BBISHOP], sq)) {
        return BISHOP;
    }
    if (_test_bit(bitboards[WROOK], sq) or _test_bit(bitboards[BROOK], sq)) {
        return ROOK;
    }
    if (_test_bit(bitboards[WQUEEN], sq) or _test_bit(bitboards[BQUEEN], sq)) {
        return QUEEN;
    }
    if (_test_bit(bitboards[WKING], sq) or _test_bit(bitboards[BKING], sq)) {
        return KING;
    }
    return -1;
}
std::string board::piece_type(int piece) {
    if (piece == -1) {
        return "-";
    }
    if (piece == 0) {
        return "P";
    }
    if (piece == 1) {
        return "K";
    }
    if (piece == 2) {
        return "B";
    }
    if (piece == 3) {
        return "R";
    }
    if (piece == 4) {
        return "Q";
    }
    if (piece == 5) {
        return "K";
    }
    if (piece == 6) {
        return "p";
    }
    if (piece == 7) {
        return "k";
    }
    if (piece == 8) {
        return "b";
    }
    if (piece == 9) {
        return "r";
    }
    if (piece == 10) {
        return "q";
    }
    if (piece == 11) {
        return "k";
    }
    else {
        return "??";
    }
}

inline int board::square_file(int sq) {
    //Gets the file index of the square where 0 is the a-file
    return sq & 7;
}
inline int board::square_rank(int sq) {
    //Gets the rank index of the square where 0 is the first rank."""
    return sq >> 3;
}
inline bool board::is_square_attacked(int sq, int cast_check) {
    bool white = false;
    U64 enemy;
    U64 us;
    if (cast_check > -1) {
        if (cast_check == 1) {
            white = true;
            us = occupancies[0];
            enemy = occupancies[1];
        }
        if (cast_check == 0) {
            us = occupancies[1];
            enemy = occupancies[0];
        }
    }
    else {
        if (piece_color(sq)) {
            us = occupancies[0];
            enemy = occupancies[1];
            white = true;
        }
        else {
            us = occupancies[1];
            enemy = occupancies[0];
        }
    }

    unsigned long index;

    if (white) {
        if (_rays[NORTH][sq] & enemy) {
            index = _bitscanforward(enemy & _rays[NORTH][sq]);
            if (_test_bit(bitboards[BROOK], index) or _test_bit(bitboards[BQUEEN], index)) {
                // If this is true the ray is not hitting a piece from us
                if (not(_rays[NORTH][sq] & us)) {
                    return true;
                }
                else {
                    // We hit a piece from us. If the distance to it is smaller than to the blocker we cannot block the check
                    int blockedus = _bitscanforward(_rays[NORTH][sq] & us);
                    if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                        return true;
                    }
                }
            }
        }

        if (_rays[SOUTH][sq] & enemy) {
            index = _bitscanreverse(enemy & _rays[SOUTH][sq]);
            if (_test_bit(bitboards[BROOK], index) or _test_bit(bitboards[BQUEEN], index)) {
                if (not(_rays[SOUTH][sq] & us)) {
                    return true;
                }
                else {
                    int blockedus = _bitscanreverse(_rays[SOUTH][sq] & us);
                    if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                        return true;
                    }
                }
            }
        }

        if (_rays[EAST][sq] & enemy) {
            index = _bitscanforward(enemy & _rays[EAST][sq]);
            if (_test_bit(bitboards[BROOK], index) or _test_bit(bitboards[BQUEEN], index)) {
                if (not(_rays[EAST][sq] & us)) {
                    return true;
                }
                else {
                    int blockedus = _bitscanforward(_rays[EAST][sq] & us);
                    if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                        return true;
                    }
                }
            }

        }
        if (_rays[WEST][sq] & enemy) {
            index = _bitscanreverse(enemy & _rays[WEST][sq]);
            if (_test_bit(bitboards[BROOK], index) or _test_bit(bitboards[BQUEEN], index)) {
                if (not(_rays[WEST][sq] & us)) {
                    return true;
                }
                else {
                    int blockedus = _bitscanreverse(_rays[WEST][sq] & us);
                    if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                        return true;
                    }
                }
            }
        }

        if (_rays[NORTH_WEST][sq] & enemy) {
            index = _bitscanforward(enemy & _rays[NORTH_WEST][sq]);
            if (_test_bit(bitboards[BPAWN], index) or _test_bit(bitboards[BKING], index)) {
                if (square_distance(sq, index) == 1) {
                    return true;
                }
            }
            if (_test_bit(bitboards[BBISHOP], index) or _test_bit(bitboards[BQUEEN], index)) {
                if (not(_rays[NORTH_WEST][sq] & us)) {
                    return true;
                }
                else {
                    int blockedus = _bitscanforward(_rays[NORTH_WEST][sq] & us);
                    if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                        return true;
                    }
                }
            }
        }

        if (_rays[NORTH_EAST][sq] & enemy) {
            index = _bitscanforward(enemy & _rays[NORTH_EAST][sq]);
            if (_test_bit(bitboards[BPAWN], index) or _test_bit(bitboards[BKING], index)) {
                if (square_distance(sq, index) == 1) {
                    return true;
                }
            }
            if (_test_bit(bitboards[BBISHOP], index) or _test_bit(bitboards[BQUEEN], index)) {
                if (not(_rays[NORTH_EAST][sq] & us)) {
                    return true;
                }
                else {
                    int blockedus = _bitscanforward(_rays[NORTH_EAST][sq] & us);
                    if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                        return true;
                    }
                }
            }
        }

        if (_rays[SOUTH_WEST][sq] & enemy) {
            index = _bitscanreverse(enemy & _rays[SOUTH_WEST][sq]);
            if (_test_bit(bitboards[BKING], index)) {
                if (square_distance(sq, index) == 1) {
                    return true;
                }
            }
            if (_test_bit(bitboards[BBISHOP], index) or _test_bit(bitboards[BQUEEN], index)) {
                if (not(_rays[SOUTH_WEST][sq] & us)) {
                    return true;
                }
                else {
                    int blockedus = _bitscanreverse(_rays[SOUTH_WEST][sq] & us);
                    if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                        return true;
                    }
                }
            }
        }

        if (_rays[SOUTH_EAST][sq] & enemy) {
            index = _bitscanreverse(enemy & _rays[SOUTH_EAST][sq]);
            if (_test_bit(bitboards[BKING], index)) {
                if (square_distance(sq, index) == 1) {
                    return true;
                }
            }
            if (_test_bit(bitboards[BBISHOP], index) or _test_bit(bitboards[BQUEEN], index)) {
                if (not(_rays[SOUTH_EAST][sq] & us)) {
                    return true;
                }
                else {
                    int blockedus = _bitscanreverse(_rays[SOUTH_EAST][sq] & us);
                    if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                        return true;
                    }
                }
            }
        }
        if (knightattacks[sq] & (knights & enemy)) {
            return true;
        }
    }
    else {
        //BLACK
        if (_rays[NORTH][sq] & enemy) {
            index = _bitscanforward(enemy & _rays[NORTH][sq]);
            if (_test_bit(bitboards[WROOK], index) or _test_bit(bitboards[WQUEEN], index)) {
                if (not(_rays[NORTH][sq] & us)) {
                    return true;
                }
                else {
                    int blockedus = _bitscanforward(_rays[NORTH][sq] & us);
                    if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                        return true;
                    }
                }
            }
        }

        if (_rays[SOUTH][sq] & enemy) {
            index = _bitscanreverse(enemy & _rays[SOUTH][sq]);
            if (_test_bit(bitboards[WROOK], index) or _test_bit(bitboards[WQUEEN], index)) {
                if (not(_rays[SOUTH][sq] & us)) {
                    return true;
                }
                else {
                    int blockedus = _bitscanreverse(_rays[SOUTH][sq] & us);
                    if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                        return true;
                    }
                }
            }

        }

        if (_rays[EAST][sq] & enemy) {
            index = _bitscanforward(enemy & _rays[EAST][sq]);
            if (_test_bit(bitboards[WROOK], index) or _test_bit(bitboards[WQUEEN], index)) {
                if (not(_rays[EAST][sq] & us)) {
                    return true;
                }
                else {
                    int blockedus = _bitscanforward(_rays[EAST][sq] & us);
                    if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                        return true;
                    }
                }
            }

        }

        if (_rays[WEST][sq] & enemy) {
            index = _bitscanreverse(enemy & _rays[WEST][sq]);
            if (_test_bit(bitboards[WROOK], index) or _test_bit(bitboards[WQUEEN], index)) {
                if (not(_rays[WEST][sq] & us)) {
                    return true;
                }
                else {
                    int blockedus = _bitscanreverse(_rays[WEST][sq] & us);
                    if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                        return true;
                    }
                }
            }
        }

        if (_rays[NORTH_WEST][sq] & enemy) {
            index = _bitscanforward(enemy & _rays[NORTH_WEST][sq]);
            if (_test_bit(bitboards[WKING], index)) {
                if (square_distance(sq, index) == 1) {
                    return true;
                }
            }
            if (_test_bit(bitboards[WBISHOP], index) or _test_bit(bitboards[WQUEEN], index)) {
                if (not(_rays[NORTH_WEST][sq] & us)) {
                    return true;
                }
                else {
                    int blockedus = _bitscanforward(_rays[NORTH_WEST][sq] & us);
                    if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                        return true;
                    }
                }
            }
        }

        if (_rays[NORTH_EAST][sq] & enemy) {
            index = _bitscanforward(enemy & _rays[NORTH_EAST][sq]);
            if (_test_bit(bitboards[WKING], index)) {
                if (square_distance(sq, index) == 1) {
                    return true;
                }
            }
            if (_test_bit(bitboards[WBISHOP], index) or _test_bit(bitboards[WQUEEN], index)) {
                if (not(_rays[NORTH_EAST][sq] & us)) {
                    return true;
                }
                else {
                    int blockedus = _bitscanforward(_rays[NORTH_EAST][sq] & us);
                    if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                        return true;
                    }
                }
            }
        }

        if (_rays[SOUTH_WEST][sq] & enemy) {
            index = _bitscanreverse(enemy & _rays[SOUTH_WEST][sq]);
            if (_test_bit(bitboards[WPAWN], index) or _test_bit(bitboards[WKING], index)) {
                if (square_distance(sq, index) == 1) {
                    return true;
                }
            }
            if (_test_bit(bitboards[WBISHOP], index) or _test_bit(bitboards[WQUEEN], index)) {
                if (not(_rays[SOUTH_WEST][sq] & us)) {
                    return true;
                }
                else {
                    int blockedus = _bitscanreverse(_rays[SOUTH_WEST][sq] & us);
                    if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                        return true;
                    }
                }
            }
        }

        if (_rays[SOUTH_EAST][sq] & enemy) {
            index = _bitscanreverse(enemy & _rays[SOUTH_EAST][sq]);
            if (_test_bit(bitboards[WPAWN], index) or _test_bit(bitboards[WKING], index)) {
                if (square_distance(sq, index) == 1) {
                    return true;
                }
            }
            if (_test_bit(bitboards[WBISHOP], index) or _test_bit(bitboards[WQUEEN], index)) {
                if (not(_rays[SOUTH_EAST][sq] & us)) {
                    return true;
                }
                else {
                    int blockedus = _bitscanreverse(_rays[SOUTH_EAST][sq] & us);
                    if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                        return true;
                    }
                }
            }
        }
        if (knightattacks[sq] & (knights & enemy)) {
            return true;
        }
    }
    return false;

}
inline int board::square_distance(int a, int b) {
    /* 0 if a=b otherwise >0*/
    return std::max(abs(square_file(a) - square_file(b)), abs(square_rank(a) - square_rank(b)));
}
inline int board::distance_to_edge_right(int sq) {
    /* 0 if h file */
    return (7 - square_file(sq));
}
inline int board::distance_to_edge_left(int sq) {
    /* 0 if a file*/
    return square_file(sq);
}
inline bool board::horizontal_pawn_pin(int sq) {
    U64 us;
    U64 enemy;
    if (_test_bit(occupancies[0], sq)) {
        us = occupancies[0] ;
        enemy = occupancies[1] & (rooks | queens);
        if (_rays[EAST][sq] & both & enemy and _rays[WEST][sq] & us) {
            int index = _bitscanforward(_rays[EAST][sq] & both & enemy);
            int index2 = _bitscanreverse(_rays[WEST][sq] & us);
            if (not(piece_color(index)) and (_test_bit(bitboards[WKING], index2))) {
                return true;
            }
        }
        if (_rays[WEST][sq] & both & enemy and _rays[EAST][sq] & us) {
            int index = _bitscanreverse(_rays[WEST][sq] & both & enemy);
            int index2 = _bitscanforward(_rays[EAST][sq] & us);

            if (not(piece_color(index)) and (_test_bit(bitboards[WKING], index2))) {
                return true;
            }
        }
    }
    else {
        us = occupancies[1];
        enemy = occupancies[0] & (rooks | queens);
        if (_rays[EAST][sq] & both & enemy and _rays[WEST][sq] & us) {
            int index = _bitscanforward(_rays[EAST][sq] & both & enemy);
            int index2 = _bitscanreverse(_rays[WEST][sq] & us);
            if ((piece_color(index)) and ( _test_bit(bitboards[BKING], index2))) {
                return true;
            }
        }
        if (_rays[WEST][sq] & both & enemy and _rays[EAST][sq] & us) {
            int index = _bitscanreverse(_rays[WEST][sq] & both & enemy);
            int index2 = _bitscanforward(_rays[EAST][sq] & us);

            if ((piece_color(index)) and (_test_bit(bitboards[BKING], index2))) {
                return true;
            }
        }
    }

    return false;
}
inline BoardState board::encode_board_state(U64 wpawn, U64 wknight, U64 wbishop, U64 wrook, U64 wqueen, U64 wking,
                                            U64 bpawn, U64 bknight, U64 bbishop, U64 brook, U64 bqueen, U64 bking,
                                            int ep, int castle) 
{
BoardState board;
board.wpawn = wpawn;
board.wknight = wknight;
board.wbishop = wbishop;
board.wrook = wrook;
board.wqueen = wqueen;
board.wking = wking;
board.bpawn = bpawn;
board.bknight = bknight;
board.bbishop = bbishop;
board.brook = brook;
board.bqueen = bqueen;
board.bking = bking;
board.en_passant = ep;
board.castle_rights = castle;
return board;
}

inline U64 board::sliding_attacks(int square, int deltas[4]) {
    // only used for knight attacks
    U64 attacks = 0ULL;
    for (unsigned int i = 0; i < 4; i++) {
        int sq = square;
        sq += deltas[i];

        if ((0 <= sq and sq < 64) and (square_distance(sq, sq - deltas[i]) <= 2)) {
            attacks |= (1ULL << sq);
        }
        if ((square_distance(square, square - deltas[i])) <= 2  and (0 <= sq - (2 * deltas[i]) and sq - (2 * deltas[i]) < 64)) { 
            attacks |= 1ULL << (sq - (2 * deltas[i]));
        }
    }
    return attacks;
}
inline U64 board::pawn_attacks(int sq) {
    U64 attacks = 0;
    U64 bb = 0ULL;
    bb |= (1ULL << sq);
    if (piece_color(sq)) {
        U64 attacks_left = (bb << 7) & not_h_file;
        U64 attacks_right = (bb << 9) & not_a_file;
        attacks = attacks_left | attacks_right;
    }
    else {
        U64 attacks_left = (bb >> 7) & not_a_file;
        U64 attacks_right = (bb >> 9) & not_h_file;
        attacks = attacks_left | attacks_right;
    }
    return attacks;
}
inline U64 board::Valid_Moves_Pawn(int sq) {
    U64 pawn_move = 0ULL;
    U64 pawn_move_left = 0ULL;
    U64 pawn_move_right = 0ULL;
    U64 pawn_move_attack_union = 0ULL;
    U64 pawn_move_forward = 0ULL;
    U64 pawn_move_forward_double = 0ULL;
    U64 valid_move = 0ULL;
    pawn_move |= (1ULL << sq);

    if (piece_color(sq)) {
        //White Pawn
        pawn_move_left = (pawn_move << 7) & not_h_file;
        pawn_move_right = (pawn_move << 9) & not_a_file;
        pawn_move_attack_union = (pawn_move_left | pawn_move_right) & occupancies[1];
        // en passant
        if (en_passant_square != no_sq and square_rank(sq) == 4) {
            pawn_move_attack_union = ((pawn_move_left | pawn_move_right) & occupancies[1]) | ((1ULL << en_passant_square) & (pawn_move_left | pawn_move_right));
        }
        // North pawn push
        pawn_move_forward = pawn_move << 8 & ~both;
        pawn_move_forward_double = (pawn_move_forward << 8) & ~rank_2_mask & ~both;
        valid_move = (pawn_move_forward) | (pawn_move_forward_double) | (pawn_move_attack_union);
    }
    else {
        //Black Pawn
        pawn_move_left = (pawn_move >> 7) & not_a_file;
        pawn_move_right = (pawn_move >> 9) & not_h_file;
        pawn_move_attack_union = (pawn_move_left | pawn_move_right) & occupancies[0];
        // en passant
        if (en_passant_square != no_sq and square_rank(sq) == 3) {
            // Old implementation should be the same perft numbers are the same
            // just being here for my safety
            //pawn_move_attack_union = ((pawn_move_attack_union) | ((1ULL << en_passant_square) & (pawn_move_left | pawn_move_right))) & ~occupancies[1];
            pawn_move_attack_union = ((pawn_move_left | pawn_move_right) & occupancies[0])| ((1ULL << en_passant_square) & (pawn_move_left | pawn_move_right));
        }
        //South pawn push
        pawn_move_forward = pawn_move >> 8 & ~both;
        pawn_move_forward_double = (pawn_move_forward >> 8) & ~rank_7_mask & ~both;
        valid_move = (pawn_move_forward) | (pawn_move_forward_double) | pawn_move_attack_union;
    }
    return valid_move;
}
inline U64 board::Valid_Moves_Knight(int sq) {
    U64 knight_move;
    if(_test_bit(occupancies[0],sq)){
        knight_move = knightattacks[sq] & (occupancies[1] | ~occupancies[0]);
    }
    else {
        knight_move = knightattacks[sq] & (occupancies[0] | ~occupancies[1]);
    }
    return knight_move;
}
inline U64 board::Valid_Moves_Bishop(int sq) {
    U64 bishop_move = 0ULL;
    U64 victims;
    U64 blockers;
    bishop_move |= (1ULL << sq);
    

    //if (std::bitset<64>(occupancies[0]).test(sq) == 1) {
    if (_test_bit(occupancies[0], sq)) {
        victims = occupancies[1];
        blockers = occupancies[0];
    }
    else {
        victims = occupancies[0];
        blockers = occupancies[1];
    }
    int index;
    bishop_move |= _rays[NORTH_WEST][sq];

    if (_rays[NORTH_WEST][sq] & victims) {

        index = _bitscanforward(victims & _rays[NORTH_WEST][sq]);
        bishop_move &= ~_rays[NORTH_WEST][index];
    }
    if (_rays[NORTH_WEST][sq] & blockers) {
        index = _bitscanforward(blockers & _rays[NORTH_WEST][sq]);
        bishop_move &= ~_rays[NORTH_WEST][index - 7];
    }

    bishop_move |= _rays[NORTH_EAST][sq];
    if (_rays[NORTH_EAST][sq] & victims) {
        index = _bitscanforward(victims & _rays[NORTH_EAST][sq]);
        bishop_move &= ~_rays[NORTH_EAST][index];
    }
    if (_rays[NORTH_EAST][sq] & blockers) {
        index = _bitscanforward(blockers & _rays[NORTH_EAST][sq]);
        bishop_move &= ~_rays[NORTH_EAST][index - 9];
    }

    bishop_move |= _rays[SOUTH_WEST][sq];
    if (_rays[SOUTH_WEST][sq] & victims) {
        index = _bitscanreverse(victims & _rays[SOUTH_WEST][sq]);
        bishop_move &= ~_rays[SOUTH_WEST][index];
    }
    if (_rays[SOUTH_WEST][sq] & blockers) {
        index = _bitscanreverse(blockers & _rays[SOUTH_WEST][sq]);
        bishop_move &= ~_rays[SOUTH_WEST][index + 9];
    }

    bishop_move |= _rays[SOUTH_EAST][sq];
    if (_rays[SOUTH_EAST][sq] & victims) {
        index = _bitscanreverse(victims & _rays[SOUTH_EAST][sq]);
        bishop_move &= ~_rays[SOUTH_EAST][index];
    }
    if (_rays[SOUTH_EAST][sq] & blockers) {
        index = _bitscanreverse(blockers & _rays[SOUTH_EAST][sq]);
        bishop_move &= ~_rays[SOUTH_EAST][index + 7];
    }
    bishop_move &= ~(1ULL << sq);
    return bishop_move;
}
inline U64 board::Valid_Moves_Rook(int sq) {
    U64 rook_move = 0ULL;
    U64 victims;
    U64 blockers;

    rook_move |= (1ULL << sq);
    

    //if (std::bitset<64>(occupancies[0]).test(sq) == 1) {
    if (_test_bit(occupancies[0], sq)) {
        victims = occupancies[1];
        blockers = occupancies[0];
    }
    else {
        victims = occupancies[0];
        blockers = occupancies[1];
    }

    int index;
    rook_move |= _rays[NORTH][sq];

    if (_rays[NORTH][sq] & victims) {
        index = _bitscanforward(victims & _rays[NORTH][sq]);
        rook_move &= ~_rays[NORTH][index];
    }
    if (_rays[NORTH][sq] & blockers) {
        index = _bitscanforward(blockers & _rays[NORTH][sq]);
        rook_move &= ~_rays[NORTH][index - 8];
    }

    rook_move |= _rays[SOUTH][sq];
    if (_rays[SOUTH][sq] & victims) {
        index = _bitscanreverse(victims & _rays[SOUTH][sq]);
        rook_move &= ~_rays[SOUTH][index];
    }
    if (_rays[SOUTH][sq] & blockers) {
        index = _bitscanreverse(blockers & _rays[SOUTH][sq]);
        rook_move &= ~_rays[SOUTH][index + 8];
    }

    rook_move |= _rays[EAST][sq];
    if (_rays[EAST][sq] & victims) {
        index = _bitscanforward(victims & _rays[EAST][sq]);
        rook_move &= ~_rays[EAST][index];

    }
    if (_rays[EAST][sq] & blockers) {
        index = _bitscanforward(blockers & _rays[EAST][sq]);
        rook_move &= ~_rays[EAST][index - 1];
    }

    rook_move |= _rays[WEST][sq];
    if (_rays[WEST][sq] & victims) {
        index = _bitscanreverse(victims & _rays[WEST][sq]);
        rook_move &= ~_rays[WEST][index];
    }
    if (_rays[WEST][sq] & blockers) {
        index = _bitscanreverse(blockers & _rays[WEST][sq]);
        rook_move &= ~_rays[WEST][index + 1];
    }
    rook_move &= ~(1ULL << sq);
    return rook_move;
}
inline U64 board::Valid_Moves_Queen(int sq) {
    U64 queen_move = 0ULL;
    queen_move |= (1ULL << sq);
    queen_move |= Valid_Moves_Bishop(sq);
    queen_move |= Valid_Moves_Rook(sq);
    queen_move &= ~(1ULL << sq);
    return queen_move;
}
inline U64 board::Valid_Moves_King(int sq) {
    U64 king_move = 0ULL;
    king_move |= (1ULL << sq);

    U64 king_up = king_move << 8 & ~rank_1_mask;
    U64 king_down = king_move >> 8 & ~rank_8_mask;
    U64 king_right = king_move << 1 & not_a_file;
    U64 king_left = king_move >> 1 & not_h_file;
    U64 king_up_right = king_move << 9 & not_a_file;
    U64 king_up_left = king_move << 7 & not_h_file;
    U64 king_down_right = king_move >> 7 & not_a_file;
    U64 king_down_left = (king_move >> 9) & not_h_file;
    U64 king_valid_move;
    U64 blockers = both;

    
    if (piece_color(sq)) {
        king_valid_move = (king_up | king_down | king_right | king_left | king_up_right | king_up_left | king_down_right | king_down_left) & ~occupancies[0];
    }
    else {
        king_valid_move = (king_up | king_down | king_right | king_left | king_up_right | king_up_left | king_down_right | king_down_left) & ~occupancies[1];
    }

    // Castling
    unsigned long rook_index;
    //King side White
    if (castling_rights & wk and (_rays[EAST][sq] & blockers) and (piece_color(sq))) {
        rook_index = _bitscanforward(blockers & _rays[EAST][sq]);
        if (rook_index == 7) {
            if (not is_square_attacked(4, 1) and not is_square_attacked(5, 1) and not is_square_attacked(6, 1)) {
                U64 to_index = 0ULL;
                to_index |= 1ULL << 6;
                king_valid_move |= to_index;
            }
        }
    }
    //Queen side White
    if (castling_rights & wq and (_rays[WEST][sq] & blockers) and (piece_color(sq))) {
        rook_index = _bitscanreverse(blockers & _rays[WEST][sq]);
        if (rook_index == 0) {
            if (not is_square_attacked(4, 1) and not is_square_attacked(3, 1) and not is_square_attacked(2, 1)) {
                U64 to_index = 0ULL;
                to_index |= 1ULL << 2;
                king_valid_move |= to_index;
            }
        }
    }
    //King side Black
    if (castling_rights & bk and (_rays[EAST][sq] & blockers) and not (piece_color(sq))) {
        rook_index = _bitscanforward(blockers & _rays[EAST][sq]);
        if (rook_index == 63) {
            if (not is_square_attacked(62,0) and not is_square_attacked(61,0) and not is_square_attacked(60,0)) {
                U64 to_index = 0ULL;
                to_index |= 1ULL << 62;
                king_valid_move |= to_index;
            }
        }
    }
    //Queen side Black
    if (castling_rights & bq and (_rays[WEST][sq] & blockers) and not (piece_color(sq))) {
        rook_index = _bitscanreverse(blockers & _rays[WEST][sq]);
        if (rook_index == 56) {
            if (not is_square_attacked(60,0) and not is_square_attacked(59,0) and not is_square_attacked(58,0)) {
                U64 to_index = 0ULL;
                to_index |= 1ULL << 58;
                king_valid_move |= to_index;
            }
        }
    }
    return king_valid_move;
};

inline void board::make_move(Move &move) {

    int from_square = move.from_square;
    int to_square = move.to_square;
    int promotion_piece = move.promotion;
    int piece = move.piece;
    piece = piece + (side_to_move * 6);
    int captured_piece = -1;
    // piece needs to be set at its bitboard remove this for performance if you are 100% theres a piece at that square
    if (not _test_bit(bitboards[piece], to_square)) {
        BoardState boardstate = encode_board_state(bitboards[0], bitboards[1], bitboards[2], bitboards[3], bitboards[4], bitboards[5], bitboards[6], bitboards[7], bitboards[8], bitboards[9], bitboards[10], bitboards[11], en_passant_square, castling_rights);
        move_stack[move_stack_index] = boardstate;
        move_stack_index++;
        int test = side_to_move ^ 1;
        //Capture
        if (_test_bit(occupancies[test], to_square)) {
            for (int i = 0; i < 6; i++) {
                if (_test_bit(bitboards[i + (test * 6)], to_square)) {
                    //Capturing rook loses others side castle rights
                    captured_piece = i + (test * 6);
                    if (captured_piece == WROOK) {
                        if (to_square == 7) {
                            if (castling_rights & wk) {
                                castling_rights ^= wk;
                            }
                        }
                        if (to_square == 0) {
                            if (castling_rights & wq) {
                                castling_rights ^= wq;
                            }
                        }
                    }
                    if (captured_piece == BROOK) {
                        if (to_square == 63) {
                            if (castling_rights & bk) {
                                castling_rights ^= bk;
                            }

                        }
                        if (to_square == 56) {
                            if (castling_rights & bq) {
                                castling_rights ^= bq;
                            }
                        }
                    }
                    break;
                }
            }
        }

        // King move loses castle rights
        if (piece == WKING) {
            if (to_square != 6 and to_square != 2) {
                if (castling_rights & wq) {
                    castling_rights ^= wq;
                }
                if (castling_rights & wk) {
                    castling_rights ^= wk;
                }
            }
        }
        if (piece == BKING) {
            if (to_square != 62 and to_square != 58) {
                if (castling_rights & bq) {
                    castling_rights ^= bq;
                }
                if (castling_rights & bk) {
                    castling_rights ^= bk;
                }
            }

        }
        // Rook move loses castle rights
        if (piece == WROOK) {
            if (from_square == 7 and castling_rights & wk) {
                castling_rights ^= wk;
            }
            if (from_square == 0 and castling_rights & wq) {
                castling_rights ^= wq;
            }
        }
        if (piece == BROOK) {
            if (from_square == 63 and castling_rights & bk) {
                castling_rights ^= bk;
            }
            if (from_square == 56 and castling_rights & bq) {
                castling_rights ^= bq;
            }
        }

        // Actual sastling
        if (piece == WKING and square_distance(from_square, to_square) == 2) {
            if (to_square == 6) {
                if (castling_rights & wk) {
                    castling_rights ^= wk;
                    bitboards[WROOK] &= ~(1ULL << 7);
                    bitboards[WROOK] |= (1ULL << 5);
                }

            }
            else {
                if (castling_rights & wq) {
                    castling_rights ^= wq;
                    bitboards[WROOK] &= ~(1ULL << 0);
                    bitboards[WROOK] |= (1ULL << 3);
                }
            }
        }
        if (piece == BKING and square_distance(from_square, to_square) == 2) {
            if (to_square == 62) {
                castling_rights ^= bk;
                bitboards[BROOK] &= ~(1ULL << 63);
                bitboards[BROOK] |= (1ULL << 61);
            }
            else {
                castling_rights ^= bq;
                bitboards[BROOK] &= ~(1ULL << 56);
                bitboards[BROOK] |= (1ULL << 59);
            }
        }
        // Remove enemy piece if en passant capture
        if ((abs(from_square - to_square) == 7 or abs(from_square - to_square) == 9)) {
            if (piece == WPAWN and square_rank(to_square) == 5 and piece_at(to_square - 8) == BPAWN and piece_at(to_square) == -1) { //
                en_passant_square = no_sq;
                bitboards[BPAWN] &= ~(1ULL << (to_square - 8));
            }
            if (piece == BPAWN and square_rank(to_square) == 2 and piece_at(to_square + 8) == WPAWN and piece_at(to_square) == -1) {
                en_passant_square = no_sq;
                bitboards[WPAWN] &= ~(1ULL << (to_square + 8));
            }
        }
        // remove en passant if it wasnt played immediately
        if (en_passant_square != no_sq) {
            en_passant_square = no_sq;
        }
        // set en passant square if pawns double move
        if (piece == WPAWN and abs(from_square - to_square) == 16) {
            en_passant_square = from_square + 8;
        }
        if (piece == BPAWN and abs(from_square - to_square) == 16) {
            en_passant_square = from_square - 8;
        }

        // Remove and set piece
        bitboards[piece] &= ~(1ULL << from_square);
        bitboards[piece] |= (1ULL << to_square);
        // Remove captured piece
        if (captured_piece != -1) {
            bitboards[captured_piece] &= ~(1ULL << to_square);
        }

        // Promotion
        if (promotion_piece > 0 and promotion_piece < 7) {
            bitboards[piece] &= ~(1ULL << to_square);
            promotion_piece = promotion_piece + (side_to_move * 6);

            bitboards[promotion_piece] |= (1ULL << to_square);
        }
        side_to_move ^= 1;
        update_occupancies();
    }
    else {
        std::cout << "Not valid move" << std::endl;
    }
};
inline void board::unmake_move() {
    if (move_stack_index >= 0) {
        move_stack_index--;
        BoardState board;
        board = move_stack[move_stack_index];
        bitboards[0] = board.wpawn;
        bitboards[1] = board.wknight;
        bitboards[2] = board.wbishop;
        bitboards[3] = board.wrook;
        bitboards[4] = board.wqueen;
        bitboards[5] = board.wking;
        bitboards[6] = board.bpawn;
        bitboards[7] = board.bknight;
        bitboards[8] = board.bbishop;
        bitboards[9] = board.brook;
        bitboards[10] = board.bqueen;
        bitboards[11] = board.bking;
        en_passant_square = board.en_passant;
        castling_rights = board.castle_rights;
        side_to_move ^= 1;
        update_occupancies();
    }
    //No need to remove the entry because it will be overwritten 
}
MoveList board::generate_moves() {
    int pawn_moves = 0;
    int knight_moves = 0;
    int bishop_moves = 0;
    int rook_moves = 0;
    int queen_moves = 0;
    MoveList possible_moves{};
    int index = 0;
    U64 we;
    Move move;
    int enemy = side_to_move ^ 1;
    if (side_to_move == 0) {
        we = occupancies[0];
    }
    else {
        we = occupancies[1];
    }
    U64 pawn_mask = pawns & we;
    U64 knight_mask = knights & we;
    U64 bishop_mask = bishops & we;
    U64 rook_mask = rooks & we;
    U64 queen_mask = queens & we;
    U64 king_mask = kings & we;
    unsigned long king_sq = _bitscanforward(king_mask);
    count = 0;
    while (king_mask) {
        king_sq = _bitscanforward(king_mask);
        U64 move_mask = Valid_Moves_King(king_sq);
        while(move_mask) {
            unsigned long to_index = _bitscanforward(move_mask);
            if (occupancies[enemy]) {
                move.capture = piece_at(to_index, enemy);
            }
            else {
                move.capture = -1;
            }
            move.piece = KING;
            move.from_square = king_sq;
            move.to_square = to_index;
            move.promotion = -1;
            make_move(move);
            if (not(is_square_attacked(to_index, side_to_move))) {
                possible_moves.movelist[count] = move;
                count++;
            }
            unmake_move();
            move_mask = _blsr_u64(move_mask);
        }
        king_mask = _blsr_u64(king_mask);
    }
    king_sq = _bitscanforward(kings & we);
    unsigned long pawn_index;
    while (pawn_mask) {
        pawn_index = _bitscanforward(pawn_mask);
        U64 move_mask = Valid_Moves_Pawn(pawn_index);
        while (move_mask) {
            unsigned long to_index = _bitscanforward(move_mask);
            if (occupancies[enemy]) {
                move.capture = piece_at(to_index, enemy);
            }
            else {
                move.capture = -1;
            }
            move.piece = PAWN;
            move.from_square = pawn_index;
            move.to_square = to_index;
            make_move(move);
            if (not(is_square_attacked(king_sq))) {
                // Promotion
                if (square_rank(to_index) == 7 or square_rank(to_index) == 0) {
                    move.promotion = QUEEN;
                    possible_moves.movelist[count] = move;
                    count++;
                    move.promotion = ROOK;
                    possible_moves.movelist[count] = move;
                    count++;
                    move.promotion = KNIGHT;
                    possible_moves.movelist[count] = move;
                    count++;
                    move.promotion = BISHOP;
                    possible_moves.movelist[count] = move;
                    count++;
                }
                else {
                    move.promotion = -1;
                    possible_moves.movelist[count] = move;
                    count++;
                }
                pawn_moves++;
                index++;
            }
            unmake_move();
            move_mask = _blsr_u64(move_mask);
        }
        pawn_mask = _blsr_u64(pawn_mask);
    }
    unsigned long knight_index;
    while (knight_mask) {
        knight_index = _bitscanforward(knight_mask);
        U64 move_mask = Valid_Moves_Knight(knight_index);
        while (move_mask) {
            unsigned long to_index = _bitscanforward(move_mask);
            if (occupancies[enemy]) {
                move.capture = piece_at(to_index, enemy);
            }
            else {
                move.capture = -1;
            }
            move.piece = KNIGHT;
            move.from_square = knight_index;
            move.to_square = to_index;
            move.promotion = -1;
            make_move(move);
            if (not(is_square_attacked(king_sq, side_to_move))) {
                possible_moves.movelist[count] = move;
                count++;
                knight_moves++;
                index++;
            }
            unmake_move();
            move_mask = _blsr_u64(move_mask);
        }
        knight_mask = _blsr_u64(knight_mask);
    }
    unsigned long bishop_index;
    while (bishop_mask) {
        bishop_index = _bitscanforward(bishop_mask);
        U64 move_mask = Valid_Moves_Bishop(bishop_index);
        while (move_mask) {
            unsigned long to_index = _bitscanforward(move_mask);
            if (occupancies[enemy]) {
                move.capture = piece_at(to_index, enemy);
            }
            else {
                move.capture = -1;
            }
            move.piece = BISHOP;
            move.from_square = bishop_index;
            move.to_square = to_index;
            move.promotion = -1;
            make_move(move);
            if (not(is_square_attacked(king_sq, side_to_move))) {
                possible_moves.movelist[count] = move;
                count++;
                bishop_moves++;
                index++;
            }
            unmake_move();
            move_mask = _blsr_u64(move_mask);
        }
        bishop_mask = _blsr_u64(bishop_mask);
    }
    unsigned long rook_index;
    while (rook_mask) {
        rook_index = _bitscanforward(rook_mask);
        U64 move_mask = Valid_Moves_Rook(rook_index);
        while (move_mask) {
            unsigned long to_index = _bitscanforward(move_mask);
            if (occupancies[enemy]) {
                move.capture = piece_at(to_index, enemy);
            }
            else {
                move.capture = -1;
            }
            move.piece = ROOK;
            move.from_square = rook_index;
            move.to_square = to_index;
            move.promotion = -1;
            make_move(move);
            if (not(is_square_attacked(king_sq, side_to_move))) {
                possible_moves.movelist[count] = move;
                count++;
                rook_moves++;
                index++;
            }
            unmake_move();
            move_mask = _blsr_u64(move_mask);
        }
        rook_mask = _blsr_u64(rook_mask);
    }
    unsigned long queen_index;
    while (queen_mask) {
        queen_index = _bitscanforward(queen_mask);
        U64 move_mask = Valid_Moves_Queen(queen_index);
        while (move_mask) {
            unsigned long to_index = _bitscanforward(move_mask);
            if (occupancies[enemy]) {
                move.capture = piece_at(to_index, enemy);
            }
            else {
                move.capture = -1;
            }
            move.piece = QUEEN;
            move.from_square = queen_index;
            move.to_square = to_index;
            move.promotion = -1;
            make_move(move);
            if (not(is_square_attacked(king_sq, side_to_move))) {
                possible_moves.movelist[count] = move;
                count++;
                queen_moves++;
                index++;
            }
            unmake_move();
            move_mask = _blsr_u64(move_mask);
        }
        queen_mask = _blsr_u64(queen_mask);
    }
    return possible_moves;
}
void board::init() {
    std::cout << " " << horizontal_pawn_pin(53) << " ";
}
void board::print_bitboard(std::bitset<64> bitset) {
    std::string str_bitset = bitset.to_string();
    int n = 0;
    for (int i = 0; i < 64; i += 8)
    {
        std::string x = str_bitset.substr(i, 8);
        reverse(x.begin(), x.end());
        std::cout << x << std::endl;
        n += 8;
    }
    std::cout << '\n' << std::endl;
}

void board::print_board() {
    for (int i = 63; i >= 0; i -= 8)
    {
        std::cout << " " << piece_type(piece_at(i-7)) << " " << piece_type(piece_at(i-6)) << " " << piece_type(piece_at(i-5)) << " " << piece_type(piece_at(i-4)) << " " << piece_type(piece_at(i-3)) << " " << piece_type(piece_at(i-2)) << " " << piece_type(piece_at(i-1)) << " " << piece_type(piece_at(i)) << " " << std::endl;
    }
    std::cout << '\n' << std::endl;
}

U64 Perft::speed_test_perft(int depth) {
    U64 nodes = 0;
    MoveList n_moves = this_board.generate_moves();
    if (depth == 0) {
        return 1;
    }
    else {
        int count = this_board.count;
        for (int i = 0; i < count; i++) {
            Move move = n_moves.movelist[i];
            this_board.make_move(move);
            nodes += speed_test_perft(depth - 1);
            this_board.unmake_move();
        }
    }
    return nodes;
}
U64 Perft::bulk_perft(int depth, int max) {
    U64 nodes = 0;
    MoveList n_moves = this_board.generate_moves();
    MA myarray;
    Pertft_Info pf;
    if (depth == 1) {
        return this_board.count;
    }
    else {
        int count = this_board.count;
        for (int i = 0; i < count; i++) {
            Move move = n_moves.movelist[i];
            this_board.make_move(move);
            nodes += bulk_perft(depth - 1, max);
            this_board.unmake_move();
            if (depth == max) {
                pf.from_square = move.from_square;
                pf.to_square = move.to_square;
                pf.promotion_piece = move.promotion;
                pf.nodes = nodes;
                myarray.myarray.push_back(pf);
                nodes = 0;
            }
        }
    }
    U64 c = 0;
    for (int i = 0; i < myarray.myarray.size(); i++)
    {
        std::cout << square_to_coordinates_perft[myarray.myarray[i].from_square];
        std::cout << square_to_coordinates_perft[myarray.myarray[i].to_square];
        if (myarray.myarray[i].promotion_piece != -1) {
            std::cout << " " << piece_type(myarray.myarray[i].promotion_piece) << " ";
        }
        else {
            std::cout << " ";
        }
        std::cout << myarray.myarray[i].nodes;
        std::cout << std::endl;
        c += myarray.myarray[i].nodes;
        
    }
    if (depth != max) {
        return nodes;
    }
    else {
        return c;
    }
}

int perft_test(std::string fen, int depth) {
    board board;
    board.apply_fen(fen);
    Perft perft(board);
    auto begin = std::chrono::high_resolution_clock::now();
    U64 x = perft.bulk_perft(depth, depth);
    auto end = std::chrono::high_resolution_clock::now();
    auto time_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
    std::cout << "fen " << fen << " nodes " << x << " nps " << x / (time_diff / 1000000000.0f) << " time " << time_diff / 1000000000.0f << " seconds" << std::endl;
    return x & 18446744073709551615ULL;
}

int test() {
    std::string fen1 = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::string fen2 = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::string fen3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
    std::string fen4 = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    std::string fen5 = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
    std::string fen6 = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ";
    auto begin = std::chrono::high_resolution_clock::now();

    if (perft_test(fen1, 6) == 119060324) { // 4 == 197281     5 == 4865609    6 == 119060324	
        std::cout << "\n" << "Correct" << "\n" << std::endl;
    }
    else {
        std::cout << "False" << std::endl;
    };

    if (perft_test(fen2, 5) == 193690690) { // 4 == 4085603      3 == 97862       5 == 193690690
        std::cout << "\n" << "Correct" << "\n" << std::endl;
    }
    else {
        std::cout << "False" << std::endl;
    };
    if (perft_test(fen3, 6) == 11030083) {    // 6 == 11030083        5 == 674624
        std::cout << "\n" << "Correct" << "\n" << std::endl;
    }
    else {
        std::cout << "False" << std::endl;
    };
    if (perft_test(fen4, 5) == 15833292) {    // 5 == 15833292        4 == 422333
        std::cout << "\n" << "Correct" << "\n" << std::endl;
    }
    else {
        std::cout << "False" << std::endl;
    };
    if (perft_test(fen5, 5) == 89941194) {   //5 == 89941194    4 ==2103487
        std::cout << "\n" << "Correct" << "\n" << std::endl;
    }
    else {
        std::cout << "False" << std::endl;
    };
    if (perft_test(fen6, 4) == 3894594) {     // 4 == 3894594     3 == 89890
        std::cout << "\n" << "Correct" << "\n" << std::endl;
    }
    else {
        std::cout << "False" << std::endl;
    };
    std::cout << "Finished perft positions" << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    auto time_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
    std::cout << time_diff / 1000000000.0f << " seconds" << std::endl;
    return 0;
}