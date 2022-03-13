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

#include "general.h"
#include "board.h"
#include <chrono>

std::vector<std::string> Board::split_fen(std::string fen)
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
}
int Board::get_en_passant_square() {
    return en_passant_square;
}
inline void Board::update_occupancies() {
    WPawn = bitboards[WPAWN];
    WKnight = bitboards[WKNIGHT];
    WBishop = bitboards[WBISHOP];
    WRook = bitboards[WROOK];
    WQueen = bitboards[WQUEEN];
    WKing = bitboards[WKING];
    BPawn = bitboards[BPAWN];
    BKnight = bitboards[BKNIGHT];
    BBishop = bitboards[BBISHOP];
    BRook = bitboards[BROOK];
    BQueen = bitboards[BQUEEN];
    BKing = bitboards[BKING];
    White = bitboards[WPAWN] | bitboards[WKNIGHT] | bitboards[WBISHOP] | bitboards[WROOK] | bitboards[WQUEEN] | bitboards[WKING];
    Black = bitboards[BPAWN] | bitboards[BKNIGHT] | bitboards[BBISHOP] | bitboards[BROOK] | bitboards[BQUEEN] | bitboards[BKING];
    Occ = White | Black;
    occupancies[0] = White;
    occupancies[1] = Black;
}
inline BoardState Board::encode_board_state(U64 wpawn, U64 wknight, U64 wbishop, U64 wrook, U64 wqueen, U64 wking,
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
inline void Board::make_move(Move& move) {

    int from_square = move.from_square;
    int to_square = move.to_square;
    int promotion_piece = move.promotion;
    int piece = move.piece;
    piece = piece + (side_to_move * 6);
    int captured_piece = -1;
    // piece needs to be set at its bitboard remove this for performance if you are 100% theres a piece at that square
    if (not _test_bit(bitboards[piece], to_square)) {
        BoardState boardstate = encode_board_state(bitboards[WPAWN], bitboards[WKNIGHT], bitboards[WBISHOP], bitboards[WROOK], bitboards[WQUEEN], bitboards[WKING], bitboards[BPAWN], bitboards[BKNIGHT], bitboards[BBISHOP], bitboards[BROOK], bitboards[BQUEEN], bitboards[BKING], en_passant_square, castling_rights);
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

        // Actual castling
        if (piece == WKING and square_distance(from_square, to_square) == 2) {
            if (to_square == 6) {
                if (castling_rights & wk) {
                    castling_rights ^= wk;
                    bitboards[WROOK] &= ~(1ULL << 7);
                    bitboards[WROOK] |= (1ULL << 5);
                }

            }
            if (to_square == 2) {
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
        if (piece == WPAWN and (from_square - to_square) == -16) {
            en_passant_square = from_square + 8;
        }
        if (piece == BPAWN and (from_square - to_square) == 16) {
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
        print_board();
        //from_index = _bitscanforward(King);
        std::cout << "piece " << piece_type(piece) << " from "<< from_square << " to " << to_square << "\n";
        std::cout << "Not valid move"<< castling_rights << std::endl;
    }
};
inline void Board::unmake_move() {
    if (move_stack_index >= 0) {
        move_stack_index--;
        BoardState board;
        board = move_stack[move_stack_index];
        bitboards[WPAWN ] = board.wpawn;
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
        side_to_move ^= 1;
        update_occupancies();
    }
    else {
        std::cout << "no entry" << std::endl;
    }
    //No need to remove the entry because it will be overwritten 
}
void Board::print_bitboard(std::bitset<64> bitset) {
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

U64 Board::Pawns_NotLeft() {
    return ~File1;
}

U64 Board::Pawns_NotRight() {
    return ~File8;
}


U64 Board::Pawn_Forward(bool IsWhite, U64 mask) {
    if (IsWhite) return mask << 8;
    else return mask >> 8;
}


U64 Board::Pawn_Forward2(bool IsWhite, U64 mask) {
    if (IsWhite) return (Pawn_Forward(IsWhite, mask) & ~Occ) << 8;
    else return (Pawn_Forward(IsWhite, mask) & ~Occ) >> 8;
}


U64 Board::Pawn_Backward(bool IsWhite, U64 mask) {
    return Pawn_Forward(IsWhite, mask);
}


U64 Board::Pawn_Backward2(bool IsWhite, U64 mask) {
    return Pawn_Forward2(IsWhite, mask);
}


U64 Board::Pawn_AttackRight(bool IsWhite, U64 mask) {
    if (IsWhite) return mask << 9;
    else return mask >> 7;
}


U64 Board::Pawn_AttackLeft(bool IsWhite, U64 mask) {
    if (IsWhite) return mask << 7;
    else return mask >> 9;
}


U64 Board::Pawn_InvertLeft(bool IsWhite, U64 mask) {
    return Pawn_AttackRight(IsWhite, mask);
}


U64 Board::Pawn_InvertRight(bool IsWhite, U64 mask) {
    return Pawn_AttackLeft(IsWhite, mask);
}


U64 Board::Pawns_FirstRank(bool IsWhite) {
    if (IsWhite) return Rank2;
    else return Rank7;
}


U64 Board::Pawns_LastRank(bool IsWhite) {
    if (IsWhite) return Rank7;
    else return Rank2;
}


U64 Board::King(bool IsWhite) {
    if (IsWhite) return WKing;
    else return BKing;
}


U64 Board::EnemyKing(bool IsWhite)
{
    if (IsWhite) return BKing;
    else return WKing;
}

U64 Board::EnemyPawn(bool IsWhite) {
    if (IsWhite) return BPawn;
    return WPawn;
}

U64 Board::Pawns(bool IsWhite) {
    if (IsWhite) return WPawn;
    else return BPawn;
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
    if (IsWhite) return BRook | BQueen;
    return WRook | WQueen;
}


U64 Board::RookQueen(bool IsWhite)
{
    if (IsWhite) return WRook | WQueen;
    return BRook | BQueen;
}


U64 Board::EnemyBishopQueen(bool IsWhite)
{
    if (IsWhite) return bitboards[BBISHOP] | bitboards[BQUEEN];
    return bitboards[WBISHOP] | bitboards[WQUEEN];
}


U64 Board::BishopQueen(bool IsWhite)
{
    if (IsWhite) return WBishop | WQueen;
    return BBishop | BQueen;
}


U64 Board::KingPawn(bool IsWhite)
{
    if (IsWhite) return WPawn | WKing;
    return BPawn | BQueen;
}

U64 Board::EnemyKingPawn(bool IsWhite)
{
    if (IsWhite) return BPawn | BKing;
    return WPawn | WKing;
}

U64 Board::EnemyOrEmpty(bool IsWhite)
{
    if (IsWhite) return ~White;
    return ~Black;
}

U64 Board::Empty(bool IsWhite)
{
    return ~Occ;
}


U64 Board::Knights(bool IsWhite)
{
    if (IsWhite) return WKnight;
    return BKnight;
}


U64 Board::Rooks(bool IsWhite)
{
    if (IsWhite) return WRook;
    return BRook;
}


U64 Board::Bishops(bool IsWhite)
{
    if (IsWhite) return WBishop;
    return BBishop;
}


U64 Board::Queens(bool IsWhite)
{
    if (IsWhite) return WQueen;
    return BQueen;
}


U64 Board::do_checkmask(bool IsWhite, int sq) {
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

U64 Board::would_be_attack(bool IsWhite, int sq) {
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

U64 Board::is_pinned_hv(bool IsWhite, int sq) {
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


U64 Board::seen_by_pawn(bool IsWhite, int sq, int ep) {
    U64 ep_m = (1ULL << ep);
    U64 new_mask = 0ULL;
    U64 mask = (1ULL << sq);
    if (IsWhite) {
        new_mask = Pawn_Forward(IsWhite, mask) & ~Occ | Pawn_Forward2(IsWhite, mask) & ~Occ & ~rank_2_mask | Pawn_AttackLeft(IsWhite, mask) & ~White & Black & not_a_file | Pawn_AttackRight(IsWhite, mask) & ~White & Black & not_h_file;
    }
    if (!IsWhite) {
        new_mask = Pawn_Backward(IsWhite, mask) & ~Occ |  Pawn_Backward2(IsWhite, mask) & ~Occ & ~rank_7_mask | Pawn_AttackLeft(IsWhite,mask) & ~Black & White & not_a_file | Pawn_AttackRight(IsWhite, mask) & ~Black & White & not_h_file;
    }
    if (ep < 64 and square_distance(sq, ep) == 1) {
        if (not (mask & pin_hv)) {
            if (IsWhite) {
                bitboards[WPAWN] &= ~(1ULL << sq);
                bitboards[BPAWN] &= ~(1ULL << (ep - 8));
                White = bitboards[WPAWN] | bitboards[WKNIGHT] | bitboards[WBISHOP] | bitboards[WROOK] | bitboards[WQUEEN] | bitboards[WKING];
                Black = bitboards[BPAWN] | bitboards[BKNIGHT] | bitboards[BBISHOP] | bitboards[BROOK] | bitboards[BQUEEN] | bitboards[BKING];
                Occ = Black | White;
                int king_sq = _bitscanforward(King(IsWhite));
                if (not(do_checkmask(IsWhite, king_sq))) {
                    BPawn |= 1ULL << (ep - 8);
                    WPawn |= 1ULL << sq;
                    bitboards[WPAWN] |= 1ULL << sq;
                    bitboards[BPAWN] |= 1ULL << (ep - 8);
                    White = bitboards[WPAWN] | bitboards[WKNIGHT] | bitboards[WBISHOP] | bitboards[WROOK] | bitboards[WQUEEN] | bitboards[WKING];
                    Black = bitboards[BPAWN] | bitboards[BKNIGHT] | bitboards[BBISHOP] | bitboards[BROOK] | bitboards[BQUEEN] | bitboards[BKING];
                    Occ = Black | White;
                    return new_mask | ep_m;
                }
                BPawn |= 1ULL << (ep - 8);
                WPawn |= 1ULL << sq;
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
                if (not(do_checkmask(IsWhite, king_sq))) {
                    BPawn |= 1ULL << sq;
                    WPawn |= 1ULL << (ep + 8);
                    bitboards[BPAWN] |= 1ULL << sq;
                    bitboards[WPAWN] |= 1ULL << (ep + 8);
                    White = bitboards[WPAWN] | bitboards[WKNIGHT] | bitboards[WBISHOP] | bitboards[WROOK] | bitboards[WQUEEN] | bitboards[WKING];
                    Black = bitboards[BPAWN] | bitboards[BKNIGHT] | bitboards[BBISHOP] | bitboards[BROOK] | bitboards[BQUEEN] | bitboards[BKING];
                    Occ = Black | White;
                    return new_mask | ep_m;
                }
                BPawn |= 1ULL << sq;
                WPawn |= 1ULL << (ep + 8);
                bitboards[BPAWN] |= 1ULL << sq;
                bitboards[WPAWN] |= 1ULL << (ep + 8);
                White = bitboards[WPAWN] | bitboards[WKNIGHT] | bitboards[WBISHOP] | bitboards[WROOK] | bitboards[WQUEEN] | bitboards[WKING];
                Black = bitboards[BPAWN] | bitboards[BKNIGHT] | bitboards[BBISHOP] | bitboards[BROOK] | bitboards[BQUEEN] | bitboards[BKING];
                Occ = Black | White;
            }
        }
    }
    return new_mask;
}


inline U64 Board::seen_by_bishop(bool IsWhite, int sq) {
    U64 bishop_move = 0ULL;
    U64 victims;
    U64 blockers;
    //bishop_move |= (1ULL << sq);

    if (IsWhite) {
        victims = Black;
        blockers = White;
    }
    else {
        victims = White;
        blockers = Black;
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


inline U64 Board::seen_by_knight( int sq)
{
    return knightattacks[sq];
}


inline U64 Board::seen_by_rook(bool IsWhite, int sq) {
    U64 rook_move = 0ULL;
    U64 victims;
    U64 blockers;

    rook_move |= (1ULL << sq);

    if (IsWhite) {
        victims = Black;
        blockers = White;
    }
    else {
        victims = White;
        blockers = Black;
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


inline U64 Board::seen_by_queen(bool IsWhite, int sq) {
    U64 queen_move = 0ULL;
    queen_move |= (1ULL << sq);
    queen_move |= seen_by_bishop(IsWhite, sq);
    queen_move |= seen_by_rook(IsWhite, sq);
    queen_move &= ~(1ULL << sq);
    return queen_move;
}


inline U64 Board::seen_by_king( int sq) {
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


inline U64 Board::valid_pawn_moves(bool IsWhite, int sq, int ep) {
    U64 mask = 1ULL << sq;
    U64 pawn_attacks = 0ULL;
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
                BPawn &= ~(1ULL << (ep - 8));
                WPawn &= ~(1ULL << sq);
                bitboards[WPAWN] &= ~(1ULL << sq);
                bitboards[BPAWN] &= ~(1ULL << (ep - 8));
                White = bitboards[WPAWN] | bitboards[WKNIGHT] | bitboards[WBISHOP] | bitboards[WROOK] | bitboards[WQUEEN] | bitboards[WKING];
                Black = bitboards[BPAWN] | bitboards[BKNIGHT] | bitboards[BBISHOP] | bitboards[BROOK] | bitboards[BQUEEN] | bitboards[BKING];
                Occ = Black | White;
                int king_sq = _bitscanforward(King(IsWhite));
                if (not(do_checkmask(IsWhite, king_sq)) and square_rank(sq) == 4) {
                    if (square_file(ep) != 7 and square_file(sq) == 0 ) {
                        ep_take |= ep_m;
                    }
                    if (square_file(ep) != 0 and square_file(sq) == 7 ) {
                        ep_take |= ep_m;
                    }
                    if (square_file(ep) != 0 and square_file(sq) != 7 ) {
                        ep_take |= ep_m;
                    }
                    if (square_file(ep) == 0 and square_file(sq) == 1 ) {
                        ep_take |= ep_m;
                    }
                    if (square_file(ep) == 7 and square_file(sq) == 6 ) {
                        ep_take |= ep_m;
                    }
                }
                BPawn |= 1ULL << (ep - 8);
                WPawn |= 1ULL << sq;
                bitboards[WPAWN] |= 1ULL << sq;
                bitboards[BPAWN] |= 1ULL << (ep - 8);
                White = bitboards[WPAWN] | bitboards[WKNIGHT] | bitboards[WBISHOP] | bitboards[WROOK] | bitboards[WQUEEN] | bitboards[WKING];
                Black = bitboards[BPAWN] | bitboards[BKNIGHT] | bitboards[BBISHOP] | bitboards[BROOK] | bitboards[BQUEEN] | bitboards[BKING];
                Occ = Black | White;
            }
            else {
                BPawn &= ~(1ULL << sq);
                WPawn &= ~(1ULL << (ep + 8));
                bitboards[BPAWN] &= ~(1ULL << sq);
                bitboards[WPAWN] &= ~(1ULL << (ep + 8));
                White = bitboards[WPAWN] | bitboards[WKNIGHT] | bitboards[WBISHOP] | bitboards[WROOK] | bitboards[WQUEEN] | bitboards[WKING];
                Black = bitboards[BPAWN] | bitboards[BKNIGHT] | bitboards[BBISHOP] | bitboards[BROOK] | bitboards[BQUEEN] | bitboards[BKING];
                Occ = Black | White;
                int king_sq = _bitscanforward(King(IsWhite));
                if (not(do_checkmask(IsWhite, king_sq)) and square_rank(sq) == 3) {
                    if (square_file(ep) != 7 and square_file(sq) == 0 ) {
                        ep_take |= ep_m;
                    }
                    if (square_file(ep) != 0 and square_file(sq) == 7 ) {
                        ep_take |= ep_m;
                    }
                    if (square_file(ep) != 0 and square_file(sq) != 7 ) {
                        ep_take |= ep_m;
                    }
                    if (square_file(ep) == 0 and square_file(sq) == 1) {
                        ep_take |= ep_m;
                    }
                    if (square_file(ep) == 7 and square_file(sq) == 6) {
                        ep_take |= ep_m;
                    }
                }
                BPawn |= 1ULL << sq;
                WPawn |= 1ULL << (ep + 8);
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
    if (ep != 64 and IsWhite and square_distance(sq,ep) == 1 and square_rank(sq) == 4) {
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
    U64 not_pinned_r = (mask & not_h_file) << 7 & (~pin_hv | Enemy(IsWhite) >> 9);
    U64 not_pinned_l = (mask & not_a_file) >> 7 & (~pin_hv | Enemy(IsWhite) << 9);

    if (not(is_pinned_dg(IsWhite, sq))) {
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
        if (square_distance(king_sq, ep-8) == 1 and doublecheck == 1 and not is_pinned_hv(IsWhite, sq)) {
            return (ep_take);
        }
    }
    if (!IsWhite and ep != 64 and square_distance(king_sq, ep + 8) == 1 and square_distance(sq, ep + 8) == 1) {
        if (square_distance(king_sq, ep+8) == 1 and doublecheck == 1 and not is_pinned_hv(IsWhite, sq)) {
            return (ep_take);
        }
    }
    return (pawn_push | valid_attacks | ep_take) & checkmask;
}


inline U64 Board::valid_knight_moves_pin( int sq) {
    return 0ULL;
}

inline U64 Board::valid_knight_moves_unpin(bool IsWhite, int sq) {
    if (_test_bit(pin_dg, sq) or _test_bit(pin_hv, sq)) {
        return 0ULL;
    }
    return seen_by_knight(sq) & EnemyOrEmpty(IsWhite) & checkmask;
}

inline U64 Board::valid_bishop_moves_pin(bool IsWhite, int sq) {
    if (_test_bit(pin_dg, sq)) {
        
        return seen_by_bishop(IsWhite, sq) & EnemyOrEmpty(IsWhite) & checkmask & pin_dg;
    }
    if (_test_bit(pin_hv, sq)) {
        return 0ULL;
    }
    
    return seen_by_bishop(IsWhite, sq) & EnemyOrEmpty(IsWhite) & checkmask;
}

inline U64 Board::valid_bishop_moves_unpin(bool IsWhite, int sq) {
    return seen_by_bishop(IsWhite, sq) & EnemyOrEmpty(IsWhite) & checkmask;
}

inline U64 Board::valid_rook_moves_pin(bool IsWhite, int sq) {
    if (_test_bit(pin_hv, sq)) {
        return seen_by_rook(IsWhite, sq) & EnemyOrEmpty(IsWhite) & checkmask & pin_hv;
    }
    if (_test_bit(pin_dg, sq)) {
        return 0ULL;
    }
    return seen_by_rook(IsWhite, sq) & EnemyOrEmpty(IsWhite) & checkmask;
}

inline U64 Board::valid_rook_moves_unpin(bool IsWhite, int sq) {
    return seen_by_rook(IsWhite, sq) & EnemyOrEmpty(IsWhite) & checkmask;
}

inline U64 Board::valid_queen_moves_pin(bool IsWhite, int sq) {
    return valid_rook_moves_pin(IsWhite, sq) | valid_bishop_moves_pin(IsWhite, sq);
}

inline U64 Board::valid_queen_moves_unpin(bool IsWhite, int sq) {
    return valid_rook_moves_unpin(IsWhite, sq) | valid_bishop_moves_unpin(IsWhite, sq);
}

inline U64 Board::valid_king_moves(bool IsWhite, int sq) {
    U64 moves = seen_by_king(sq) & EnemyOrEmpty(IsWhite);
    U64 final_moves = 0ULL;
    int to_index;
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
    //King side White
    if (castling_rights & wk and (_rays[EAST][sq] & blockers) and IsWhite) {
        rook_index = _bitscanforward(blockers & _rays[EAST][sq]);
        if (rook_index == 7) {
            if (not is_square_attacked(IsWhite, 4) and not is_square_attacked(IsWhite, 5) and not is_square_attacked(IsWhite, 6)) {
                U64 to_index = 0ULL;
                to_index |= 1ULL << 6;
                final_moves |= to_index;
            }
        }
    }
    //Queen side White
    if (castling_rights & wq and (_rays[WEST][sq] & blockers) and IsWhite) {
        rook_index = _bitscanreverse(blockers & _rays[WEST][sq]);
        if (rook_index == 0) {
            if (not is_square_attacked(IsWhite, 4) and not is_square_attacked(IsWhite, 3) and not is_square_attacked(IsWhite, 2)) {
                U64 to_index = 0ULL;
                to_index |= 1ULL << 2;
                final_moves |= to_index;
            }
        }
    }
    //King side Black
    if (castling_rights & bk and (_rays[EAST][sq] & blockers) and not IsWhite) {
        rook_index = _bitscanforward(blockers & _rays[EAST][sq]);
        if (rook_index == 63) {
            if (not is_square_attacked(IsWhite, 62) and not is_square_attacked(IsWhite, 61) and not is_square_attacked(IsWhite, 60)) {
                U64 to_index = 0ULL;
                to_index |= 1ULL << 62;
                final_moves |= to_index;
            }
        }
    }
    //Queen side Black
    if (castling_rights & bq and (_rays[WEST][sq] & blockers) and not IsWhite) {
        rook_index = _bitscanreverse(blockers & _rays[WEST][sq]);
        if (rook_index == 56) {
            if (not is_square_attacked(IsWhite,  60) and not is_square_attacked(IsWhite, 59) and not is_square_attacked(IsWhite, 58)) {
                U64 to_index = 0ULL;
                to_index |= 1ULL << 58;
                final_moves |= to_index;
            }
        }
    }
    return final_moves;
}


inline U64 Board::valid_king_castle_ks(bool IsWhite, int sq) {
    U64 castle = 0ULL;
    if (IsWhite) {
        castle = 1ULL << 4 | 1ULL << 5 | 1ULL << 6;
    }
    else {
        castle = 1ULL << 60 | 1ULL << 61 | 1ULL << 62;
    }
    if (castle & attacked_squares) {
        return 0ULL;
    }
    return castle;
}


inline U64 Board::valid_king_castle_qs(bool IsWhite, int sq) {
    U64 castle = 0ULL;
    if (IsWhite) {
        castle = 1ULL << 4 | 1ULL << 3 | 1ULL << 2;
    }
    else {
        castle = 1ULL << 60 | 1ULL << 59 | 1ULL << 58;
    }
    if (castle & attacked_squares) {
        return 0ULL;
    }
    return castle;
}

inline bool Board::is_square_attacked(bool IsWhite, int sq) {
    U64 enemy;
    U64 us;
    if (IsWhite) {
        us = White;
        enemy = Black;
    }
    else {
        us = Black;
        enemy = White;
    }
    unsigned long index;

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


void Board::gen_attacked_squares(bool IsWhite) {
    U64 us = IsWhite ? occupancies[0] : occupancies[1];
    int index = -1;
    while (us) {
        index = _bitscanforward(us);
        U64 dg = is_pinned_dg(IsWhite, index);
        U64 hv = is_pinned_hv(IsWhite, index);
        pin_dg |= dg ? dg: 0ULL;
        pin_hv |= hv ? hv : 0ULL;
        us &= ~(1ULL << index);
    }
}

void Board::init(bool IsWhite) {
    int king_sq = _bitscanforward(King(IsWhite));
    U64 r = do_checkmask(IsWhite, king_sq);
    if (r) {
        checkmask = r;
    }
    else {
        checkmask = 18446744073709551615ULL;
    }
    pin_dg = 0ULL;
    pin_hv = 0ULL;
    gen_attacked_squares(IsWhite);
}

MoveList Board::generate_legal_moves() {
    Move move;
    MoveList possible_moves{};
    int from_index;
    U64 we;
    int enemy = side_to_move ^ 1;
    bool IsWhite = false;
    if (side_to_move == 0) {
        IsWhite = true;
        we = occupancies[0];
    }
    else {
        we = occupancies[1];
    }
    U64 pawn_mask = (BPawn | WPawn) & we;
    U64 knight_mask = (BKnight | WKnight) & we;
    U64 bishop_mask = (BBishop | WBishop) & we;
    U64 rook_mask = (BRook | WRook) & we;
    U64 queen_mask = (BQueen | WQueen) & we;
    U64 king_mask = (BKing | WKing) & we;
    int king_sq = _bitscanforward(king_mask);
    count = 0;
    init(IsWhite);
    U64 move_mask = 0ULL;
    if (doublecheck < 2) {
        while (pawn_mask) {
            from_index = _bitscanforward(pawn_mask);
            move_mask = valid_pawn_moves(IsWhite, from_index, en_passant_square);
            while (move_mask) {
                int to_index = _bitscanforward(move_mask);
                if (occupancies[enemy]) {
                    move.capture = piece_at(from_index);
                }
                else {
                    move.capture = -1;
                }
                move.piece = PAWN;
                move.from_square = from_index;
                move.to_square = to_index;
                move.promotion = -1;
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
                move_mask = _blsr_u64(move_mask);
            }
            pawn_mask = _blsr_u64(pawn_mask);
        }

        while (knight_mask) {
            from_index = _bitscanforward(knight_mask);
            if (valid_knight_moves_pin(from_index)) {
                move_mask = valid_knight_moves_pin(from_index);
            }
            else {
                move_mask = valid_knight_moves_unpin(IsWhite, from_index);
            }
            while (move_mask) {
                int to_index = _bitscanforward(move_mask);
                if (occupancies[enemy]) {
                    move.capture = piece_at(to_index, enemy);
                }
                else {
                    move.capture = -1;
                }
                move.piece = KNIGHT;
                move.from_square = from_index;
                move.to_square = to_index;
                move.promotion = -1;
                possible_moves.movelist[count] = move;
                count++;
                move_mask = _blsr_u64(move_mask);
            }
            knight_mask = _blsr_u64(knight_mask);
        }
        while (bishop_mask) {
            from_index = _bitscanforward(bishop_mask);
            move_mask = valid_bishop_moves_pin(IsWhite, from_index);
            while (move_mask) {
                int to_index = _bitscanforward(move_mask);
                if (occupancies[enemy]) {
                    move.capture = piece_at(to_index, enemy);
                }
                else {
                    move.capture = -1;
                }
                move.piece = BISHOP;
                move.from_square = from_index;
                move.to_square = to_index;
                move.promotion = -1;
                possible_moves.movelist[count] = move;
                count++;
                move_mask = _blsr_u64(move_mask);
            }
            bishop_mask = _blsr_u64(bishop_mask);
        }
        while (rook_mask) {
            from_index = _bitscanforward(rook_mask);
            move_mask = valid_rook_moves_pin(IsWhite, from_index);
            while (move_mask) {
                int to_index = _bitscanforward(move_mask);
                if (occupancies[enemy]) {
                    move.capture = piece_at(to_index, enemy);
                }
                else {
                    move.capture = -1;
                }
                move.piece = ROOK;
                move.from_square = from_index;
                move.to_square = to_index;
                move.promotion = -1;
                possible_moves.movelist[count] = move;
                count++;
                move_mask = _blsr_u64(move_mask);
            }
            rook_mask = _blsr_u64(rook_mask);
        }
        while (queen_mask) {
            from_index = _bitscanforward(queen_mask);
            move_mask = valid_queen_moves_pin(IsWhite, from_index);

            while (move_mask) {
                int to_index = _bitscanforward(move_mask);
                if (occupancies[enemy]) {
                    move.capture = piece_at(to_index, enemy);
                }
                else {
                    move.capture = -1;
                }
                move.piece = QUEEN;
                move.from_square = from_index;
                move.to_square = to_index;
                move.promotion = -1;
                possible_moves.movelist[count] = move;
                count++;
                move_mask = _blsr_u64(move_mask);
            }
            queen_mask = _blsr_u64(queen_mask);
        }
    }

    from_index = _bitscanforward(king_mask);
    move_mask = valid_king_moves(IsWhite, from_index);    
    while (move_mask) {
        int to_index = _bitscanforward(move_mask);
        if (occupancies[enemy]) {
            move.capture = piece_at(to_index, enemy);
        }
        else {
            move.capture = -1;
        }
        move.piece = KING;
        move.from_square = from_index;
        move.to_square = to_index;
        move.promotion = -1;
        possible_moves.movelist[count] = move;
        count++;
        move_mask = _blsr_u64(move_mask);
    }
    king_mask = _blsr_u64(king_mask);
    return possible_moves;
}


U64 Perft::speed_test_perft(int depth, int max) {
    U64 nodes = 0;
    bool IsWhite = this_board.side_to_move;
    MA myarray;
    Pertft_Info pf;
    if (depth == 0) {
        return 1;
    }
    else {
        MoveList n_moves = this_board.generate_legal_moves();
        int count = this_board.count;
        for (int i = 0; i < count; i++) { 
            Move move = n_moves.movelist[i];
            this_board.make_move(move);
            nodes += speed_test_perft(depth - 1, depth);
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
void Board::print_board() {
    for (int i = 63; i >= 0; i -= 8)
    {
        std::cout << " " << piece_type(piece_at(i - 7)) << " " << piece_type(piece_at(i - 6)) << " " << piece_type(piece_at(i - 5)) << " " << piece_type(piece_at(i - 4)) << " " << piece_type(piece_at(i - 3)) << " " << piece_type(piece_at(i - 2)) << " " << piece_type(piece_at(i - 1)) << " " << piece_type(piece_at(i)) << " " << std::endl;
    }
    std::cout << '\n' << std::endl;
}
int test() {
    std::string fen1 = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::string fen2 = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::string fen3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
    std::string fen4 = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    std::string fen5 = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
    std::string fen6 = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ";
    auto begin = std::chrono::high_resolution_clock::now();
    
    Board board;
    board.apply_fen(fen1);
    Perft perft(board);
    int count = 0;

    auto begin2 = std::chrono::high_resolution_clock::now();
    if (perft.speed_test_perft(6, 6) == 119060324) { // 4 == 197281     5 == 4865609    6 == 119060324	
        count++;
        std::cout << "\n" << "Correct" << "\n" << std::endl;
    }
    else {
        std::cout << "\n" << "False" << "\n" << std::endl;
    };
    auto end2 = std::chrono::high_resolution_clock::now();
    auto time_diff2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - begin2).count();
    std::cout << time_diff2 / 1000000000.0f << " seconds"<< "\n" << std::endl;

    board.apply_fen(fen2);
    perft.this_board = board;
    begin2 = std::chrono::high_resolution_clock::now();
    if (perft.speed_test_perft(5, 5) == 193690690) { // 4 == 4085603      3 == 97862       5 == 193690690
        count++;
        std::cout << "\n" << "Correct" << "\n" << std::endl;
    }
    else {
        std::cout << "\n" << "False" << "\n" << std::endl;
    };
    end2 = std::chrono::high_resolution_clock::now();
    time_diff2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - begin2).count();
    std::cout << time_diff2 / 1000000000.0f << " seconds" << "\n" << std::endl;

    board.apply_fen(fen3);
    perft.this_board = board;
    begin2 = std::chrono::high_resolution_clock::now();
    if (perft.speed_test_perft(7, 7) == 178633661) {    // 6 == 11030083        5 == 674624
        count++;
        std::cout << "\n" << "Correct" << "\n" << std::endl;
    }
    else {
        std::cout << "\n" << "False" << "\n" << std::endl;
    };
    end2 = std::chrono::high_resolution_clock::now();
    time_diff2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - begin2).count();
    std::cout << time_diff2 / 1000000000.0f << " seconds" << "\n" << std::endl;

    board.apply_fen(fen4);
    perft.this_board = board;
    begin2 = std::chrono::high_resolution_clock::now();
    if (perft.speed_test_perft(5, 5) == 15833292) {    // 5 == 15833292        4 == 422333
        count++;
        std::cout << "\n" << "Correct" << "\n" << std::endl;
    }
    else {
        std::cout << "\n" << "False" << "\n" << std::endl;
    };
    end2 = std::chrono::high_resolution_clock::now();
    time_diff2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - begin2).count();
    std::cout << time_diff2 / 1000000000.0f << " seconds" << "\n" << std::endl;

    board.apply_fen(fen5);
    perft.this_board = board;
    begin2 = std::chrono::high_resolution_clock::now();
    if (perft.speed_test_perft(5, 5) == 89941194) {   //5 == 89941194    4 ==2103487
        count++;
        std::cout << "\n" << "Correct" << "\n" << std::endl;
    }
    else {
        std::cout << "\n" << "False" << "\n" << std::endl;
    };
    end2 = std::chrono::high_resolution_clock::now();
    time_diff2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - begin2).count();
    std::cout << time_diff2 / 1000000000.0f << " seconds" << "\n" << std::endl;

    board.apply_fen(fen6);
    perft.this_board = board;
    begin2 = std::chrono::high_resolution_clock::now();
    if (perft.speed_test_perft(5, 5) == 164075551) {     // 4 == 3894594     3 == 89890
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
    std::cout << "\n" << count<<"/"<<"6"<< " Correct" << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    auto time_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
    std::cout << time_diff / 1000000000.0f << " seconds" << std::endl;
    return 0;
}
