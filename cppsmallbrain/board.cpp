#include <iostream>
#include <list>
#include <bitset>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include "rays.h"
#include <iterator>
#include <vector>
#include <chrono>
#include <cmath>
#include <intrin.h>

#define get_bit(bitboard, index) (bitboard & 1ULL << index))
#define set_bit(bitboard, index) (bitboard |= (1ULL << index))
#define pop_bit(bitboard, index) (get_bit(bitboard, index) ? bitboard ^= (1ULL << index):0)





struct Move {
    int from_square;
    int to_square;
    int promotion;
    int capture;
    int en_passent;
};

struct MoveList {
    std::vector<std::vector<unsigned long>> movelist;
};
class board
{
public:
    enum {
        WPAWN,
        WKNIGHT,
        WBISHOP,
        WROOK,
        WQUEEN,
        WKING,
        BPAWN,
        BKNIGHT,
        BBISHOP,
        BROOK,
        BQUEEN,
        BKING
    };
    enum {
        PAWN,
        KNIGHT,
        BISHOP,
        ROOK,
        QUEEN,
        KING
    };
    /* The white piece positions */
    enum {
        a8, b8, c8, d8, e8, f8, g8, h8,
        a7, b7, c7, d7, e7, f7, g7, h7,
        a6, b6, c6, d6, e6, f6, g6, h6,
        a5, b5, c5, d5, e5, f5, g5, h5,
        a4, b4, c4, d4, e4, f4, g4, h4,
        a3, b3, c3, d3, e3, f3, g3, h3,
        a2, b2, c2, d2, e2, f2, g2, h2,
        a1, b1, c1, d1, e1, f1, g1, h1, no_sq
    };
    std::string square_to_coordinates[64] = {
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    };
    std::vector<std::vector<U64> >  move_stack;
    /* U64 WhitePawns, WhiteKnights, WhiteBishops, WhiteRooks, WhiteQueens, WhiteKing,
        BlackPawns, BlackKnights, BlackBishops, BlackRooks, BlackQueens, BlackKing*/
    U64 bitboards[12] = { 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL };
    // U64 0 = AllWhitePieces 1 = AllBlackPieces
    U64 occupancies[2] = { 0ULL, 0ULL };

    U64 pawns = bitboards[0] | bitboards[6];
    U64 knights = bitboards[1] | bitboards[7];
    U64 bishops = bitboards[2] | bitboards[8];
    U64 rooks = bitboards[3] | bitboards[9];
    U64 queens = bitboards[4] | bitboards[10];
    U64 kings = bitboards[5] | bitboards[11];
    U64 both = occupancies[0] | occupancies[1];

    // 0 White 1 Black
    int side_to_move = 0;
    /*
    1  white king can castle to the king side
    2  white king can castle to the queen side
    4  black king can castle to the king side
    8  black king can castle to the queen side
    */
    int castling_rights = 0;

    //en passant square
    int en_passant_square = no_sq;

    //half_moves
    int half_moves = 0;

    //full_moves
    int full_moves = 0;

    const U64 not_a_file = 0xfefefefefefefefe;
    // not H file constant
    const U64 not_h_file = 0x7f7f7f7f7f7f7f7f;
    // not HG file constant
    const U64 not_hg_file = 4557430888798830399ULL;
    // not AB file constant
    const U64 not_ab_file = 18229723555195321596ULL;

    const U64 rank_2_mask = 18446744069431296255ULL;
    const U64 rank_7_mask = 18374966859431673855ULL;
    const U64 rank_8_mask = 18374686479671623680ULL;
    const U64 rank_1_mask = 255ULL;

    enum { wk = 1, wq = 2, bk = 4, bq = 8 };
    

    std::vector<std::string> split_fen(std::string fen)
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

    void apply_fen(std::string fen)
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
        


        //moveright
        if (move_right == "w")
        {
            side_to_move = 0;
        }
        else
        {
            side_to_move = 1;
        }

        //castling
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
                // std::cout << letter << " " << pos << " " << "rank: " << rank << " file: " << file << " sq: " << sq << std::endl;
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

        //en passant
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
        update_occupancies();
    }
    void get_side_to_move() {
        std::cout << "get_move_right: " << side_to_move << std::endl;
    }
    void get_castling_rights() {
        std::cout << "castling: " << castling_rights << std::endl;
    }
    void get_en_passant_square() {
        std::cout << "en_passant: " << en_passant_square << std::endl;
    }
    void get_half_moves() {
        std::cout << "half_moves: " << half_moves << std::endl;
    }
    void get_full_moves() {
        std::cout << "full_moves: " << full_moves << std::endl;
    }
    void update_occupancies() {
        // White
        occupancies[0] = bitboards[0] | bitboards[1] | bitboards[2] | bitboards[3] | bitboards[4] | bitboards[5];
        // Black
        occupancies[1] = bitboards[6] | bitboards[7] | bitboards[8] | bitboards[9] | bitboards[10] | bitboards[11];

        both = occupancies[0] | occupancies[1];
        pawns = bitboards[0] | bitboards[6];
        knights = bitboards[1] | bitboards[7];
        bishops = bitboards[2] | bitboards[8];
        rooks = bitboards[3] | bitboards[9];
        queens = bitboards[4] | bitboards[10];
        kings = bitboards[5] | bitboards[11];
    }
    void get_board() {
        print_bitboard(occupancies[0] | occupancies[1]);
    }
    bool piece_color(int sq) {
        if (std::bitset<64>(occupancies[0]).test(sq) == 1) {
            return true;
        }
        else {
            return false;
        }
    }

    int piece_at(int sq) {
        bool white = false;
        if (std::bitset<64>(occupancies[0]).test(sq) == 1) {
            white = true;
        }
        if (white) {
            if (std::bitset<64>(bitboards[WPAWN]).test(sq) == 1) {
                return WPAWN;
            }
            if (std::bitset<64>(bitboards[WKNIGHT]).test(sq) == 1) {
                return WKNIGHT;
            }
            if (std::bitset<64>(bitboards[WBISHOP]).test(sq) == 1) {
                return WBISHOP;
            }
            if (std::bitset<64>(bitboards[WROOK]).test(sq) == 1) {
                return WROOK;
            }
            if (std::bitset<64>(bitboards[WQUEEN]).test(sq) == 1) {
                return WQUEEN;
            }
            if (std::bitset<64>(bitboards[WKING]).test(sq) == 1) {
                return WKING;
            }
        }
        else {
            if (std::bitset<64>(bitboards[BPAWN]).test(sq) == 1) {
                return BPAWN;
            }
            if (std::bitset<64>(bitboards[BKNIGHT]).test(sq) == 1) {
                return BKNIGHT;
            }
            if (std::bitset<64>(bitboards[BBISHOP]).test(sq) == 1) {
                return BBISHOP;
            }
            if (std::bitset<64>(bitboards[BROOK]).test(sq) == 1) {
                return BROOK;
            }
            if (std::bitset<64>(bitboards[BQUEEN]).test(sq) == 1) {
                return BQUEEN;
            }
            if (std::bitset<64>(bitboards[BKING]).test(sq) == 1) {
                return BKING;
            }
        }
        return -1;

    }
    int square_file(int sq) {
        //Gets the file index of the square where 0 is the a-file
        return sq & 7;
    }
    int square_rank(int sq) {
        //Gets the rank index of the square where 0 is the first rank."""
        return sq >> 3;
    }
    int square_distance(int a, int b) {
        return std::max(abs(square_file(a) - square_file(b)), abs(square_rank(a) - square_rank(b)));
    }

    int distance_to_edge_right(int sq) {
        return (7 - square_file(sq));
    }
    int distance_to_edge_left(int sq) {
        return square_file(sq);
    }


    U64 sliding_attacks(int square, int deltas[4]) {
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

    U64 pawn_attacks(int sq) {
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
    U64 Valid_Moves_Pawn(int sq) {
        //White

        U64 pawn_move = 0ULL;
        U64 pawn_move_left = 0ULL;
        U64 pawn_move_right = 0ULL;
        U64 pawn_move_attack_union = 0ULL;
        U64 pawn_move_union_valid_left_right = 0ULL;
        U64 pawn_move_forward = 0ULL;
        U64 pawn_move_forward_double = 0ULL;
        U64 valid_move = 0ULL;
        pawn_move |= (1ULL << sq);
        if (piece_color(sq)) {
            //White Pawn
            pawn_move_left = (pawn_move << 7) & not_h_file;
            pawn_move_right = (pawn_move << 9) & not_a_file;
            pawn_move_attack_union = pawn_move_left | pawn_move_right;
            pawn_move_union_valid_left_right = (pawn_move_attack_union & occupancies[1]);
            if (en_passant_square != no_sq) {
                pawn_move_union_valid_left_right = (pawn_move_union_valid_left_right | ((1ULL << en_passant_square) & (pawn_move_attack_union))) & ~occupancies[0];
                
            }
            // North
            pawn_move_forward = pawn_move << 8 & ~both;
            pawn_move_forward_double = (pawn_move_forward << 8) & ~rank_2_mask & ~both;
            valid_move = (pawn_move_forward) | (pawn_move_forward_double) | pawn_move_union_valid_left_right;
        }
        else {
            //Black Pawn
            pawn_move_left = (pawn_move >> 7) & not_a_file;
            pawn_move_right = (pawn_move >> 9) & not_h_file;
            pawn_move_attack_union = pawn_move_left | pawn_move_right;
            pawn_move_union_valid_left_right = pawn_move_attack_union & occupancies[0];
            if (en_passant_square != no_sq) {
                pawn_move_union_valid_left_right = (pawn_move_union_valid_left_right | ((1ULL << en_passant_square) & (pawn_move_attack_union))) & ~occupancies[1];
            }
            //South
            pawn_move_forward = pawn_move >> 8 & ~both;
            pawn_move_forward_double = (pawn_move_forward >> 8) & ~rank_7_mask & ~both;
            valid_move = (pawn_move_forward) | (pawn_move_forward_double) | pawn_move_union_valid_left_right;
        }
        return valid_move;
    }
    U64 Valid_Moves_Knight(int sq) {
        U64 knight_move = 0ULL;
        int deltas[4] = { 17,15,10,6 };
        knight_move = sliding_attacks(sq, deltas);
        if (std::bitset<64>(occupancies[0]).test(sq) == 1) {
            knight_move = knight_move & (occupancies[1] | ~occupancies[0]);
        }
        else {
            knight_move = knight_move & (occupancies[0] | ~occupancies[1]);
        }
        return knight_move;
    }
    U64 Valid_Moves_Bishop(int sq) {
        U64 bishop_move = 0ULL;
        bishop_move |= (1ULL << sq);
        U64 victims;
        U64 blockers;

        init_rays();

        if (std::bitset<64>(occupancies[0]).test(sq) == 1) {
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
    U64 Valid_Moves_Rook(int sq) {
        U64 rook_move = 0ULL;
        rook_move |= (1ULL << sq);
        U64 victims;
        U64 blockers;

        init_rays();

        if (std::bitset<64>(occupancies[0]).test(sq) == 1) {
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
    U64 Valid_Moves_Queen(int sq) {
        U64 queen_move = 0ULL;
        queen_move |= (1ULL << sq);
        queen_move |= Valid_Moves_Bishop(sq);
        queen_move |= Valid_Moves_Rook(sq);
        queen_move &= ~(1ULL << sq);
        return queen_move;
    }
    U64 Valid_Moves_King(int sq) {
        U64 king_move = 0ULL;
        init_rays();
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
    bool in_check(int sq, int cast_check=-1) {

        init_rays();
        bool white = false;
        U64 enemy;
        U64 us;
        //enemy = both;
        if (cast_check > -1) {
            if (cast_check == 1) {
                
                white = true;
                us = occupancies[0];
                enemy = occupancies[1];
                //enemy = both;
            }
            if (cast_check == 0) {
                //enemy = both;
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
                if (piece_at(index) == BROOK or piece_at(index) == BQUEEN) {
                    // if we dont have a piece here its definitely not blocked
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
                if (piece_at(index) == BROOK or piece_at(index) == BQUEEN) {
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
                
                if (piece_at(index) == BROOK or piece_at(index) == BQUEEN) {
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
                if (piece_at(index) == BROOK or piece_at(index) == BQUEEN) {
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
                if (piece_at(index) == BPAWN or piece_at(index) == BKING) {
                    if (square_distance(sq, index) == 1) {
                        return true;
                    }
                }
                if (piece_at(index) == BBISHOP or piece_at(index) == BQUEEN) {
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
                if (piece_at(index) == BPAWN or piece_at(index) == BKING) {
                    if (square_distance(sq, index) == 1) {

                        return true;
                    }
                }
                if (piece_at(index) == BBISHOP or piece_at(index) == BQUEEN) {
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
                if (piece_at(index) == BKING) {
                    if (square_distance(sq, index) == 1) {
                        return true;
                    }
                }
                if (piece_at(index) == BBISHOP or piece_at(index) == BQUEEN) {
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
                if (piece_at(index) == BKING) {
                    if (square_distance(sq, index) == 1) {

                        return true;
                    }
                }
                if (piece_at(index) == BBISHOP or piece_at(index) == BQUEEN) {
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
        else { //BLACK
            
            
            if (_rays[NORTH][sq] & enemy) {
                index = _bitscanforward(enemy & _rays[NORTH][sq]);
                if (piece_at(index) == WROOK or piece_at(index) == WQUEEN) {
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
                if (piece_at(index) == WROOK or piece_at(index) == WQUEEN) {
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
                if (piece_at(index) == WROOK or piece_at(index) == WQUEEN) {
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
                if (piece_at(index) == WROOK or piece_at(index) == WQUEEN) {
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
                if (piece_at(index) == WKING) {
                    if (square_distance(sq, index) == 1) {
                        return true;
                    }
                }
                if (piece_at(index) == WBISHOP or piece_at(index) == WQUEEN) {
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
                if (piece_at(index) == WKING) {
                    if (square_distance(sq, index) == 1) {
                        return true;
                    }
                }
                if (piece_at(index) == 2 or piece_at(index) == 4) {
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
                if (piece_at(index) == WPAWN or piece_at(index) == WKING) {
                    if (square_distance(sq, index) == 1) {
                        return true;
                    }
                }
                if (piece_at(index) == WBISHOP or piece_at(index) == WQUEEN) {
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
                if (piece_at(index) == WPAWN or piece_at(index) == WKING) {
                    
                    if (square_distance(sq, index) == 1) {
                        return true;
                    }
                }
                if (piece_at(index) == WBISHOP or piece_at(index) == WQUEEN) {
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
    void make_move(int piece, int from_square, int to_square, int promotion_piece = 0) {
        piece = piece + (side_to_move * 6);

        if (std::bitset<64>(bitboards[piece]).test(from_square) == 1) {
            if (std::bitset<64>(bitboards[piece]).test(to_square) != 1) {
                move_stack.push_back({ bitboards[0],bitboards[1],bitboards[2],bitboards[3],bitboards[4],bitboards[5],bitboards[6],bitboards[7],bitboards[8],bitboards[9] ,bitboards[10],bitboards[11], en_passant_square | 0ULL, castling_rights | 0ULL });
                int test = side_to_move ^ 1;
                //Capture
                if (std::bitset<64>(occupancies[test]).test(to_square) == 1) {
                    for (int i = 0; i < 6; i++) {
                        if (std::bitset<64>(bitboards[i + (test * 6)]).test(to_square) == 1) {
                            //Capturing rook loses others side castle rights
                            if (piece_at(to_square) == WROOK) {
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
                            if (piece_at(to_square) == BROOK) {
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
                            
                            bitboards[i + (test * 6)] &= ~(1ULL << to_square);
                            break;
                        }
                    }
                }
                bitboards[piece] &= ~(1ULL << from_square);
                bitboards[piece] |= (1ULL << to_square);
                
                // King move loses castle rights
                if (piece == WKING ) {
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

                //Castling
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
                if (promotion_piece > 0) {
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
        }
    }
    void unmake_move(int piece, int from_square, int to_square) {
        if (move_stack.size() > 0) {
            for (int i = 0; i < 12; i++) {
                bitboards[i] = move_stack.back()[i];
            }
            en_passant_square = move_stack.back()[12];
            castling_rights = move_stack.back()[13];
            side_to_move ^= 1;
            update_occupancies();
            move_stack.pop_back();
        }
    }
    int index_lsb_bit(U64 mask) {
        int index = _bitscanforward(mask);
        return index;
    }
    MoveList generate_moves() {
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

        unsigned long king_sq = index_lsb_bit(king_mask);

        while (king_mask) {
            king_sq = index_lsb_bit(king_mask);
            U64 move_mask = Valid_Moves_King(king_sq);
            while(move_mask) {
                unsigned long to_index = index_lsb_bit(move_mask);
                make_move(KING, king_sq, to_index);
                if (not(in_check(to_index))) {
                    possible_moves.movelist.push_back(std::vector<unsigned long> {king_sq, to_index});
                }
                unmake_move(KING, king_sq, to_index);
                move_mask &= ~(1ULL << to_index);
            }
            king_mask &= ~(1ULL << king_sq);
        }

        king_sq = index_lsb_bit(kings & we);
        for (int i = 0; i < 5; i++) {
            if (i == 0) {
                unsigned long pawn_index;
                while (pawn_mask) {
                    pawn_index = index_lsb_bit(pawn_mask);
                    U64 move_mask = Valid_Moves_Pawn(pawn_index);
                    while (move_mask) {
                        unsigned long to_index = index_lsb_bit(move_mask);
                        make_move(PAWN, pawn_index, to_index);
                        if (not(in_check(king_sq))) {
                            // Promotion
                            if (square_rank(to_index) == 7 or square_rank(to_index) == 0) {
                                possible_moves.movelist.push_back(std::vector<unsigned long> {pawn_index, to_index, QUEEN});
                                possible_moves.movelist.push_back(std::vector<unsigned long> {pawn_index, to_index, ROOK});
                                possible_moves.movelist.push_back(std::vector<unsigned long> {pawn_index, to_index, KNIGHT});
                                possible_moves.movelist.push_back(std::vector<unsigned long> {pawn_index, to_index, BISHOP});
                            }
                            else {
                                possible_moves.movelist.push_back(std::vector<unsigned long> {pawn_index, to_index});
                            }
                            pawn_moves++;
                            index++;
                        }
                        unmake_move(PAWN, pawn_index, to_index);
                        move_mask &= ~(1ULL << to_index);
                    }
                    pawn_mask &= ~(1ULL << pawn_index);
                }
            }
            if (i == 1) {
                unsigned long knight_index;
                while (knight_mask) {
                    knight_index = index_lsb_bit(knight_mask);
                    U64 move_mask = Valid_Moves_Knight(knight_index);
                    while (move_mask) {
                        unsigned long to_index = index_lsb_bit(move_mask);
                        make_move(KNIGHT, knight_index, to_index);
                        if (not(in_check(king_sq))) {
                            possible_moves.movelist.push_back(std::vector<unsigned long> {knight_index, to_index});
                            knight_moves++;
                            index++;
                        }
                        unmake_move(KNIGHT, knight_index, to_index);
                        move_mask &= ~(1ULL << to_index);
                    }
                    knight_mask &= ~(1ULL << knight_index);
                }
            }
            if (i == 2) {
                unsigned long bishop_index;
                while (bishop_mask) {
                    bishop_index = index_lsb_bit(bishop_mask);
                    U64 move_mask = Valid_Moves_Bishop(bishop_index);
                    while(move_mask) {
                        unsigned long to_index = index_lsb_bit(move_mask);
                        make_move(BISHOP, bishop_index, to_index);
                        if (not(in_check(king_sq))) {
                            possible_moves.movelist.push_back(std::vector<unsigned long> {bishop_index, to_index});
                            bishop_moves++;
                            index++;
                        } 
                        unmake_move(BISHOP, bishop_index, to_index);
                        move_mask &= ~(1ULL << to_index);
                    }
                    bishop_mask &= ~(1ULL << bishop_index);
                }
            }
            if (i == 3) {
                unsigned long rook_index;
                while (rook_mask) {
                    
                    rook_index = index_lsb_bit(rook_mask);
                    U64 move_mask = Valid_Moves_Rook(rook_index);
                    
                    while(move_mask) {
                        unsigned long to_index = index_lsb_bit(move_mask);
                        make_move(ROOK, rook_index, to_index);
                        if (not(in_check(king_sq))) {
                            possible_moves.movelist.push_back(std::vector<unsigned long> {rook_index, to_index});
                            rook_moves++;
                            index++;
                        }
                        unmake_move(ROOK, rook_index, to_index);
                        move_mask &= ~(1ULL << to_index);
                    }
                    rook_mask &= ~(1ULL << rook_index);
                }
            }
            if (i == 4) {
                unsigned long queen_index;
                while (queen_mask) {
                    queen_index = index_lsb_bit(queen_mask);
                    U64 move_mask = Valid_Moves_Queen(queen_index);
                    while(move_mask) {
                        unsigned long to_index = index_lsb_bit(move_mask);
                        make_move(QUEEN, queen_index, to_index);
                        if (not(in_check(king_sq))) {
                            possible_moves.movelist.push_back(std::vector<unsigned long> {queen_index, to_index});
                            queen_moves++;
                            index++;
                        }
                        unmake_move(QUEEN, queen_index, to_index);
                        move_mask &= ~(1ULL << to_index);
                    }
                    queen_mask &= ~(1ULL << queen_index);
                }
            }
        }

        return possible_moves;

    }
    void print_bitboard(std::bitset<64> bitset) {
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
};

std::string square_to_coordinates[64] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
};
void print_movelist(MoveList ml) {
    int index = 0;
    for (int i = 0; i < ml.movelist.size(); i++) {
        for (int j = 0; j < ml.movelist[i].size(); j++) {
            
            std::cout << square_to_coordinates[ml.movelist[i][j]] << ' '; //ml.movelist[i][j]
        }
        index++;
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << index << std::endl;
}

struct MA {
    std::vector<std::vector<unsigned long>> myarray;
};

unsigned long perft(board& board, int depth, int max) {
    unsigned long nodes = 0;
    MoveList n_moves = board.generate_moves();
    MA myarray;
    int side_to_move = board.side_to_move;
    int piece = -1;
    unsigned long promotion_piece = 0;
    if (depth == 1) {
        return n_moves.movelist.size();
    }
    else {
        for (int i = 0; i < n_moves.movelist.size(); i++) {
            for (int j = 0; j <= 6; j++) {
                if (std::bitset<64>(board.bitboards[j + (6 * side_to_move)]).test(n_moves.movelist[i][0]) == 1) {
                    piece = j;
                    break;
                }
            }
            if (n_moves.movelist[i].size() > 2) {
                promotion_piece = n_moves.movelist[i][2];
            }
            else {
                promotion_piece = 0;
            }
            
            board.make_move(piece, n_moves.movelist[i][0], n_moves.movelist[i][1], promotion_piece);
            nodes += perft(board, depth - 1, max);
            board.unmake_move(piece, n_moves.movelist[i][0], n_moves.movelist[i][1]);
            if (depth == max) {
                myarray.myarray.push_back({ n_moves.movelist[i][0], n_moves.movelist[i][1], promotion_piece, nodes });
                nodes = 0;
            }
        }

    }
    int  c = 0;
    for (int i = 0; i < myarray.myarray.size(); i++)
    {
        for (int j = 0; j < myarray.myarray[i].size(); j++)
        {
            if (j == 0 or j == 1) {
                std::cout << square_to_coordinates[myarray.myarray[i][j]] << " ";
                
            }
            else {
 
                std::cout << myarray.myarray[i][j] << " ";

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
void perft_console() {
    
    std::string fen;
    board board;
    bool b = true;
    int depth;
    while (b) {
        std::cout << "Fen: " << std::endl;
        std::getline(std::cin, fen);
        std::cout << "Depth: " << std::endl;
        std::cin >> depth;
        if (depth == 0) {
            b = false;
        }
        board.apply_fen(fen);
        board.split_fen(fen);
        auto begin = std::chrono::high_resolution_clock::now();
        U64 x = perft(board, depth, depth);
        auto end = std::chrono::high_resolution_clock::now();
        auto time_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
        std::cout << "nodes " << x << " nps " << x / (time_diff / 1000000000.0f) << " time " << time_diff / 1000000000.0f << " seconds" << std::endl;
        
    }

}

int main()
{
    //std::string fen = "6k1/4Pp2/8/6PP/1pbP3P/2P3pP/P1K2PPP/8 w - - 0 1";
    //std::string fen = "6k1/4Pp2/8/3N2PP/1pbPPp1P/2P3pP/P1K1PPPP/8 w - - 0 1";
    //std::string fen = "6k1/4Pp2/8/3N2PP/1pbPPp1P/2P3pP/P1K1PPPP/8 w - - 0 1";
    //std::string fen = "8/8/2pp4/8/3RB3/3PQ3/8/8 w - - 0 1";
    //std::string fen = "k4r2/8/8/8/8/8/8/4K2R w K - 0 1";
    //std::string fen = "8/8/2pp4/8/3RB3/3P2k1/8/4B3 b - - 0 1";
    //std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::string fen = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";
    //int index;
    //index = _bitscanforward(0);
    //std::cout << "index: " << index;
    // 
    board board;
    board.apply_fen(fen);
    board.split_fen(fen);
    //perft_console();

    //print_movelist(board.generate_moves());
    
    //std::cout << board.generate_moves().movelist.size() << std::endl;

    //test t;
    //test_fun(t, 3);
    // 
    
    auto begin = std::chrono::high_resolution_clock::now();
    int search = 4;
    U64 x = perft(board, search, search);
    auto end = std::chrono::high_resolution_clock::now();
    auto time_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
    std::cout << "nodes " << x << " nps " << x / (time_diff / 1000000000.0f) << " time " << time_diff / 1000000000.0f << " seconds" << std::endl;

    //for (int i = 0; i < 12; i++) {
    //    board.print_bitboard(board.bitboards[i]);
    //}
    //board.print_bitboard(board.occupancies[0]);
    //board.print_bitboard(board.occupancies[1]);
    //board.print_bitboard(board.both);
    //std::cout << board.both;
    // 
    //

    
    
    //board.get_board();
    //board.print_bitboard(board.Valid_Moves_Pawn(48));
    //board.print_bitboard(board.Valid_Moves_Knight(1));
    //board.print_bitboard(board.Valid_Moves_Bishop(2));
    //board.print_bitboard(board.Valid_Moves_Rook(0));
    //board.print_bitboard(board.Valid_Moves_Queen(3));
    //board.print_bitboard(board.Valid_Moves_King(4));
    //board.print_bitboard(board.Valid_Moves_King(31));
    //std::cout << board.in_check(35, 1);

    //board.print_bitboard();
    // board.get_board();
    // board.make_move(10, 9);
    // board.get_board();
    //

    // board.Valid_Moves_King(10);
    // board.get_full_moves();
     //std::cout << "White: " << board.occupancies[0] << " Black: " << std::bitset<64>(board.occupancies[1]) << std::endl;
    return 0;
}