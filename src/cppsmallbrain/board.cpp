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
#include <chrono>
#include <map>

#include "zobrist.h"
#include "general.h"
#include "board.h"

std::vector<std::string> split_input(std::string fen)
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

void Board::apply_fen(std::string fen)
{
    for (int i = 0; i < 12; i++)
    {
        bitboards[i] = 0ULL;
    }
    std::string position = split_input(fen)[0];
    std::string move_right = split_input(fen)[1];
    std::string castling = split_input(fen)[2];
    std::string en_passant = split_input(fen)[3];
    std::string half_move_clock = split_input(fen)[4];
    std::string full_move_counter = split_input(fen)[5];

    // Side to move
    // I really want to change 0 to 1 but any changes to this have failed, i'll try to change this in the future.
    if (move_right == "w")
    {
        side_to_move = 0;
    }
    else
    {
        side_to_move = 1;
    }
    // Placing pieces on the board
    int pos = 0;
    int sq;
    char letter;
    std::map<char, int> piece_to_int =
    {
    { 'P', 0 },
    { 'N', 1 },
    { 'B', 2 },
    { 'R', 3 },
    { 'Q', 4 },
    { 'K', 5 },
    { 'p', 6 },
    { 'n', 7 },
    { 'b', 8 },
    { 'r', 9 },
    { 'q', 10 },
    { 'k', 11 },
    };
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            sq = rank * 8 + file;
            letter = position[pos];
            if (pos >= position.size()){
                break;
            }
            if (piece_to_int.count(letter)) {
                int piece = piece_to_int[letter];
                bitboards[piece] |= (1ULL << sq);
                pos++;
            }
            if (letter - '0' >= 2 and letter - '0' <= 7) {
                file += letter - '0' - 1;
                pos++;
            }
            if (letter == '1') {
                pos++;
            }
            if (letter == '8') {
                rank--;
                file--;
                pos++;
            }
            if (letter == '/') {
                file--;
                pos++;
            }
        }
    }
    // Encodes castling rights
    castling_rights = 0;
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
    for (int sq = 0; sq < 64; sq++) {
        if (piece_type_at(sq) != -1) {
            board_pieces[sq] = piece_at(sq);
        }
        else {
            board_pieces[sq] = -1;
        }
    }
}

//returns en passant square
int Board::get_en_passant_square() {
    return en_passant_square;
}

U64 Board::generate_zhash() {
    U64 hash = 0ULL;
    U64 white = White;
    U64 black = Black;
    bool IsWhite = side_to_move ? 0 : 1;

    while (white) {
        int sq = _bitscanforward(white);
        int piece = piece_type_at(sq);
        if (piece != -1) {
            piece = piece * 2 + 1;
            hash ^= RANDOM_ARRAY[64 * piece + sq];
        }
        white = _blsr_u64(white);
    }
    while (black) {
        int sq = _bitscanforward(black);
        int piece = piece_type_at(sq);
        if (piece != -1) {
            piece = piece * 2;
            hash ^= RANDOM_ARRAY[64 * piece + sq];
        }
        black = _blsr_u64(black);
    }

    U64 ep_hash = 0ULL;
    if (get_en_passant_square() != 64) {
        U64 ep_mask = 0ULL;
        if (IsWhite) {
            U64 ep_square = 1ULL << get_en_passant_square();
            ep_mask = Pawn_AttackLeft(!IsWhite, ep_square) | Pawn_AttackRight(!IsWhite, ep_square);
        }
        else {
            U64 ep_square = 1ULL << get_en_passant_square();
            ep_mask = Pawn_AttackLeft(IsWhite, ep_square) | Pawn_AttackRight(IsWhite, ep_square);
        }
        U64 color_p = IsWhite ? White : Black;
        if (ep_mask & (bitboards[WPAWN] | bitboards[BPAWN]) & color_p) {
            ep_hash = RANDOM_ARRAY[772 + square_file(get_en_passant_square())];
        }
    }
    U64 turn_hash = IsWhite ? RANDOM_ARRAY[780] : 0;
    U64 cast_hash = 0ULL;
    if (castling_rights & 1) {
        cast_hash ^= RANDOM_ARRAY[768];
    }
    if (castling_rights & 2) {
        cast_hash ^= RANDOM_ARRAY[768 + 1];
    }
    if (castling_rights & 4) {
        cast_hash ^= RANDOM_ARRAY[768 + 2];
    }
    if (castling_rights & 8) {
        cast_hash ^= RANDOM_ARRAY[768 + 3];
    }

    return hash ^ cast_hash ^ turn_hash ^ ep_hash;
}

inline BoardState Board::encode_board_state(U64 wpawn, U64 wknight, U64 wbishop, U64 wrook, U64 wqueen, U64 wking,
    U64 bpawn, U64 bknight, U64 bbishop, U64 brook, U64 bqueen, U64 bking,
    int ep, int castle, int all_pieces[64])
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
    std::copy(all_pieces, all_pieces+64, board.piece_loc);
    return board;
}

inline void Board::update_occupancies() {
    White = bitboards[WPAWN] | bitboards[WKNIGHT] | bitboards[WBISHOP] | bitboards[WROOK] | bitboards[WQUEEN] | bitboards[WKING];
    Black = bitboards[BPAWN] | bitboards[BKNIGHT] | bitboards[BBISHOP] | bitboards[BROOK] | bitboards[BQUEEN] | bitboards[BKING];
    Occ = White | Black;
    White = White;
    Black = Black;
}

void Board::add_repetition(U64 hash) {
    if (repetition_table.count(hash)) {
        repetition_table[hash]++;
    }
    else {
        repetition_table[hash] = 1;
    }
};

void Board::remove_repetition(U64 hash) {
    if (repetition_table[hash] == 0) {
        std::cout << "ERROR REMOVE REPETITION \n";
    }
    repetition_table[hash]--;
};

bool Board::is_threefold_rep() {
    U64 hash = generate_zhash();
    if (repetition_table[hash] >= 2) {
        return true;
    }
    return false;
}

bool Board::is_threefold_rep3() {
    U64 hash = generate_zhash();
    if (repetition_table[hash] >= 3) {
        return true;
    }
    return false;
}

void Board::make_move(Move& move) {

    int from_square = move.from_square;
    int to_square = move.to_square;
    int promotion_piece = move.promotion;
    int piece = move.piece;
    if (move.null == 1) {
        side_to_move ^= 1;
        return;
    }

    if (move.piece == -1) {
        piece = piece_at_square(from_square);
    }
    else {
        piece = piece + (side_to_move * 6);
    }

    int captured_piece = -1;
    bool IsWhite = side_to_move ? 0 : 1;
    bool enemy = side_to_move ^ 1;

    // piece needs to be set at its bitboard remove this for performance if you are 100% theres a piece at that square
    if (not _test_bit(bitboards[piece], to_square)) {
        BoardState boardstate = encode_board_state(bitboards[WPAWN], bitboards[WKNIGHT], bitboards[WBISHOP], bitboards[WROOK], bitboards[WQUEEN], bitboards[WKING], bitboards[BPAWN], bitboards[BKNIGHT], bitboards[BBISHOP], bitboards[BROOK], bitboards[BQUEEN], bitboards[BKING], en_passant_square, castling_rights, board_pieces);
        move_stack.push(boardstate);
        //Capture
        captured_piece = piece_at_square(to_square);
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
        // King move loses castle rights
        if (piece == WKING) {
            if (to_square != 6 and to_square != 2) {
                castling_rights &= ~(1);
                castling_rights &= ~(2);
            }
        }
        if (piece == BKING) {
            if (to_square != 62 and to_square != 58) {
                castling_rights &= ~(4);
                castling_rights &= ~(8);
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

        // Actual castling
        if (piece == WKING and square_distance(from_square, to_square) == 2) {
            if (to_square == 6 and _test_bit(bitboards[WROOK], 7)) {
                if (castling_rights & wk) {
                    castling_rights &= ~(1);
                    castling_rights &= ~(2);
                    bitboards[WROOK] &= ~(1ULL << 7);
                    bitboards[WROOK] |= (1ULL << 5);
                    board_pieces[7] = -1;
                    board_pieces[5] = WROOK;
                }
            }
            if (to_square == 2 and _test_bit(bitboards[WROOK], 0)) {
                if (castling_rights & wq) {
                    castling_rights &= ~(1);
                    castling_rights &= ~(2);
                    bitboards[WROOK] &= ~(1ULL << 0);
                    bitboards[WROOK] |= (1ULL << 3);
                    board_pieces[0] = -1;
                    board_pieces[3] = WROOK;
                }
            }
        }
        if (piece == BKING and square_distance(from_square, to_square) == 2) {
            if (to_square == 62 and _test_bit(bitboards[BROOK], 63)) {
                if (castling_rights & bk) {
                    castling_rights &= ~(4);
                    castling_rights &= ~(8);
                    bitboards[BROOK] &= ~(1ULL << 63);
                    bitboards[BROOK] |= (1ULL << 61);
                    board_pieces[63] = -1;
                    board_pieces[61] = BROOK;
                }
            }
            if (to_square == 58 and _test_bit(bitboards[BROOK], 56)) {
                if (castling_rights & bq) {
                    castling_rights &= ~(4);
                    castling_rights &= ~(8);
                    bitboards[BROOK] &= ~(1ULL << 56);
                    bitboards[BROOK] |= (1ULL << 59);
                    board_pieces[56] = -1;
                    board_pieces[59] = BROOK;
                }
            }
        }

        // Remove enemy piece if en passant capture
        if ((abs(from_square - to_square) == 7 or abs(from_square - to_square) == 9)) {
            if (piece == WPAWN and square_rank(to_square) == 5 and piece_at_square(to_square - 8) == BPAWN and piece_at_square(to_square) == -1) { //
                en_passant_square = no_sq;
                bitboards[BPAWN] &= ~(1ULL << (to_square - 8));
                board_pieces[to_square - 8] = -1;
            }
            if (piece == BPAWN and square_rank(to_square) == 2 and piece_at_square(to_square + 8) == WPAWN and piece_at_square(to_square) == -1) {
                en_passant_square = no_sq;
                bitboards[WPAWN] &= ~(1ULL << (to_square + 8));
                board_pieces[to_square + 8] = -1;
            }
        }
        // remove en passant if it wasnt played immediately
        if (en_passant_square != no_sq) {
            en_passant_square = no_sq;
        }

        // set en passant square if pawns double move
        if (piece == WPAWN and (from_square - to_square) == -16) {
            en_passant_square = from_square + 8;
        }
        if (piece == BPAWN and (from_square - to_square) == 16) {
            en_passant_square = from_square - 8;
        }

        // Remove and set piece
        bitboards[piece] &= ~(1ULL << from_square);
        bitboards[piece] |= (1ULL << to_square);
        board_pieces[from_square] = -1;
        board_pieces[to_square] = piece;

        // Remove captured piece
        if (captured_piece != -1) {
            bitboards[captured_piece] &= ~(1ULL << to_square);
        }

        // Promotion
        if (promotion_piece > 0 and promotion_piece < 7) {
            bitboards[piece] &= ~(1ULL << to_square);
            promotion_piece = promotion_piece + (side_to_move * 6);
            board_pieces[to_square] = promotion_piece;
            bitboards[promotion_piece] |= (1ULL << to_square);
        }

        // Swap color
        side_to_move ^= 1;

        //Update
        update_occupancies();
        //add_repetition(generate_zhash());
    }
    else {
        // Not valid move
        print_board();
        std::cout << "piece " << piece_type(piece) << " from " << from_square << " to " << to_square << "\n";
        std::cout << "Not valid move" << castling_rights << std::endl;
    }
};

void Board::unmake_move() {
    if (move_stack.size() > 0) {
        //remove_repetition(generate_zhash());
        BoardState board;
        board = move_stack.top();
        bitboards[WPAWN] = board.wpawn;
        bitboards[WKNIGHT] = board.wknight;
        bitboards[WBISHOP] = board.wbishop;
        bitboards[WROOK] = board.wrook;
        bitboards[WQUEEN] = board.wqueen;
        bitboards[WKING] = board.wking;
        bitboards[BPAWN] = board.bpawn;
        bitboards[BKNIGHT] = board.bknight;
        bitboards[BBISHOP] = board.bbishop;
        bitboards[BROOK] = board.brook;
        bitboards[BQUEEN] = board.bqueen;
        bitboards[BKING] = board.bking;
        en_passant_square = board.en_passant;
        castling_rights = board.castle_rights;
        std::copy(board.piece_loc, board.piece_loc+64, board_pieces);
        side_to_move ^= 1;
        update_occupancies();
        move_stack.pop();
    }
    else {
        //Tried to unmake a move although theres no previous move
        std::cout << "no entry" << std::endl;
    }
}

// prints a U64 in bitboard representation
void Board::print_bitboard(std::bitset<64> bits) {
    std::string str_bitset = bits.to_string();
    for (int i = 0; i < 64; i += 8)
    {
        std::string x = str_bitset.substr(i, 8);
        reverse(x.begin(), x.end());
        std::cout << x << std::endl;
    }
    std::cout << '\n' << std::endl;
}

void Board::print_board() {
    for (int i = 63; i >= 0; i -= 8)
    {
        std::cout << " " << piece_type(piece_at(i - 7)) << " " << piece_type(piece_at(i - 6)) << " " << piece_type(piece_at(i - 5)) << " " << piece_type(piece_at(i - 4)) << " " << piece_type(piece_at(i - 3)) << " " << piece_type(piece_at(i - 2)) << " " << piece_type(piece_at(i - 1)) << " " << piece_type(piece_at(i)) << " " << std::endl;
    }
    std::cout << '\n' << std::endl;
}

void Board::print_board2() {
    for (int i = 63; i >= 0; i -= 8)
    {
        std::cout << " " << piece_type(board_pieces[i-7]) << " " << piece_type(board_pieces[i - 6]) << " " << piece_type(board_pieces[i - 5]) << " " << piece_type(board_pieces[i - 4]) << " " << piece_type(board_pieces[i - 3]) << " " << piece_type(board_pieces[i - 2]) << " " << piece_type(board_pieces[i - 1]) << " " << piece_type(board_pieces[i]) << " " << std::endl;
    }
    std::cout << '\n' << std::endl;
}

inline U64 Board::Pawn_Forward(bool IsWhite, U64 mask) {
    if (IsWhite) return mask << 8;
    else return mask >> 8;
}

inline U64 Board::Pawn_Forward2(bool IsWhite, U64 mask) {
    if (IsWhite) return (Pawn_Forward(IsWhite, mask) & ~Occ) << 8;
    else return (Pawn_Forward(IsWhite, mask) & ~Occ) >> 8;
}

inline U64 Board::Pawn_Backward(bool IsWhite, U64 mask) {
    return Pawn_Forward(IsWhite, mask);
}

inline U64 Board::Pawn_Backward2(bool IsWhite, U64 mask) {
    return Pawn_Forward2(IsWhite, mask);
}

U64 Board::Pawn_AttackRight(bool IsWhite, U64 mask) {
    if (IsWhite) return mask << 9;
    else return mask >> 7;
}

inline U64 Board::Pawn_AttackLeft(bool IsWhite, U64 mask) {
    if (IsWhite) return mask << 7;
    else return mask >> 9;
}

U64 Board::King(bool IsWhite) {
    if (IsWhite) return bitboards[WKING];
    else return bitboards[BKING];
}

U64 Board::EnemyKing(bool IsWhite)
{
    if (IsWhite) return bitboards[BKING];
    else return bitboards[WKING];
}

U64 Board::EnemyPawn(bool IsWhite) {
    if (IsWhite) return bitboards[BPAWN];
    return bitboards[WPAWN];
}

U64 Board::Pawns(bool IsWhite) {
    if (IsWhite) return bitboards[WPAWN];
    else return bitboards[BPAWN];
}

U64 Board::OwnColor(bool IsWhite)
{
    if (IsWhite) return White;
    return Black;
}

U64 Board::Enemy(bool IsWhite)
{
    if (IsWhite) return Black;
    return White;
}

U64 Board::EnemyRookQueen(bool IsWhite)
{
    if (IsWhite) return bitboards[BROOK] | bitboards[BQUEEN];
    return bitboards[WROOK] | bitboards[WQUEEN];
}

U64 Board::RookQueen(bool IsWhite)
{
    if (IsWhite) return bitboards[WROOK] | bitboards[WQUEEN];
    return bitboards[BROOK] | bitboards[BQUEEN];
}

U64 Board::EnemyBishopQueen(bool IsWhite)
{
    if (IsWhite) return bitboards[BBISHOP] | bitboards[BQUEEN];
    return bitboards[WBISHOP] | bitboards[WQUEEN];
}

U64 Board::BishopQueen(bool IsWhite)
{
    if (IsWhite) return bitboards[WBISHOP] | bitboards[WQUEEN];
    return bitboards[BBISHOP] | bitboards[BQUEEN];
}

U64 Board::KingPawn(bool IsWhite)
{
    if (IsWhite) return bitboards[WPAWN] | bitboards[WKING];
    return bitboards[BPAWN] | bitboards[BKING];
}

U64 Board::EnemyKingPawn(bool IsWhite)
{
    if (IsWhite) return bitboards[BPAWN] | bitboards[BKING];
    return bitboards[WPAWN] | bitboards[WKING];
}

inline U64 Board::EnemyOrEmpty(bool IsWhite)
{
    if (IsWhite) return ~White;
    return ~Black;
}

U64 Board::Empty()
{
    return ~Occ;
}

U64 Board::Knights(bool IsWhite)
{
    if (IsWhite) return bitboards[WKNIGHT];
    return bitboards[BKNIGHT];
}

U64 Board::Rooks(bool IsWhite)
{
    if (IsWhite) return bitboards[WROOK];
    return bitboards[BROOK];
}

U64 Board::Bishops(bool IsWhite)
{
    if (IsWhite) return bitboards[WBISHOP];
    return bitboards[BBISHOP];
}

U64 Board::Queens(bool IsWhite)
{
    if (IsWhite) return bitboards[WQUEEN];
    return bitboards[BQUEEN];
}

// creates the checkmask
U64 Board::create_checkmask(bool IsWhite, int sq) {
    U64 us = IsWhite ? White : Black;
    U64 enemy = IsWhite ? Black : White;
    U64 checks = 0ULL;
    int index = 0;
    
    // reset doublecheck var
    doublecheck = 0;

    if (_rays[NORTH][sq] & enemy) {
        index = _bitscanforward(enemy & _rays[NORTH][sq]);
        if (_test_bit(EnemyRookQueen(IsWhite), index)) {
            // If this is true the ray is not hitting a piece from us
            if (not(_rays[NORTH][sq] & us)) {
                checks |= _rays[NORTH][sq] & ~_rays[NORTH][index];
                doublecheck++;
            }
            else {
                // We hit a piece from us. If the distance to it is smaller than to the blocker we cannot block the check
                int blockedus = _bitscanforward(_rays[NORTH][sq] & us);
                if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                    checks |= _rays[NORTH][sq] & ~_rays[NORTH][index];
                    doublecheck++;
                }
            }
        }
    }

    if (_rays[SOUTH][sq] & enemy) {
        index = _bitscanreverse(enemy & _rays[SOUTH][sq]);
        if (_test_bit(EnemyRookQueen(IsWhite), index)) {
            if (not(_rays[SOUTH][sq] & us)) {
                checks |= _rays[SOUTH][sq] & ~_rays[SOUTH][index];
                doublecheck++;
            }
            else {
                int blockedus = _bitscanreverse(_rays[SOUTH][sq] & us);
                if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                    checks |= _rays[SOUTH][sq] & ~_rays[SOUTH][index];
                    doublecheck++;
                }
            }
        }
    }

    if (_rays[EAST][sq] & enemy) {
        index = _bitscanforward(enemy & _rays[EAST][sq]);
        if (_test_bit(EnemyRookQueen(IsWhite), index)) {
            if (not(_rays[EAST][sq] & us)) {
                checks |= _rays[EAST][sq] & ~_rays[EAST][index];
                doublecheck++;
            }
            else {
                int blockedus = _bitscanforward(_rays[EAST][sq] & us);
                if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                    checks |= _rays[EAST][sq] & ~_rays[EAST][index];
                    doublecheck++;
                }
            }
        }
    }

    if (_rays[WEST][sq] & enemy) {
        index = _bitscanreverse(enemy & _rays[WEST][sq]);
        if (_test_bit(EnemyRookQueen(IsWhite), index)) {
            if (not(_rays[WEST][sq] & us)) {
                checks |= _rays[WEST][sq] & ~_rays[WEST][index];
                doublecheck++;
            }
            else {
                int blockedus = _bitscanreverse(_rays[WEST][sq] & us);
                if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                    checks |= _rays[WEST][sq] & ~_rays[WEST][index];
                    doublecheck++;
                }
            }
        }
    }

    if (_rays[NORTH_WEST][sq] & enemy) {

        index = _bitscanforward(enemy & _rays[NORTH_WEST][sq]);
        if (_test_bit(EnemyKing(IsWhite), index)) {
            if (square_distance(sq, index) == 1) {
                checks |= _rays[NORTH_WEST][sq] & ~_rays[NORTH_WEST][index];
                doublecheck++;
            }
        }
        if (IsWhite and _test_bit(EnemyPawn(IsWhite), index)) {
            if (square_distance(sq, index) == 1) {
                checks |= _rays[NORTH_WEST][sq] & ~_rays[NORTH_WEST][index];
                doublecheck++;
            }
        }
        if (_test_bit(EnemyBishopQueen(IsWhite), index)) {
            if (not(_rays[NORTH_WEST][sq] & us)) {
                checks |= _rays[NORTH_WEST][sq] & ~_rays[NORTH_WEST][index];
                doublecheck++;
            }
            else {
                int blockedus = _bitscanforward(_rays[NORTH_WEST][sq] & us);
                if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                    checks |= _rays[NORTH_WEST][sq] & ~_rays[NORTH_WEST][index];
                    doublecheck++;
                }
            }
        }
    }

    if (_rays[NORTH_EAST][sq] & enemy) {
        index = _bitscanforward(enemy & _rays[NORTH_EAST][sq]);
        if (_test_bit(EnemyKing(IsWhite), index)) {
            if (square_distance(sq, index) == 1) {
                checks |= _rays[NORTH_EAST][sq] & ~_rays[NORTH_EAST][index];
                doublecheck++;
            }
        }
        if (IsWhite and _test_bit(EnemyPawn(IsWhite), index)) {
            if (square_distance(sq, index) == 1) {
                checks |= _rays[NORTH_EAST][sq] & ~_rays[NORTH_EAST][index];
                doublecheck++;
            }
        }
        if (_test_bit(EnemyBishopQueen(IsWhite), index)) {

            if (not(_rays[NORTH_EAST][sq] & us)) {
                checks |= _rays[NORTH_EAST][sq] & ~_rays[NORTH_EAST][index];
                doublecheck++;
            }
            else {
                int blockedus = _bitscanforward(_rays[NORTH_EAST][sq] & us);
                if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                    checks |= _rays[NORTH_EAST][sq] & ~_rays[NORTH_EAST][index];
                    doublecheck++;
                }
            }
        }
    }

    if (_rays[SOUTH_WEST][sq] & enemy) {
        index = _bitscanreverse(enemy & _rays[SOUTH_WEST][sq]);
        if (_test_bit(EnemyKing(IsWhite), index)) {
            if (square_distance(sq, index) == 1) {
                checks |= _rays[SOUTH_WEST][sq] & ~_rays[SOUTH_WEST][index];
                doublecheck++;
            }
        }
        if (!IsWhite and _test_bit(EnemyPawn(IsWhite), index)) {
            if (square_distance(sq, index) == 1) {
                checks |= _rays[SOUTH_WEST][sq] & ~_rays[SOUTH_WEST][index];
                doublecheck++;
            }
        }
        if (_test_bit(EnemyBishopQueen(IsWhite), index)) {
            if (not(_rays[SOUTH_WEST][sq] & us)) {
                checks |= _rays[SOUTH_WEST][sq] & ~_rays[SOUTH_WEST][index];
                doublecheck++;
            }
            else {
                int blockedus = _bitscanreverse(_rays[SOUTH_WEST][sq] & us);
                if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                    checks |= _rays[SOUTH_WEST][sq] & ~_rays[SOUTH_WEST][index];
                    doublecheck++;
                }
            }
        }
    }

    if (_rays[SOUTH_EAST][sq] & enemy) {
        index = _bitscanreverse(enemy & _rays[SOUTH_EAST][sq]);
        if (_test_bit(EnemyKing(IsWhite), index)) {
            if (square_distance(sq, index) == 1) {
                checks |= _rays[SOUTH_EAST][sq] & ~_rays[SOUTH_EAST][index];
                doublecheck++;
            }
        }
        if (!IsWhite and _test_bit(EnemyPawn(IsWhite), index)) {
            if (square_distance(sq, index) == 1) {
                checks |= _rays[SOUTH_EAST][sq] & ~_rays[SOUTH_EAST][index];
                doublecheck++;
            }
        }
        if (_test_bit(EnemyBishopQueen(IsWhite), index)) {
            if (not(_rays[SOUTH_EAST][sq] & us)) {
                checks |= _rays[SOUTH_EAST][sq] & ~_rays[SOUTH_EAST][index];
                doublecheck++;
            }
            else {
                int blockedus = _bitscanreverse(_rays[SOUTH_EAST][sq] & us);
                if (square_distance(index, sq) < square_distance(blockedus, sq)) {
                    checks |= _rays[SOUTH_EAST][sq] & ~_rays[SOUTH_EAST][index];
                    doublecheck++;
                }
            }
        }
    }

    if (knightattacks[sq] & (Knights(!IsWhite) & enemy)) {
        doublecheck++;
        checks |= knightattacks[sq] & Knights(!IsWhite) & enemy;
    }

    return checks;
}

// checks if removing the piece at square would be an attack on our king
U64 Board::would_be_attack(bool IsWhite, int sq) {
    U64 us = IsWhite ? White : Black;
    U64 enemy = IsWhite ? Black : White;
    U64 checks = 0ULL;
    int index = 0;
    int king_sq = _bitscanforward(King(IsWhite));
    enemy &= ~(1ULL << sq);

    if (_rays[NORTH][king_sq] & enemy) {
        index = _bitscanforward(enemy & _rays[NORTH][king_sq]);
        if (_test_bit(EnemyRookQueen(IsWhite), index)) {
            // If this is true the ray is not hitting a piece from us
            if (not(_rays[NORTH][king_sq] & us)) {
                checks |= _rays[NORTH][king_sq] & ~_rays[NORTH][index];
            }
            else {
                // We hit a piece from us. If the distance to it is smaller than to the blocker we cannot block the check
                int blockedus = _bitscanforward(_rays[NORTH][king_sq] & us);
                if (square_distance(index, king_sq) < square_distance(blockedus, king_sq)) {
                    checks |= _rays[NORTH][king_sq] & ~_rays[NORTH][index];
                }
            }
        }
    }

    if (_rays[SOUTH][king_sq] & enemy) {
        index = _bitscanreverse(enemy & _rays[SOUTH][king_sq]);
        if (_test_bit(EnemyRookQueen(IsWhite), index)) {
            if (not(_rays[SOUTH][king_sq] & us)) {
                checks |= _rays[SOUTH][king_sq] & ~_rays[SOUTH][index];
            }
            else {
                int blockedus = _bitscanreverse(_rays[SOUTH][king_sq] & us);
                if (square_distance(index, king_sq) < square_distance(blockedus, king_sq)) {
                    checks |= _rays[SOUTH][king_sq] & ~_rays[SOUTH][index];
                }
            }
        }
    }

    return checks;
}

// detects if the piece is pinned horizontal or vertical
U64 Board::is_pinned_hv(bool IsWhite, int sq) {
    U64 enemy;
    U64 us;
    enemy = IsWhite ? Black : White;
    us = IsWhite ? White : Black;
    int index;
    U64 checks = 0ULL;
    int king_sq = _bitscanforward(King(IsWhite));
    us &= ~(1ULL << sq);
    if (_rays[NORTH][king_sq] & enemy) {
        index = _bitscanforward(enemy & _rays[NORTH][king_sq]);
        if (_test_bit(EnemyRookQueen(IsWhite), index)) {
            // If this is true the ray is not hitting a piece from us
            if (not(_rays[NORTH][king_sq] & us)) {
                checks |= _rays[NORTH][king_sq] & ~_rays[NORTH][index];
            }
            else {
                // We hit a piece from us. If the distance to it is smaller than to the blocker we cannot block the check
                int blockedus = _bitscanforward(_rays[NORTH][king_sq] & us);
                if (square_distance(index, king_sq) < square_distance(blockedus, king_sq)) {
                    checks |= _rays[NORTH][king_sq] & ~_rays[NORTH][index];
                }
            }
        }
    }

    if (_rays[SOUTH][king_sq] & enemy) {
        index = _bitscanreverse(enemy & _rays[SOUTH][king_sq]);
        if (_test_bit(EnemyRookQueen(IsWhite), index)) {
            if (not(_rays[SOUTH][king_sq] & us)) {
                checks |= _rays[SOUTH][king_sq] & ~_rays[SOUTH][index];
            }
            else {
                int blockedus = _bitscanreverse(_rays[SOUTH][king_sq] & us);
                if (square_distance(index, king_sq) < square_distance(blockedus, king_sq)) {
                    checks |= _rays[SOUTH][king_sq] & ~_rays[SOUTH][index];
                }
            }
        }
    }

    if (_rays[EAST][king_sq] & enemy) {
        index = _bitscanforward(enemy & _rays[EAST][king_sq]);
        if (_test_bit(EnemyRookQueen(IsWhite), index)) {
            if (not(_rays[EAST][king_sq] & us)) {
                checks |= _rays[EAST][king_sq] & ~_rays[EAST][index];
            }
            else {
                int blockedus = _bitscanforward(_rays[EAST][king_sq] & us);
                if (square_distance(index, king_sq) < square_distance(blockedus, king_sq)) {
                    checks |= _rays[EAST][king_sq] & ~_rays[EAST][index];
                }
            }
        }
    }

    if (_rays[WEST][king_sq] & enemy) {
        index = _bitscanreverse(enemy & _rays[WEST][king_sq]);
        if (_test_bit(EnemyRookQueen(IsWhite), index)) {
            if (not(_rays[WEST][king_sq] & us)) {
                checks |= _rays[WEST][king_sq] & ~_rays[WEST][index];
            }
            else {
                int blockedus = _bitscanreverse(_rays[WEST][king_sq] & us);
                if (square_distance(index, king_sq) < square_distance(blockedus, king_sq)) {
                    checks |= _rays[WEST][king_sq] & ~_rays[WEST][index];
                }
            }
        }
    }
    return checks;
}

// detects if the piece is pinned diagonal
U64 Board::is_pinned_dg(bool IsWhite, int sq) {
    U64 enemy;
    U64 us;
    if (IsWhite) {
        enemy = Black;
        us = White;
    }
    else {
        enemy = White;
        us = Black;
    }
    int index;
    U64 checks = 0ULL;
    int king_sq = _bitscanforward(King(IsWhite));

    us &= ~(1ULL << sq);
    if (_rays[NORTH_WEST][king_sq] & enemy) {

        index = _bitscanforward(enemy & _rays[NORTH_WEST][king_sq]);
        if (_test_bit(EnemyKingPawn(IsWhite), index)) {
            if (square_distance(king_sq, index) == 1) {
                checks |= _rays[NORTH_WEST][king_sq] & ~_rays[NORTH_WEST][index];
            }
        }
        if (_test_bit(EnemyBishopQueen(IsWhite), index)) {
            if (not(_rays[NORTH_WEST][king_sq] & us)) {
                checks |= _rays[NORTH_WEST][king_sq] & ~_rays[NORTH_WEST][index];
            }
            else {
                int blockedus = _bitscanforward(_rays[NORTH_WEST][king_sq] & us);
                if (square_distance(index, king_sq) < square_distance(blockedus, king_sq)) {
                    checks |= _rays[NORTH_WEST][king_sq] & ~_rays[NORTH_WEST][index];
                }
            }
        }
    }

    if (_rays[NORTH_EAST][king_sq] & enemy) {
        index = _bitscanforward(enemy & _rays[NORTH_EAST][king_sq]);
        if (_test_bit(EnemyKingPawn(IsWhite), index)) {
            if (square_distance(king_sq, index) == 1) {
                checks |= _rays[NORTH_EAST][king_sq] & ~_rays[NORTH_EAST][index];
            }
        }

        if (_test_bit(EnemyBishopQueen(IsWhite), index)) {

            if (not(_rays[NORTH_EAST][king_sq] & us)) {
                checks |= _rays[NORTH_EAST][king_sq] & ~_rays[NORTH_EAST][index];
            }
            else {
                int blockedus = _bitscanforward(_rays[NORTH_EAST][king_sq] & us);
                if (square_distance(index, king_sq) < square_distance(blockedus, king_sq)) {
                    checks |= _rays[NORTH_EAST][king_sq] & ~_rays[NORTH_EAST][index];
                }
            }
        }
    }

    if (_rays[SOUTH_WEST][king_sq] & enemy) {
        index = _bitscanreverse(enemy & _rays[SOUTH_WEST][king_sq]);
        if (_test_bit(EnemyKingPawn(IsWhite), index)) {
            if (square_distance(king_sq, index) == 1) {
                checks |= _rays[SOUTH_WEST][king_sq] & ~_rays[SOUTH_WEST][index];
            }
        }
        if (_test_bit(EnemyBishopQueen(IsWhite), index)) {
            if (not(_rays[SOUTH_WEST][king_sq] & us)) {
                checks |= _rays[SOUTH_WEST][king_sq] & ~_rays[SOUTH_WEST][index];
            }
            else {
                int blockedus = _bitscanreverse(_rays[SOUTH_WEST][king_sq] & us);
                if (square_distance(index, king_sq) < square_distance(blockedus, king_sq)) {
                    checks |= _rays[SOUTH_WEST][king_sq] & ~_rays[SOUTH_WEST][index];
                }
            }
        }
    }

    if (_rays[SOUTH_EAST][king_sq] & enemy) {
        index = _bitscanreverse(enemy & _rays[SOUTH_EAST][king_sq]);
        if (_test_bit(EnemyKingPawn(IsWhite), index)) {
            if (square_distance(king_sq, index) == 1) {
                checks |= _rays[SOUTH_EAST][king_sq] & ~_rays[SOUTH_EAST][index];
            }
        }
        if (_test_bit(EnemyBishopQueen(IsWhite), index)) {
            if (not(_rays[SOUTH_EAST][king_sq] & us)) {
                checks |= _rays[SOUTH_EAST][king_sq] & ~_rays[SOUTH_EAST][index];
            }
            else {
                int blockedus = _bitscanreverse(_rays[SOUTH_EAST][king_sq] & us);
                if (square_distance(index, king_sq) < square_distance(blockedus, king_sq)) {
                    checks |= _rays[SOUTH_EAST][king_sq] & ~_rays[SOUTH_EAST][index];
                }
            }
        }
    }
    if (not(_test_bit(checks, sq))) {
        return 0ULL;
    }
    return checks;
}

// squares seen by a pawn
U64 Board::seen_by_pawn(bool IsWhite, int sq, int ep) {
    U64 ep_m = (1ULL << ep);
    U64 new_mask = 0ULL;
    U64 mask = (1ULL << sq);
    if (IsWhite) {
        new_mask = Pawn_Forward(IsWhite, mask) & ~Occ | Pawn_Forward2(IsWhite, mask) & ~Occ & ~rank_2_mask | Pawn_AttackLeft(IsWhite, mask) & ~White & Black & not_a_file | Pawn_AttackRight(IsWhite, mask) & ~White & Black & not_h_file;
    }
    if (!IsWhite) {
        new_mask = Pawn_Backward(IsWhite, mask) & ~Occ | Pawn_Backward2(IsWhite, mask) & ~Occ & ~rank_7_mask | Pawn_AttackLeft(IsWhite, mask) & ~Black & White & not_a_file | Pawn_AttackRight(IsWhite, mask) & ~Black & White & not_h_file;
    }
    if (ep < 64 and square_distance(sq, ep) == 1) {
        if (not (mask & pin_hv)) {
            if (IsWhite) {
                White &= ~(1ULL << sq);
                Black &= ~(1ULL << (ep - 8));
                Occ = Black | White;
                int king_sq = _bitscanforward(King(IsWhite));
                if (not(create_checkmask(IsWhite, king_sq))) {
                    White |= (1ULL << sq);
                    Black |= (1ULL << (ep - 8));
                    Occ = Black | White;
                    return new_mask | ep_m;
                }
                White |= (1ULL << sq);
                Black |= (1ULL << (ep - 8));
                Occ = Black | White;

            }
            else {
                Black &= ~(1ULL << sq);
                White &= ~(1ULL << (ep + 8));
                Occ = Black | White;
                int king_sq = _bitscanforward(King(IsWhite));
                if (not(create_checkmask(IsWhite, king_sq))) {
                    Black |= (1ULL << sq);
                    White |= (1ULL << (ep + 8));
                    Occ = Black | White;
                    return new_mask | ep_m;
                }
                Black |= (1ULL << sq);
                White |= (1ULL << (ep + 8));
                Occ = Black | White;
            }
        }
    }
    return new_mask;
}

// squares seen by a bishop
inline U64 Board::seen_by_bishop(bool IsWhite, int sq) {
    U64 bishop_move = 0ULL;
    U64 victims = IsWhite ? Black : White;
    U64 blockers = IsWhite ? White : Black;
    int index = 0;
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

// squares seen by a knight
inline U64 Board::seen_by_knight(int sq) {
    return knightattacks[sq];
}

// squares seen by a rook
inline U64 Board::seen_by_rook(bool IsWhite, int sq) {
    U64 rook_move = 0ULL;
    U64 victims = IsWhite ? Black : White;
    U64 blockers = IsWhite ? White : Black;
    int index = 0;

    rook_move |= (1ULL << sq);
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

// squares seen by a king
inline U64 Board::seen_by_king(int sq) {
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

    return (king_up | king_down | king_right | king_left | king_up_right | king_up_left | king_down_right | king_down_left);
}

// legal pawn moves
U64 Board::valid_pawn_moves(bool IsWhite, int sq, int ep) {
    U64 mask = 1ULL << sq;
    U64 attack_l = 0ULL;
    U64 attack_r = 0ULL;
    U64 pawn_push = 0ULL;
    U64 ep_take = 0ULL;
    U64 ep_m = (1ULL << ep);
    U64 valid_attacks = 0ULL;
    //Save double check
    int save_db = doublecheck;
    // DIAGONAL EN PASSANT PINS DO NOT EXIST !!
    if (ep < 64 and square_distance(sq, ep) == 1) {
        if (not (mask & pin_hv)) {
            if (IsWhite) {
                bitboards[BPAWN] &= ~(1ULL << (ep - 8));
                bitboards[WPAWN] &= ~(1ULL << sq);
                White = bitboards[WPAWN] | bitboards[WKNIGHT] | bitboards[WBISHOP] | bitboards[WROOK] | bitboards[WQUEEN] | bitboards[WKING];
                Black = bitboards[BPAWN] | bitboards[BKNIGHT] | bitboards[BBISHOP] | bitboards[BROOK] | bitboards[BQUEEN] | bitboards[BKING];
                Occ = Black | White;
                int king_sq = _bitscanforward(King(IsWhite));
                if (not(create_checkmask(IsWhite, king_sq)) and square_rank(sq) == 4) {
                    if (square_file(ep) != 7 and square_file(sq) == 0) {
                        ep_take |= ep_m;
                    }
                    if (square_file(ep) != 0 and square_file(sq) == 7) {
                        ep_take |= ep_m;
                    }
                    if (square_file(ep) != 0 and square_file(sq) != 7) {
                        ep_take |= ep_m;
                    }
                    if (square_file(ep) == 0 and square_file(sq) == 1) {
                        ep_take |= ep_m;
                    }
                    if (square_file(ep) == 7 and square_file(sq) == 6) {
                        ep_take |= ep_m;
                    }
                }
                bitboards[WPAWN] |= 1ULL << sq;
                bitboards[BPAWN] |= 1ULL << (ep - 8);
                White = bitboards[WPAWN] | bitboards[WKNIGHT] | bitboards[WBISHOP] | bitboards[WROOK] | bitboards[WQUEEN] | bitboards[WKING];
                Black = bitboards[BPAWN] | bitboards[BKNIGHT] | bitboards[BBISHOP] | bitboards[BROOK] | bitboards[BQUEEN] | bitboards[BKING];
                Occ = Black | White;
            }
            else {
                bitboards[BPAWN] &= ~(1ULL << sq);
                bitboards[WPAWN] &= ~(1ULL << (ep + 8));
                White = bitboards[WPAWN] | bitboards[WKNIGHT] | bitboards[WBISHOP] | bitboards[WROOK] | bitboards[WQUEEN] | bitboards[WKING];
                Black = bitboards[BPAWN] | bitboards[BKNIGHT] | bitboards[BBISHOP] | bitboards[BROOK] | bitboards[BQUEEN] | bitboards[BKING];
                Occ = Black | White;
                int king_sq = _bitscanforward(King(IsWhite));
                if (not(create_checkmask(IsWhite, king_sq)) and square_rank(sq) == 3) {
                    if (square_file(ep) != 7 and square_file(sq) == 0) {
                        ep_take |= ep_m;
                    }
                    if (square_file(ep) != 0 and square_file(sq) == 7) {
                        ep_take |= ep_m;
                    }
                    if (square_file(ep) != 0 and square_file(sq) != 7) {
                        ep_take |= ep_m;
                    }
                    if (square_file(ep) == 0 and square_file(sq) == 1) {
                        ep_take |= ep_m;
                    }
                    if (square_file(ep) == 7 and square_file(sq) == 6) {
                        ep_take |= ep_m;
                    }
                }
                bitboards[BPAWN] |= 1ULL << sq;
                bitboards[WPAWN] |= 1ULL << (ep + 8);
                White = bitboards[WPAWN] | bitboards[WKNIGHT] | bitboards[WBISHOP] | bitboards[WROOK] | bitboards[WQUEEN] | bitboards[WKING];
                Black = bitboards[BPAWN] | bitboards[BKNIGHT] | bitboards[BBISHOP] | bitboards[BROOK] | bitboards[BQUEEN] | bitboards[BKING];
                Occ = Black | White;
            }
        }
    }
    //Reset doublecheck
    doublecheck = save_db;
    // Ep for vertical takes 8/2p5/3p2k1/KP5r/5pP1/8/4P3/6R1 b - g3 0 3
    if (ep != 64 and IsWhite and square_distance(sq, ep) == 1 and square_rank(sq) == 4) {
        if (would_be_attack(true, ep - 8) and not ep_take and doublecheck == 0) {
            ep_take |= ep_m;
        }
    }
    if (ep != 64 and !IsWhite and square_distance(sq, ep) == 1 and square_rank(sq) == 3) {
        if (would_be_attack(false, ep + 8) and not ep_take and doublecheck == 0) {
            ep_take |= ep_m;
        }
    }

    if (IsWhite) {
        attack_l = Pawn_AttackLeft(IsWhite, mask) & Black & not_h_file;
        attack_r = Pawn_AttackRight(IsWhite, mask) & Black & not_a_file;
        pawn_push = Pawn_Forward(IsWhite, mask) & ~Occ | Pawn_Forward2(IsWhite, mask) & ~Occ & ~rank_2_mask;
    }
    else {
        attack_l = Pawn_AttackLeft(IsWhite, mask) & White & not_h_file;
        attack_r = Pawn_AttackRight(IsWhite, mask) & White & not_a_file;
        pawn_push = Pawn_Backward(IsWhite, mask) & ~Occ | Pawn_Backward2(IsWhite, mask) & ~Occ & ~rank_7_mask;
    }

    //if (not(is_pinned_dg(IsWhite, sq))) { Keep this for safety reason the version below should work
    if (not _test_bit(pin_dg, sq)) {
        valid_attacks |= attack_r;
        valid_attacks |= attack_l;
    }
    else {
        valid_attacks |= attack_l & pin_dg;
        valid_attacks |= attack_r & pin_dg;
        return valid_attacks & checkmask;
    }
    if (_test_bit(pin_hv, sq)) {
        return pawn_push & pin_hv & checkmask;
    }
    // 8/8/3p4/1Pp3kr/1K3R2/8/4P1P1/8 w - c6 0 3
    int king_sq = _bitscanforward(King(IsWhite));
    if (IsWhite and ep != 64 and square_distance(king_sq, ep - 8) == 1 and square_distance(sq, ep - 8) == 1) {
        if (square_distance(king_sq, ep - 8) == 1 and doublecheck == 1 and not _test_bit(pin_hv, sq)) {// not is_pinned_hv(IsWhite, sq)) {
            return (ep_take);
        }
    }
    if (!IsWhite and ep != 64 and square_distance(king_sq, ep + 8) == 1 and square_distance(sq, ep + 8) == 1) {
        if (square_distance(king_sq, ep + 8) == 1 and doublecheck == 1 and not _test_bit(pin_hv, sq)) { //not is_pinned_hv(IsWhite, sq)) {
            return (ep_take);
        }
    }
    return (pawn_push | valid_attacks | ep_take) & checkmask;
}

// legal knight moves
inline U64 Board::valid_knight_moves(bool IsWhite, int sq) {
    if (_test_bit(pin_dg, sq) or _test_bit(pin_hv, sq)) {
        return 0ULL;
    }
    return seen_by_knight(sq) & EnemyOrEmpty(IsWhite) & checkmask;
}

// legal bishop moves
U64 Board::valid_bishop_moves(bool IsWhite, int sq) {
    if (_test_bit(pin_dg, sq)) {
        return seen_by_bishop(IsWhite, sq) & EnemyOrEmpty(IsWhite) & checkmask & pin_dg;
    }
    if (_test_bit(pin_hv, sq)) {
        return 0ULL;
    }

    return seen_by_bishop(IsWhite, sq) & EnemyOrEmpty(IsWhite) & checkmask;
}

// legal rook moves
inline U64 Board::valid_rook_moves(bool IsWhite, int sq) {
    if (_test_bit(pin_hv, sq)) {
        return seen_by_rook(IsWhite, sq) & EnemyOrEmpty(IsWhite) & checkmask & pin_hv;
    }
    if (_test_bit(pin_dg, sq)) {
        return 0ULL;
    }
    return seen_by_rook(IsWhite, sq) & EnemyOrEmpty(IsWhite) & checkmask;
}

// legal queen moves
inline U64 Board::valid_queen_moves(bool IsWhite, int sq) {
    return valid_rook_moves(IsWhite, sq) | valid_bishop_moves(IsWhite, sq);
}

// legal king moves
U64 Board::valid_king_moves(bool IsWhite, int sq) {
    U64 moves = seen_by_king(sq) & EnemyOrEmpty(IsWhite);
    U64 final_moves = 0ULL;
    int to_index;
    // Remove King 
    if (IsWhite) {
        White &= ~(1ULL << sq);
        Occ = Black | White;
    }
    else {
        Black &= ~(1ULL << sq);
        Occ = Black | White;
    }
    while (moves) {
        to_index = _bitscanforward(moves);
        final_moves |= not(is_square_attacked(IsWhite, to_index)) ? (1ULL << to_index) : 0ULL;
        moves &= ~(1ULL << to_index);
    }
    // Place King back
    if (IsWhite) {
        White |= (1ULL << sq);
        Occ = Black | White;
    }
    else {
        Black |= (1ULL << sq);
        Occ = Black | White;
    }

    // Castling
    unsigned long rook_index;
    U64 blockers = Occ;
    U64 castling_index = 0ULL;
    //King side White
    if (castling_rights & wk and (_rays[EAST][sq] & blockers) and IsWhite) {
        rook_index = _bitscanforward(blockers & _rays[EAST][sq]);
        if (rook_index == 7) {
            if (not is_square_attacked(IsWhite, 4) and not is_square_attacked(IsWhite, 5) and not is_square_attacked(IsWhite, 6)) {
                castling_index |= 1ULL << 6;
                final_moves |= castling_index;
            }
        }
    }
    //Queen side White
    if (castling_rights & wq and (_rays[WEST][sq] & blockers) and IsWhite) {
        rook_index = _bitscanreverse(blockers & _rays[WEST][sq]);
        if (rook_index == 0) {
            if (not is_square_attacked(IsWhite, 4) and not is_square_attacked(IsWhite, 3) and not is_square_attacked(IsWhite, 2)) {
                castling_index |= 1ULL << 2;
                final_moves |= castling_index;
            }
        }
    }
    //King side Black
    if (castling_rights & bk and (_rays[EAST][sq] & blockers) and not IsWhite) {
        rook_index = _bitscanforward(blockers & _rays[EAST][sq]);
        if (rook_index == 63) {
            if (not is_square_attacked(IsWhite, 62) and not is_square_attacked(IsWhite, 61) and not is_square_attacked(IsWhite, 60)) {
                castling_index |= 1ULL << 62;
                final_moves |= castling_index;
            }
        }
    }
    //Queen side Black
    if (castling_rights & bq and (_rays[WEST][sq] & blockers) and not IsWhite) {
        rook_index = _bitscanreverse(blockers & _rays[WEST][sq]);
        if (rook_index == 56) {
            if (not is_square_attacked(IsWhite, 60) and not is_square_attacked(IsWhite, 59) and not is_square_attacked(IsWhite, 58)) {

                castling_index |= 1ULL << 58;
                final_moves |= castling_index;
            }
        }
    }
    return final_moves;
}

// detects if the square is attacked
bool Board::is_square_attacked(bool IsWhite, int sq) {
    U64 us = IsWhite ? White : Black;
    U64 enemy = IsWhite ? Black : White;
    unsigned long index = 0;

    if (_rays[NORTH][sq] & enemy) {
        index = _bitscanforward(enemy & _rays[NORTH][sq]);
        if (_test_bit(EnemyKing(IsWhite), index)) {
            if (square_distance(sq, index) == 1) {
                return true;
            }
        }
        if (_test_bit(EnemyRookQueen(IsWhite), index)) {
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
        if (_test_bit(EnemyKing(IsWhite), index)) {
            if (square_distance(sq, index) == 1) {

                return true;
            }
        }
        if (_test_bit(EnemyRookQueen(IsWhite), index)) {
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
        if (_test_bit(EnemyKing(IsWhite), index)) {
            if (square_distance(sq, index) == 1) {
                return true;
            }
        }
        if (_test_bit(EnemyRookQueen(IsWhite), index)) {
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
        if (_test_bit(EnemyKing(IsWhite), index)) {
            if (square_distance(sq, index) == 1) {
                return true;
            }
        }
        if (_test_bit(EnemyRookQueen(IsWhite), index)) {
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
        if (_test_bit(EnemyKing(IsWhite), index)) {
            if (square_distance(sq, index) == 1) {

                return true;
            }
        }
        if (IsWhite and _test_bit(EnemyPawn(IsWhite), index)) {
            if (square_distance(sq, index) == 1) {

                return true;
            }
        }
        if (_test_bit(EnemyBishopQueen(IsWhite), index)) {
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
        if (_test_bit(EnemyKing(IsWhite), index)) {
            if (square_distance(sq, index) == 1) {
                return true;
            }
        }
        if (IsWhite and _test_bit(EnemyPawn(IsWhite), index)) {
            if (square_distance(sq, index) == 1) {
                return true;
            }
        }
        if (_test_bit(EnemyBishopQueen(IsWhite), index)) {
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
        if (_test_bit(EnemyKing(IsWhite), index)) {
            if (square_distance(sq, index) == 1) {
                return true;
            }
        }
        if (!IsWhite and _test_bit(EnemyPawn(IsWhite), index)) {
            if (square_distance(sq, index) == 1) {
                return true;
            }
        }
        if (_test_bit(EnemyBishopQueen(IsWhite), index)) {
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
        if (_test_bit(EnemyKing(IsWhite), index)) {
            if (square_distance(sq, index) == 1) {
                return true;
            }
        }
        if (!IsWhite and _test_bit(EnemyPawn(IsWhite), index)) {
            if (square_distance(sq, index) == 1) {
                return true;
            }
        }
        if (_test_bit(EnemyBishopQueen(IsWhite), index)) {
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

    if (knightattacks[sq] & (Knights(!IsWhite) & enemy)) {
        return true;
    }

    return false;
}

// creates all pins
inline void Board::create_pin_masks(bool IsWhite) {
    U64 us = IsWhite ? White : Black;
    int index = -1;
    while (us) {
        index = _bitscanforward(us);
        U64 dg = is_pinned_dg(IsWhite, index);
        U64 hv = is_pinned_hv(IsWhite, index);
        pin_dg |= dg ? dg : 0ULL;
        pin_hv |= hv ? hv : 0ULL;
        us &= ~(1ULL << index);
    }
}

// initialises variables for move generation
void Board::init(bool IsWhite) {
    int king_sq = _bitscanforward(King(IsWhite));
    U64 r = create_checkmask(IsWhite, king_sq);
    checkmask = r ? r : 18446744073709551615ULL;
    // reset pin masks
    pin_dg = 0ULL;
    pin_hv = 0ULL;
    create_pin_masks(IsWhite);
}

// detects checkmate
bool Board::is_checkmate(bool IsWhite) {
    int king_sq = _bitscanforward(King(IsWhite));
    init(IsWhite);
    if (is_square_attacked(IsWhite, king_sq)) {
        init(IsWhite);
        // only king moves are valid
        if (doublecheck == 2) {
            valid_king_moves(IsWhite, king_sq);
            if (not valid_king_moves(IsWhite, king_sq)) {
                return true;
            }
        }
        // test all moves
        if (doublecheck == 1) {
            generate_legal_moves();
            if (move_stack.size() == 0) {
                return true;
            }
        }
    }
    return false;
}

// detects stalemate
bool Board::is_stalemate(bool IsWhite) {
    int king_sq = _bitscanforward(King(IsWhite));
    init(IsWhite);
    // if we have a legal king move it cannot be stalemate
    if (valid_king_moves(IsWhite, king_sq)) {
        return false;
    }
    int from_index;
    U64 we = IsWhite ? White : Black;
    U64 pawn_mask = (bitboards[BPAWN] | bitboards[WPAWN]) & we;
    U64 knight_mask = (bitboards[BKNIGHT] | bitboards[WKNIGHT]) & we;
    U64 bishop_mask = (bitboards[BBISHOP] | bitboards[WBISHOP]) & we;
    U64 rook_mask = (bitboards[BROOK] | bitboards[WROOK]) & we;
    U64 queen_mask = (bitboards[BQUEEN] | bitboards[WQUEEN]) & we;
    U64 move_mask = 0ULL;
    //Easiest to look up
    while (knight_mask) {
        from_index = _bitscanforward(knight_mask);
        move_mask = valid_knight_moves(IsWhite, from_index);
        if (move_mask) {
            return false;
        }
        knight_mask = _blsr_u64(knight_mask);
    }
    //Highest probabilty that any of these can move
    while (pawn_mask) {
        from_index = _bitscanforward(pawn_mask);
        move_mask = valid_pawn_moves(IsWhite, from_index, en_passant_square);
        if (move_mask) {
            return false;
        }
        pawn_mask = _blsr_u64(pawn_mask);
    }
    while (bishop_mask) {
        from_index = _bitscanforward(bishop_mask);
        move_mask = valid_bishop_moves(IsWhite, from_index);
        if (move_mask) {
            return false;
        }
        bishop_mask = _blsr_u64(bishop_mask);
    }
    while (rook_mask) {
        from_index = _bitscanforward(rook_mask);
        move_mask = valid_rook_moves(IsWhite, from_index);
        if (move_mask) {
            return false;
        }
        rook_mask = _blsr_u64(rook_mask);
    }
    while (queen_mask) {
        from_index = _bitscanforward(queen_mask);
        move_mask = valid_queen_moves(IsWhite, from_index);
        if (move_mask) {
            return false;
        }
        queen_mask = _blsr_u64(queen_mask);
    }
    return true;
}

// returns 1 for checkmate 0 for stalemate and -1 for not over
int Board::is_game_over(bool IsWhite) {
    if (is_checkmate(IsWhite)) {
        return 1;
    }
    if (is_stalemate(IsWhite)) {
        return 0;
    }
    return -1;
}

MoveList Board::generate_legal_moves() {
    Move move;
    MoveList possible_moves{};
    int from_index = 0;
    int enemy = side_to_move ^ 1;
    U64 we = side_to_move ? Black : White;
    bool IsWhite = side_to_move ? false : true;
    U64 pawn_mask = (bitboards[BPAWN] | bitboards[WPAWN]) & we;
    U64 knight_mask = (bitboards[BKNIGHT] | bitboards[WKNIGHT]) & we;
    U64 bishop_mask = (bitboards[BBISHOP] | bitboards[WBISHOP]) & we;
    U64 rook_mask = (bitboards[BROOK] | bitboards[WROOK]) & we;
    U64 queen_mask = (bitboards[BQUEEN] | bitboards[WQUEEN]) & we;
    U64 king_mask = (bitboards[BKING] | bitboards[WKING]) & we;
    int king_sq = _bitscanforward(king_mask);
    init(IsWhite);
    U64 move_mask = 0ULL;
    if (doublecheck < 2) {
        while (pawn_mask) {
            from_index = _bitscanforward(pawn_mask);
            move_mask = valid_pawn_moves(IsWhite, from_index, en_passant_square);
            while (move_mask) {
                int to_index = _bitscanforward(move_mask);
                move.piece = PAWN;
                move.from_square = from_index;
                move.to_square = to_index;
                move.promotion = -1;
                if (square_rank(to_index) == 7 or square_rank(to_index) == 0) {
                    move.promotion = QUEEN;
                    possible_moves.movelist[possible_moves.e] = move;
                    possible_moves.e++;
                    move.promotion = ROOK;
                    possible_moves.movelist[possible_moves.e] = move;
                    possible_moves.e++;
                    move.promotion = KNIGHT;
                    possible_moves.movelist[possible_moves.e] = move;
                    possible_moves.e++;
                    move.promotion = BISHOP;
                    possible_moves.movelist[possible_moves.e] = move;
                    possible_moves.e++;
                }
                else {
                    move.promotion = -1;
                    possible_moves.movelist[possible_moves.e] = move;
                    possible_moves.e++;
                }
                move_mask = _blsr_u64(move_mask);
            }
            pawn_mask = _blsr_u64(pawn_mask);
        }
        while (knight_mask) {
            from_index = _bitscanforward(knight_mask);
            move_mask = valid_knight_moves(IsWhite, from_index);
            while (move_mask) {
                int to_index = _bitscanforward(move_mask);
                move.piece = KNIGHT;
                move.from_square = from_index;
                move.to_square = to_index;
                move.promotion = -1;
                possible_moves.movelist[possible_moves.e] = move;
                possible_moves.e++;
                move_mask = _blsr_u64(move_mask);
            }
            knight_mask = _blsr_u64(knight_mask);
        }
        while (bishop_mask) {
            from_index = _bitscanforward(bishop_mask);
            move_mask = valid_bishop_moves(IsWhite, from_index);
            while (move_mask) {
                int to_index = _bitscanforward(move_mask);
                move.piece = BISHOP;
                move.from_square = from_index;
                move.to_square = to_index;
                move.promotion = -1;
                possible_moves.movelist[possible_moves.e] = move;
                possible_moves.e++;
                move_mask = _blsr_u64(move_mask);
            }
            bishop_mask = _blsr_u64(bishop_mask);
        }
        while (rook_mask) {
            from_index = _bitscanforward(rook_mask);
            move_mask = valid_rook_moves(IsWhite, from_index);
            while (move_mask) {
                int to_index = _bitscanforward(move_mask);
                move.piece = ROOK;
                move.from_square = from_index;
                move.to_square = to_index;
                move.promotion = -1;
                possible_moves.movelist[possible_moves.e] = move;
                possible_moves.e++;
                move_mask = _blsr_u64(move_mask);
            }
            rook_mask = _blsr_u64(rook_mask);
        }
        while (queen_mask) {
            from_index = _bitscanforward(queen_mask);
            move_mask = valid_queen_moves(IsWhite, from_index);

            while (move_mask) {
                int to_index = _bitscanforward(move_mask);
                move.piece = QUEEN;
                move.from_square = from_index;
                move.to_square = to_index;
                move.promotion = -1;
                possible_moves.movelist[possible_moves.e] = move;
                possible_moves.e++;
                move_mask = _blsr_u64(move_mask);
            }
            queen_mask = _blsr_u64(queen_mask);
        }
    }

    move_mask = valid_king_moves(IsWhite, king_sq);
    while (move_mask) {
        int to_index = _bitscanforward(move_mask);
        move.piece = KING;
        move.from_square = king_sq;
        move.to_square = to_index;
        move.promotion = -1;
        possible_moves.movelist[possible_moves.e] = move;
        possible_moves.e++;
        move_mask = _blsr_u64(move_mask);
    }
    king_mask = _blsr_u64(king_mask);
    return possible_moves;
}

MoveList Board::generate_capture_moves() {
    MoveList possible_moves;
    Move move;
    memset(possible_moves.movelist, 0, sizeof(possible_moves.movelist));
    U64 we = side_to_move ? Black : White;
    int enemy = side_to_move ? 1 : 0;
    bool IsWhite = side_to_move ? 0 : 1;

    U64 pawn_mask = (bitboards[BPAWN] | bitboards[WPAWN]) & we;
    U64 knight_mask = (bitboards[BKNIGHT] | bitboards[WKNIGHT]) & we;
    U64 bishop_mask = (bitboards[BBISHOP] | bitboards[WBISHOP]) & we;
    U64 rook_mask = (bitboards[BROOK] | bitboards[WROOK]) & we;
    U64 queen_mask = (bitboards[BQUEEN] | bitboards[WQUEEN]) & we;
    U64 king_mask = (bitboards[BKING] | bitboards[WKING]) & we;
    int king_sq = _bitscanforward(king_mask);
    init(IsWhite);
    create_pin_masks(IsWhite);
    int from_square = -1;
    int to_square = -1;
    U64 move_mask = 0ULL;
    possible_moves.e = 0;
    bool in_check = is_square_attacked(IsWhite, king_sq);
    if (possible_moves.e < 0) {
        return {};
    }
    while (pawn_mask) {
        from_square = _bitscanforward(pawn_mask);
        move_mask = valid_pawn_moves(IsWhite, from_square, en_passant_square);
        while (move_mask) {
            to_square = _bitscanforward(move_mask);
            move.piece = PAWN;
            move.from_square = from_square;
            move.to_square = to_square;
            if (piece_at_square(to_square) != -1 or square_rank(to_square) == 7 or square_rank(to_square) == 0 or in_check) {
                if (square_rank(to_square) == 7 or square_rank(to_square) == 0) {
                    move.promotion = QUEEN;
                    possible_moves.movelist[possible_moves.e] = move;
                    possible_moves.e++;
                    move.promotion = ROOK;
                    possible_moves.movelist[possible_moves.e] = move;
                    possible_moves.e++;
                    move.promotion = KNIGHT;
                    possible_moves.movelist[possible_moves.e] = move;
                    possible_moves.e++;
                    move.promotion = BISHOP;
                    possible_moves.movelist[possible_moves.e] = move;
                    possible_moves.e++;
                }
                else {
                    move.promotion = -1;
                    possible_moves.movelist[possible_moves.e] = move;
                    possible_moves.e++;
                }
            }
            move_mask = _blsr_u64(move_mask);
        }
        pawn_mask = _blsr_u64(pawn_mask);
    }
    while (knight_mask) {
        from_square = _bitscanforward(knight_mask);
        move_mask = valid_knight_moves(IsWhite, from_square);
        while (move_mask) {
            to_square = _bitscanforward(move_mask);
            move.piece = KNIGHT;
            move.from_square = from_square;
            move.to_square = to_square;
            if (piece_at_square(to_square) != -1 or in_check) {
                move.promotion = -1;
                possible_moves.movelist[possible_moves.e] = move;
                possible_moves.e++;
            }
            move_mask = _blsr_u64(move_mask);
        }
        knight_mask = _blsr_u64(knight_mask);
    }
    while (bishop_mask) {
        from_square = _bitscanforward(bishop_mask);
        move_mask = valid_knight_moves(IsWhite, from_square);
        while (move_mask) {
            to_square = _bitscanforward(move_mask);
            move.piece = BISHOP;
            move.from_square = from_square;
            move.to_square = to_square;
            if (piece_at_square(to_square) != -1 or in_check) {
                move.promotion = -1;
                possible_moves.movelist[possible_moves.e] = move;
                possible_moves.e++;
            }
            move_mask = _blsr_u64(move_mask);
        }
        bishop_mask = _blsr_u64(bishop_mask);
    }
    while (rook_mask) {
        from_square = _bitscanforward(rook_mask);
        move_mask = valid_knight_moves(IsWhite, from_square);
        while (move_mask) {
            to_square = _bitscanforward(move_mask);
            move.piece = ROOK;
            move.from_square = from_square;
            move.to_square = to_square;
            if (piece_at_square(to_square) != -1 or in_check) {
                move.promotion = -1;
                possible_moves.movelist[possible_moves.e] = move;
                possible_moves.e++;
            }
            move_mask = _blsr_u64(move_mask);
        }
        rook_mask = _blsr_u64(rook_mask);
    }
    while (queen_mask) {
        from_square = _bitscanforward(queen_mask);
        move_mask = valid_knight_moves(IsWhite, from_square);
        while (move_mask) {
            to_square = _bitscanforward(move_mask);
            move.piece = QUEEN;
            move.from_square = from_square;
            move.to_square = to_square;
            if (piece_at_square(to_square) != -1 or in_check) {
                move.promotion = -1;
                possible_moves.movelist[possible_moves.e] = move;
                possible_moves.e++;
            }
            move_mask = _blsr_u64(move_mask);
        }
        queen_mask = _blsr_u64(queen_mask);
    }
    while (king_mask) {
        from_square = _bitscanforward(king_mask);
        move_mask = valid_knight_moves(IsWhite, from_square);
        while (move_mask) {
            to_square = _bitscanforward(move_mask);
            move.piece = KING;
            move.from_square = from_square;
            move.to_square = to_square;
            if (piece_at_square(to_square) != -1 or in_check) {
                move.promotion = -1;
                possible_moves.movelist[possible_moves.e] = move;
                possible_moves.e++;
            }
            move_mask = _blsr_u64(move_mask);
        }
        king_mask = _blsr_u64(king_mask);
    }
    return possible_moves;
}

U64 Perft::speed_test_perft(int depth, int max) {
    U64 nodes = 0;
    MA myarray;
    Pertft_Info pf;
    if (depth == 0) {
        return 1;
    }
    else {
        MoveList n_moves = this_board->generate_legal_moves();
        int count = n_moves.e;
        for (int i = 0; i < count; i++) {
            Move move = n_moves.movelist[i];
            this_board->make_move(move);
            nodes += speed_test_perft(depth - 1, depth);
            this_board->unmake_move();
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
    for (int i = 0; i < myarray.myarray.size(); i++)
    {
        std::cout << square_to_coordinates[myarray.myarray[i].from_square];
        std::cout << square_to_coordinates[myarray.myarray[i].to_square];
        std::cout << " ";

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
};

U64 Perft::bulk_test_perft(int depth, int max) {
    U64 nodes = 0;
    MA myarray;
    Pertft_Info pf;
    MoveList n_moves = this_board->generate_legal_moves();
    int count = n_moves.e;
    if (depth == 1) {
        return count;
    }
    else {
        for (int i = 0; i < count; i++) {
            Move move = n_moves.movelist[i];
            this_board->make_move(move);
            nodes += bulk_test_perft(depth - 1, depth);
            this_board->unmake_move();
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
    for (int i = 0; i < myarray.myarray.size(); i++)
    {
        std::cout << square_to_coordinates[myarray.myarray[i].from_square];
        std::cout << square_to_coordinates[myarray.myarray[i].to_square];
        std::cout << " ";

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
};

std::string Board::piece_type(int piece) {
    if (piece == -1) {
        return "-";
    }
    if (piece == 0) {
        return "P";
    }
    if (piece == 1) {
        return "N";
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
        return "n";
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

void Perft::test() {
    std::string fen1 = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::string fen2 = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::string fen3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
    std::string fen4 = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    std::string fen5 = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
    std::string fen6 = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ";
    auto begin = std::chrono::high_resolution_clock::now();

    this_board->apply_fen(fen1);
    int count = 0;

    auto begin2 = std::chrono::high_resolution_clock::now();
    if (bulk_test_perft(6, 6) == 119060324) { // 4 == 197281     5 == 4865609    6 == 119060324	
        count++;
        std::cout << "\n" << "Correct" << "\n" << std::endl;
    }
    else {
        std::cout << "\n" << "False" << "\n" << std::endl;
    };
    auto end2 = std::chrono::high_resolution_clock::now();
    auto time_diff2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - begin2).count();
    std::cout << time_diff2 / 1000000000.0f << " seconds" << "\n" << std::endl;

    this_board->apply_fen(fen2);
    begin2 = std::chrono::high_resolution_clock::now();
    if (bulk_test_perft(5, 5) == 193690690) { // 4 == 4085603      3 == 97862       5 == 193690690
        count++;
        std::cout << "\n" << "Correct" << "\n" << std::endl;
    }
    else {
        std::cout << "\n" << "False" << "\n" << std::endl;
    };
    end2 = std::chrono::high_resolution_clock::now();
    time_diff2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - begin2).count();
    std::cout << time_diff2 / 1000000000.0f << " seconds" << "\n" << std::endl;

    this_board->apply_fen(fen3);
    begin2 = std::chrono::high_resolution_clock::now();
    if (bulk_test_perft(7, 7) == 178633661) {    // 6 == 11030083        5 == 674624
        count++;
        std::cout << "\n" << "Correct" << "\n" << std::endl;
    }
    else {
        std::cout << "\n" << "False" << "\n" << std::endl;
    };
    end2 = std::chrono::high_resolution_clock::now();
    time_diff2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - begin2).count();
    std::cout << time_diff2 / 1000000000.0f << " seconds" << "\n" << std::endl;

    this_board->apply_fen(fen4);
    begin2 = std::chrono::high_resolution_clock::now();
    if (bulk_test_perft(6, 6) == 706045033) {    // 5 == 15833292        4 == 422333
        count++;
        std::cout << "\n" << "Correct" << "\n" << std::endl;
    }
    else {
        std::cout << "\n" << "False" << "\n" << std::endl;
    };
    end2 = std::chrono::high_resolution_clock::now();
    time_diff2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - begin2).count();
    std::cout << time_diff2 / 1000000000.0f << " seconds" << "\n" << std::endl;

    this_board->apply_fen(fen5);
    begin2 = std::chrono::high_resolution_clock::now();
    if (bulk_test_perft(5, 5) == 89941194) {   //5 == 89941194    4 ==2103487     6 == 3048196529
        count++;
        std::cout << "\n" << "Correct" << "\n" << std::endl;
    }
    else {
        std::cout << "\n" << "False" << "\n" << std::endl;
    };
    end2 = std::chrono::high_resolution_clock::now();
    time_diff2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - begin2).count();
    std::cout << time_diff2 / 1000000000.0f << " seconds" << "\n" << std::endl;

    this_board->apply_fen(fen6);
    begin2 = std::chrono::high_resolution_clock::now();
    if (bulk_test_perft(5, 5) == 164075551) {     // 4 == 3894594     3 == 89890     5 == 164075551
        count++;
        std::cout << "\n" << "Correct" << "\n" << std::endl;
    }
    else {
        std::cout << "\n" << "False" << "\n" << std::endl;
    };
    end2 = std::chrono::high_resolution_clock::now();
    time_diff2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - begin2).count();
    std::cout << time_diff2 / 1000000000.0f << " seconds" << "\n" << std::endl;

    std::cout << "Finished perft positions" << std::endl;
    std::cout << "\n" << count << "/" << "6" << " Correct" << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    auto time_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
    U64 total = 164075551ULL + 89941194ULL + 706045033ULL + 178633661ULL + 193690690ULL + 119060324ULL;
    std::cout << std::fixed << "nps " << total / (time_diff / 1000000000.0f) << " time " << time_diff / 1000000000.0f << " seconds" << std::endl;
}