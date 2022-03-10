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
inline int board::square_file(int sq) {
    //Gets the file index of the square where 0 is the a-file
    return sq & 7;
}
inline int board::square_rank(int sq) {
    //Gets the rank index of the square where 0 is the first rank."""
    return sq >> 3;
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
U64 board::sliding_attacks(int square, int deltas[4]) {
    // only used for knight attacks
    U64 attacks = 0ULL;
    for (unsigned int i = 0; i < 4; i++) {
        int sq = square;
        sq += deltas[i];

        if ((0 <= sq and sq < 64) and (square_distance(sq, sq - deltas[i]) <= 2)) {
            attacks |= (1ULL << sq);
        }
        if ((0 <= sq - (2 * deltas[i]) and sq - (2 * deltas[i]) < 64) and (square_distance(square, square - deltas[i])) <= 2) {
            attacks |= 1ULL << (sq - (2 * deltas[i]));
        }
    }
    return attacks;
}
U64 board::pawn_attacks(int sq) {
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
            pawn_move_attack_union |= (1ULL << en_passant_square) & (pawn_move_left | pawn_move_right);
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
            pawn_move_attack_union |= (1ULL << en_passant_square) & (pawn_move_left | pawn_move_right);
        }
        //South pawn push
        pawn_move_forward = pawn_move >> 8 & ~both;
        pawn_move_forward_double = (pawn_move_forward >> 8) & ~rank_7_mask & ~both;
        valid_move = (pawn_move_forward) | (pawn_move_forward_double) | pawn_move_attack_union;
    }
    return valid_move;
}
inline U64 board::Valid_Moves_Knight(int sq) {
    U64 knight_move = 0ULL;
    int deltas[4] = { 17,15,10,6 };
    knight_move = sliding_attacks(sq, deltas);
    //if (std::bitset<64>(occupancies[0]).test(sq) == 1) {
    if(_test_bit(occupancies[0],sq)){
        knight_move = knight_move & (occupancies[1] | ~occupancies[0]);
    }
    else {
        knight_move = knight_move & (occupancies[0] | ~occupancies[1]);
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
    if ((_rays[EAST][sq] & blockers) and castling_rights & wk and (piece_color(sq))) {
        rook_index = _bitscanforward(blockers & _rays[EAST][sq]);
        if (rook_index == 7) {
            if (not in_check(4, 1) and not in_check(5, 1) and not in_check(6, 1)) {
                U64 to_index = 0ULL;
                to_index |= 1ULL << 6;
                king_valid_move |= to_index;
            }
        }
    }
    //King side Black
    if ((_rays[EAST][sq] & blockers) and castling_rights & bk and not (piece_color(sq))) {
        rook_index = _bitscanforward(blockers & _rays[EAST][sq]);
        if (rook_index == 63) {
            if (not in_check(62,0) and not in_check(61,0) and not in_check(60,0)) {
                U64 to_index = 0ULL;
                to_index |= 1ULL << 62;
                king_valid_move |= to_index;
            }
        }
    }
    //Queen side White
    if ((_rays[WEST][sq] & blockers) and castling_rights & wq and (piece_color(sq))) { 
        rook_index = _bitscanreverse(blockers & _rays[WEST][sq]);
        if (rook_index == 0) {
            if (not in_check(4,1) and not in_check(3,1) and not in_check(2,1)) {
                U64 to_index = 0ULL;
                to_index |= 1ULL << 2;
                king_valid_move |= to_index;
            }
        }
    }
    //Queen side Black
    if ((_rays[WEST][sq] & blockers) and castling_rights & bq and not (piece_color(sq))) {
        rook_index = _bitscanreverse(blockers & _rays[WEST][sq]);
        if (rook_index == 56) {
            if (not in_check(60,0) and not in_check(59,0) and not in_check(58,0)) {
                U64 to_index = 0ULL;
                to_index |= 1ULL << 58;
                king_valid_move |= to_index;
            }
        }
    }
    return king_valid_move;
};
inline bool board::in_check(int sq, int cast_check) {
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
            if (piece_at(index, 0) == BROOK or piece_at(index, 0) == BQUEEN) {
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
            if (piece_at(index, 0) == BROOK or piece_at(index, 0) == BQUEEN) {
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
            if (piece_at(index, 0) == BROOK or piece_at(index, 0) == BQUEEN) {
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
            if (piece_at(index, 0) == BROOK or piece_at(index, 0) == BQUEEN) {
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
            if (piece_at(index, 0) == BPAWN or piece_at(index, 0) == BKING) {
                if (square_distance(sq, index) == 1) {
                    return true;
                }
            }
            if (piece_at(index, 0) == BBISHOP or piece_at(index, 0) == BQUEEN) {
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
            if (piece_at(index, 0) == BPAWN or piece_at(index, 0) == BKING) {
                if (square_distance(sq, index) == 1) {
                    return true;
                }
            }
            if (piece_at(index, 0) == BBISHOP or piece_at(index, 0) == BQUEEN) {
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
            if (piece_at(index, 0) == BKING) {
                if (square_distance(sq, index) == 1) {
                    return true;
                }
            }
            if (piece_at(index, 0) == BBISHOP or piece_at(index, 0) == BQUEEN) {
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
            if (piece_at(index, 0) == BKING) {
                if (square_distance(sq, index) == 1) {
                    return true;
                }
            }
            if (piece_at(index, 0) == BBISHOP or piece_at(index, 0) == BQUEEN) {
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
        if (Valid_Moves_Knight(sq) & knights & enemy) {
            return true;
        }
    }
    else { 
        //BLACK
        if (_rays[NORTH][sq] & enemy) {
            index = _bitscanforward(enemy & _rays[NORTH][sq]);
            if (piece_at(index, 1) == WROOK or piece_at(index, 1) == WQUEEN) {
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
            if (piece_at(index, 1) == WROOK or piece_at(index, 1) == WQUEEN) {
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
            if (piece_at(index, 1) == WROOK or piece_at(index, 1) == WQUEEN) {
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
            if (piece_at(index, 1) == WROOK or piece_at(index, 1) == WQUEEN) {
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
            if (piece_at(index, 1) == WKING) {
                if (square_distance(sq, index) == 1) {
                    return true;
                }
            }
            if (piece_at(index, 1) == WBISHOP or piece_at(index, 1) == WQUEEN) {
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
            if (piece_at(index, 1) == WKING) {
                if (square_distance(sq, index) == 1) {
                    return true;
                }
            }
            if (piece_at(index, 1) == 2 or piece_at(index, 1) == 4) {
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
            if (piece_at(index, 1) == WPAWN or piece_at(index, 1) == WKING) {
                if (square_distance(sq, index) == 1) {
                    return true;
                }
            }
            if (piece_at(index, 1) == WBISHOP or piece_at(index, 1) == WQUEEN) {
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
            if (piece_at(index, 1) == WPAWN or piece_at(index, 1) == WKING) {
                if (square_distance(sq, index) == 1) {
                    return true;
                }
            }
            if (piece_at(index, 1) == WBISHOP or piece_at(index, 1) == WQUEEN) {
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
        if (Valid_Moves_Knight(sq) & knights & enemy) {
            return true;
        }
    }
    return false;

}
BoardState board::encode_board_state(U64 wpawn, U64 wknight, U64 wbishop, U64 wrook, U64 wqueen, U64 wking, U64 bpawn, U64 bknight, U64 bbishop, U64 brook, U64 bqueen, U64 bking, int ep, int castle) {
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

inline void board::make_move(Move move) {

    int from_square = move.from_square;
    int to_square = move.to_square;
    int promotion_piece = move.promotion;
    int piece = move.piece;
    piece = piece + (side_to_move * 6);

    // piece needs to be set at its bitboard
    if (not _test_bit(bitboards[piece], to_square)) {
        BoardState boardstate = encode_board_state(bitboards[0], bitboards[1], bitboards[2], bitboards[3], bitboards[4], bitboards[5], bitboards[6], bitboards[7], bitboards[8], bitboards[9], bitboards[10], bitboards[11], en_passant_square | 0ULL, castling_rights | 0ULL);
        move_stack[move_stack_index] = boardstate;
        move_stack_index++;
        int test = side_to_move ^ 1;
        //Capture
        if (_test_bit(occupancies[test], to_square)) {
            for (int i = 0; i < 6; i++) {
                if (_test_bit(bitboards[i + (test * 6)], to_square)) {
                    //Capturing rook loses others side castle rights
                    int captured_piece = i + (test * 6);
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
                    // Remove captured piece
                    bitboards[captured_piece] &= ~(1ULL << to_square);
                    break;
                }
            }
        }
        // Remove and set piece
        bitboards[piece] &= ~(1ULL << from_square);
        bitboards[piece] |= (1ULL << to_square);

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
        if ((abs(from_square - to_square) == 7 or abs(from_square - to_square) == 9) and en_passant_square != no_sq) {
            if (piece == WPAWN and square_rank(to_square) == 5) { //
                en_passant_square = no_sq;
                bitboards[BPAWN] &= ~(1ULL << (to_square - 8));
            }
            if (piece == BPAWN and square_rank(to_square) == 2) {
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
inline void board::unmake_move(int piece, int from_square, int to_square) {
    if (move_stack_index >= 0) {
        move_stack_index--;
        BoardState board;
        board = move_stack[move_stack_index];
        bitboards[0] = board.wpawn;
        bitboards[1] = board.wknight;
        bitboards[2] = board.wbishop;
        bitboards[3] = board.wrook;
        bitboards[4] = board.wqueen ;
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
        //No need to remove the entry because it will be overwritten
    }
}
MoveList board::generate_moves() {
    int pawn_moves = 0;
    int knight_moves = 0;
    int bishop_moves = 0;
    int rook_moves = 0;
    int queen_moves = 0;
    MoveList possible_moves;
    
    U64 we;
    if (side_to_move == 0) {
        we = occupancies[0];
    }
    else {
        we = occupancies[1];
    }
    int index = 0;
    U64 pawn_mask = pawns & we;
    U64 knight_mask = knights & we;
    U64 bishop_mask = bishops & we;
    U64 rook_mask = rooks & we;
    U64 queen_mask = queens & we;
    U64 king_mask = kings & we;
    
    Move move;
    unsigned long king_sq = _bitscanforward(king_mask);
    int enemy = side_to_move ^ 1;
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
            move.en_passent = no_sq;
            move.promotion = -1;
            make_move(move);
            if (not(in_check(to_index))) {
                possible_moves.movelist[count] = move;
                count++;

            }
            unmake_move(KING, king_sq, to_index);
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
            move.en_passent = no_sq;
            make_move(move);
            if (not(in_check(king_sq))) {
                // Promotion
                if (square_rank(to_index) == 7 or square_rank(to_index) == 0) {
                    move.promotion = QUEEN;
                    //possible_moves.movelist.push_back(move);
                    possible_moves.movelist[count] = move;
                    count++;
                    move.promotion = ROOK;
                    //possible_moves.movelist.push_back(move);
                    possible_moves.movelist[count] = move;
                    count++;
                    move.promotion = KNIGHT;
                    //possible_moves.movelist.push_back(move);
                    possible_moves.movelist[count] = move;
                    count++;
                    move.promotion = BISHOP;
                    //possible_moves.movelist.push_back(move);
                    possible_moves.movelist[count] = move;
                    count++;
                }
                else {
                    move.piece = PAWN;
                    move.from_square = pawn_index;
                    move.to_square = to_index;
                    move.en_passent = no_sq;
                    move.promotion = -1;
                    //possible_moves.movelist.push_back(move);
                    possible_moves.movelist[count] = move;
                    count++;
                }
                pawn_moves++;
                index++;
            }
            unmake_move(PAWN, pawn_index, to_index);
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
            move.en_passent = no_sq;
            move.promotion = -1;
            make_move(move);
            if (not(in_check(king_sq))) {
                //possible_moves.movelist.push_back(move);
                possible_moves.movelist[count] = move;
                count++;
                knight_moves++;
                index++;
            }
            unmake_move(KNIGHT, knight_index, to_index);
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
            move.en_passent = no_sq;
            move.promotion = -1;
            make_move(move);
            if (not(in_check(king_sq))) {
                //possible_moves.movelist.push_back(move);
                possible_moves.movelist[count] = move;
                count++;
                bishop_moves++;
                index++;
            }
            unmake_move(BISHOP, bishop_index, to_index);
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
            move.en_passent = no_sq;
            move.promotion = -1;
            make_move(move);
            if (not(in_check(king_sq))) {
                //possible_moves.movelist.push_back(move);
                possible_moves.movelist[count] = move;
                count++;
                rook_moves++;
                index++;
            }
            unmake_move(ROOK, rook_index, to_index);
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
            move.en_passent = no_sq;
            move.promotion = -1;
            make_move(move);
            if (not(in_check(king_sq))) {
                //possible_moves.movelist.push_back(move);
                possible_moves.movelist[count] = move;
                count++;
                queen_moves++;
                index++;
            }
            unmake_move(QUEEN, queen_index, to_index);
            move_mask = _blsr_u64(move_mask);
        }
        queen_mask = _blsr_u64(queen_mask);
    }
    return possible_moves;
}
void board::print_bitboard(std::bitset<64> bitset) {
    std::string str_bitset = bitset.to_string();
    // reverse(str_bitset.begin(), str_bitset.end());
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

std::string square_to_coordinates_perft[64] = {
"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
};
std::string piece_type(int piece) {
    if (piece == -1) {
        return "None";
    }
    if (piece == 0) {
        return "Pawn";
    }
    if (piece == 1) {
        return "Knight";
    }
    if (piece == 2) {
        return "Bishop";
    }
    if (piece == 3) {
        return "Rook";
    }
    if (piece == 4) {
        return "Queen";
    }
    if (piece == 5) {
        return "King";
    }
    else {
        return "??";
    }
}
long long perft(board board, int depth, int max) {
    long long nodes = 0;
    MoveList n_moves = board.generate_moves();
    MA myarray;
    int side_to_move = board.side_to_move;
    int piece = -1;
    unsigned long promotion_piece = 0;
    if (depth == 1) {
        return board.count;
    }
    else {
        for (int i = 0; i < board.count; i++) {
            Move move = n_moves.movelist[i];
            board.make_move(move);
            nodes += perft(board, depth - 1, max);
            long long from_square = move.from_square;
            long long to_square = move.to_square;
            long promotion_piece = move.promotion;
            board.unmake_move(piece, from_square, to_square);
            if (depth == max) {
                myarray.myarray.push_back({ from_square, to_square, promotion_piece, nodes });
                nodes = 0;
            }
        }

    }
    long long  c = 0;
    for (int i = 0; i < myarray.myarray.size(); i++)
    {
        for (int j = 0; j < myarray.myarray[i].size(); j++)
        {
            if (j == 0 or j == 1) {
                std::cout << square_to_coordinates_perft[myarray.myarray[i][j]];
                
            }
            if (j == 2) {
                std::cout << " " << piece_type(myarray.myarray[i][j]) << " " << myarray.myarray[i][3];
            }
            
        }
        c += myarray.myarray[i][3];
        std::cout << std::endl;
    }
    if (depth != max) {
        return nodes;
    }
    else {
        return c;
    }
    
}

U64 speed_test_perft(board board, int depth, int max) {
    U64 nodes = 0;
    MoveList n_moves = board.generate_moves();
    MA myarray;
    int side_to_move = board.side_to_move;
    int piece = -1;
    unsigned long promotion_piece = 0;
    if (depth == 0) {
        return 1;
    }
    else {
        for (int i = 0; i < board.count; i++) {
            Move move = n_moves.movelist[i];
            board.make_move(move);
            nodes += speed_test_perft(board, depth - 1, max);
            long long from_square = move.from_square;
            long long to_square = move.to_square;
            long promotion_piece = move.promotion;
            board.unmake_move(piece, from_square, to_square);
        }

    }
    return nodes;
}

int perft_test(std::string fen, int depth) {
    board board;
    board.apply_fen(fen);
    auto begin = std::chrono::high_resolution_clock::now();
    U64 x = perft(board, depth, depth);
    auto end = std::chrono::high_resolution_clock::now();
    auto time_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
    std::cout <<"fen " << fen << " nodes " << x << " nps " << x / (time_diff / 1000000000.0f) << " time " << time_diff / 1000000000.0f << " seconds" << std::endl;
    return x& 18446744073709551615;
}

int test() {
    std::string fen1 = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::string fen2 = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::string fen3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
    std::string fen4 = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    std::string fen5 = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
    std::string fen6 = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ";
    auto begin = std::chrono::high_resolution_clock::now();
    if (perft_test(fen1, 5) == 4865609) { // 4 == 197281     5 == 4865609
        std::cout << "\n" << "Correct" << "\n" << std::endl;
    }
    else {
        std::cout << "False" << std::endl;
    };

    if (perft_test(fen2, 4) == 4085603) { // 4 == 4085603      3 == 97862
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
    if (perft_test(fen4, 4) == 422333) {    //
        std::cout << "\n" << "Correct" << "\n" << std::endl;
    }
    else {
        std::cout << "False" << std::endl;
    };
    if (perft_test(fen5, 4) == 2103487) {
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
