#include <sstream>
#include <bitset>
#include <map>
#include <algorithm>
#include <chrono>

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
    std::vector<std::string> params = split_input(fen);
    std::string position = params[0];
    std::string move_right = params[1];
    std::string castling = params[2];
    std::string en_passant = params[3];
    std::string half_move_clock = "0";
    std::string full_move_counter = "1";
    if (params.size() > 4) {
        std::string half_move_clock = params[4];
        std::string full_move_counter = params[5];
    }

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
    size_t pos = 0;
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
    for (size_t i = 0; i < castling.size(); i++) {
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

    // Changing en passant from FEN into en passant square
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

    // half_move_clock
    half_moves = std::stoi(half_move_clock);
	
    // full_move_counter actually half moves
    full_moves = std::stoi(full_move_counter)*2;

    // Udates Bitboards
    update_occupancies();

    // Updates board pieces
    for (int sq = 0; sq < 64; sq++) {
        if (piece_type_at(sq) != -1) {
            board_pieces[sq] = piece_at(sq);
        }
        else {
            board_pieces[sq] = -1;
        }
    }
    // Updates board hash
    board_hash = generate_zobrist_hash();
}

std::string Board::get_fen() {
    int sq;
    char letter;
    std::map<int, char> piece_to_int =
    {
    { 0,'P'},
    { 1,'N'},
    { 2,'B'},
    { 3,'R'},
    { 4,'Q'},
    { 5,'K'},
    { 6,'p'},
    { 7,'n'},
    { 8,'b'},
    { 9,'r'},
    { 10,'q'},
    { 11,'k'},
    };
    std::string fen = "";
    for (int rank = 7; rank >= 0; rank--) {
        int free_space = 0;
        for (int file = 0; file < 8; file++) {
            sq = rank * 8 + file;
            int piece = piece_at_square(sq);
            if (piece != -1) {
                if (free_space) {
                    fen += std::to_string(free_space);
                    free_space = 0;
                }
                letter = piece_to_int[piece];
                fen += letter;
            }
            else {
                free_space++;
            }  
        }
        if (free_space != 0) {
            fen += std::to_string(free_space);
        }
        fen += rank > 0 ? "/" : "";
    }
    fen += side_to_move ? " b " : " w ";
    if (castling_rights & 1)
        fen += "K";
    if (castling_rights & 2)
        fen += "Q";
    if (castling_rights & 4)
        fen += "k";
    if (castling_rights & 8)
        fen += "q";
    if (castling_rights == 0)
        fen += " - ";
    if (en_passant_square == no_sq)
        fen += " - ";
    else
        fen += " "+ square_to_coordinates[en_passant_square] + " ";
	
    fen += std::to_string(half_moves);
    fen += " " + std::to_string(full_moves/2);
    return fen;
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
        std::cout << " " << piece_type(board_pieces[i - 7]) << " " << piece_type(board_pieces[i - 6]) << " " << piece_type(board_pieces[i - 5]) << " " << piece_type(board_pieces[i - 4]) << " " << piece_type(board_pieces[i - 3]) << " " << piece_type(board_pieces[i - 2]) << " " << piece_type(board_pieces[i - 1]) << " " << piece_type(board_pieces[i]) << " " << std::endl;
    }
    std::cout << '\n' << std::endl;
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
    if (repetition_table[hash] == 0) {
        repetition_table.erase(hash);
    }
};

// true if >= 2 repetitions
bool Board::is_threefold_rep() {
    if (repetition_table[board_hash] >= 2) {
        return true;
    }
    return false;
}

// true if >= 3 repetitions
bool Board::is_threefold_rep3() {
    if (repetition_table[board_hash] >= 3) {
        return true;
    }
    return false;
}

// detects checkmate
bool Board::is_checkmate(bool IsWhite) {
    int king_sq = _bitscanforward(King(IsWhite));
    init(IsWhite);
    if (is_square_attacked(IsWhite, king_sq)) {
        init(IsWhite);
        // only king moves are valid
        if (doublecheck == 2) {
            legal_king_moves(IsWhite, king_sq);
            if (not legal_king_moves(IsWhite, king_sq)) {
                return true;
            }
        }
        // test all moves
        if (doublecheck == 1) {
            MoveList moves = generate_legal_moves();
            if (moves.size == 0) {
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
    if (legal_king_moves(IsWhite, king_sq)) {
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
        from_index = pop_lsb(knight_mask);
        move_mask = legal_knight_moves(IsWhite, from_index);
        if (move_mask) {
            return false;
        }
    }
    //Highest probabilty that any of these can move
    while (pawn_mask) {
        from_index = pop_lsb(pawn_mask);
        move_mask = legal_pawn_moves(IsWhite, from_index, en_passant_square);
        if (move_mask) {
            return false;
        }
    }
    while (bishop_mask) {
        from_index = pop_lsb(bishop_mask);
        move_mask = legal_bishop_moves(IsWhite, from_index);
        if (move_mask) {
            return false;
        }
    }
    while (rook_mask) {
        from_index = pop_lsb(rook_mask);
        move_mask = legal_rook_moves(IsWhite, from_index);
        if (move_mask) {
            return false;
        }
    }
    while (queen_mask) {
        from_index = pop_lsb(queen_mask);
        move_mask = legal_queen_moves(IsWhite, from_index);
        if (move_mask) {
            return false;
        }
    }
    return true;
}

// returns true if the game is over otherwise false
bool Board::is_game_over() {
    MoveList moves = generate_legal_moves();
    int count = moves.size;
    if (!count) return true;
    if (half_moves >= 100) return true;
    if (is_threefold_rep3()) return true;
    return false;
}

U64 Board::generate_zobrist_hash() {
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
        pop_lsb(white);
    }
    while (black) {
        int sq = _bitscanforward(black);
        int piece = piece_type_at(sq);
        if (piece != -1) {
            piece = piece * 2;
            hash ^= RANDOM_ARRAY[64 * piece + sq];
        }
        pop_lsb(black);
    }

    U64 ep_hash = 0ULL;
    if (en_passant_square != 64) {
        U64 ep_square = 1ULL << en_passant_square;
        U64 ep_mask = (Pawn_AttackLeft(!IsWhite, ep_square) & not_h_file) | (Pawn_AttackRight(!IsWhite, ep_square) & not_a_file);
        U64 color_p = IsWhite ? White : Black;
        if (ep_mask & (bitboards[WPAWN] | bitboards[BPAWN]) & color_p) {
            ep_hash = RANDOM_ARRAY[772 + square_file(en_passant_square)];
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

inline void Board::save_board_state()
{
    BoardState board{};
    board.en_passant = en_passant_square;
    board.castle_rights = castling_rights;
    board.board_hash = board_hash;
    board.half_move = half_moves;
    move_stack.push(board);
}

inline void Board::update_occupancies() {
    White = bitboards[WPAWN] | bitboards[WKNIGHT] | bitboards[WBISHOP] | bitboards[WROOK] | bitboards[WQUEEN] | bitboards[WKING];
    Black = bitboards[BPAWN] | bitboards[BKNIGHT] | bitboards[BBISHOP] | bitboards[BROOK] | bitboards[BQUEEN] | bitboards[BKING];
    Occ = White | Black;
}

// returns color specific int for piece without complex computation
int8_t Board::piece_at_square(int8_t sq) {
    return board_pieces[sq];
}

// returns color specific int for piece
int8_t Board::piece_at(int8_t sq, int given) {
    bool white = false;
    if (given > -1) {
        white = given;
    }
    else {
        if (_test_bit(White, sq)) {
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

// returns int for piece
int8_t Board::piece_type_at(int8_t sq) {
    U64 pawns = bitboards[WPAWN] | bitboards[BPAWN];
    U64 knights = bitboards[WKNIGHT] | bitboards[BKNIGHT];
    U64 bishops = bitboards[WBISHOP] | bitboards[BBISHOP];
    U64 rooks = bitboards[WROOK] | bitboards[BROOK];
    U64 queens = bitboards[WQUEEN] | bitboards[BQUEEN];
    U64 kings = bitboards[WKING] | bitboards[BKING];

    if (_test_bit(pawns, sq)) {
        return 0;
    }
    if (_test_bit(knights, sq)) {
        return 1;
    }
    if (_test_bit(bishops, sq)) {
        return 2;
    }
    if (_test_bit(rooks, sq)) {
        return 3;
    }
    if (_test_bit(queens, sq)) {
        return 4;
    }
    if (_test_bit(kings, sq)) {
        return 5;
    }
    return -1;
}

int8_t Board::piece_to_piece_type(int8_t piece) {
	switch (piece) {
	case WPAWN:
		return 0;
	case WKNIGHT:
		return 1;
	case WBISHOP:
		return 2;
	case WROOK:
		return 3;
	case WQUEEN:
		return 4;
	case WKING:
		return 5;
	case BPAWN:
		return 0;
	case BKNIGHT:
		return 1;
	case BBISHOP:
		return 2;
	case BROOK:
		return 3;
	case BQUEEN:
		return 4;
	case BKING:
		return 5;
	default:
		return -1;
	}
}
// converts int to string
std::string Board::piece_type(int8_t piece) {
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

inline U64 Board::Pawn_AttackRight(bool IsWhite, U64 mask) {
    if (IsWhite) return mask << 9;
    else return mask >> 7;
}

inline U64 Board::Pawn_AttackLeft(bool IsWhite, U64 mask) {
    if (IsWhite) return mask << 7;
    else return mask >> 9;
}

bool Board::non_pawn_material(bool IsWhite) {
    if (IsWhite) return bitboards[WKNIGHT] | bitboards[WBISHOP] | bitboards[WROOK] | bitboards[WQUEEN];
    else return bitboards[BKNIGHT] | bitboards[BBISHOP] | bitboards[BROOK] | bitboards[BQUEEN];
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

// squares seen by a bishop
inline U64 Board::seen_by_bishop(bool IsWhite, int8_t sq) {
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
inline U64 Board::seen_by_knight(int8_t sq) {
    return knightattacks[sq];
}

// squares seen by a rook
inline U64 Board::seen_by_rook(bool IsWhite, int8_t sq) {
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
inline U64 Board::seen_by_king(int8_t sq) {
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
inline U64 Board::legal_pawn_moves(bool IsWhite, int8_t sq, uint8_t ep) {
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
        pawn_push = (Pawn_Forward(IsWhite, mask) & ~Occ) | (Pawn_Forward2(IsWhite, mask) & ~Occ & ~rank_2_4_mask);
    }
    else {
        attack_l = Pawn_AttackLeft(IsWhite, mask) & White & not_h_file;
        attack_r = Pawn_AttackRight(IsWhite, mask) & White & not_a_file;
        pawn_push = (Pawn_Backward(IsWhite, mask) & ~Occ) | (Pawn_Backward2(IsWhite, mask) & ~Occ & ~rank_7_5_mask);
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
inline U64 Board::legal_knight_moves(bool IsWhite, int8_t sq) {
    if (_test_bit(pin_dg, sq) or _test_bit(pin_hv, sq)) {
        return 0ULL;
    }
    return seen_by_knight(sq) & EnemyOrEmpty(IsWhite) & checkmask;
}

// legal bishop moves
inline U64 Board::legal_bishop_moves(bool IsWhite, int8_t sq) {
    if (_test_bit(pin_dg, sq)) {
        return seen_by_bishop(IsWhite, sq) & EnemyOrEmpty(IsWhite) & checkmask & pin_dg;
    }
    if (_test_bit(pin_hv, sq)) {
        return 0ULL;
    }

    return seen_by_bishop(IsWhite, sq) & EnemyOrEmpty(IsWhite) & checkmask;
}

// legal rook moves
inline U64 Board::legal_rook_moves(bool IsWhite, int8_t sq) {
    if (_test_bit(pin_hv, sq)) {
        return seen_by_rook(IsWhite, sq) & EnemyOrEmpty(IsWhite) & checkmask & pin_hv;
    }
    if (_test_bit(pin_dg, sq)) {
        return 0ULL;
    }
    return seen_by_rook(IsWhite, sq) & EnemyOrEmpty(IsWhite) & checkmask;
}

// legal queen moves
inline U64 Board::legal_queen_moves(bool IsWhite, int8_t sq) {
    return legal_rook_moves(IsWhite, sq) | legal_bishop_moves(IsWhite, sq);
}

// legal king moves
inline U64 Board::legal_king_moves(bool IsWhite, int8_t sq) {
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

// bool if square is attacked
inline bool Board::is_square_attacked(bool IsWhite, int8_t sq) {
    U64 mask = (1ULL << sq);
	if (Pawn_AttackLeft(IsWhite, mask) & Pawns(!IsWhite) & not_h_file || Pawn_AttackRight(IsWhite, mask) & Pawns(!IsWhite) & not_a_file) return true;
    if (knightattacks[sq] & Knights(!IsWhite)) return true;
    if (seen_by_king(sq) & King(!IsWhite)) return true;
	if (seen_by_bishop(IsWhite, sq) & (Bishops(!IsWhite) | Queens(!IsWhite))) return true;
    if (seen_by_rook(IsWhite, sq) & (Rooks(!IsWhite) | Queens(!IsWhite))) return true;
    return false;
}

// creates the pin mask
inline void Board::create_pin_masks(bool IsWhite) {
    int8_t sq = _bitscanforward(King(IsWhite));
    U64 victims = IsWhite ? Black : White;
    U64 blockers = IsWhite ? White : Black;
    int8_t index;
    pin_hv = 0ULL;
    pin_dg = 0ULL;
    U64 attacksN = 0ULL;
    if (_rays[NORTH][sq] & victims) {
        index = _bitscanforward(victims & _rays[NORTH][sq]);
        if (piece_type_at(index) == 3 || piece_type_at(index) == 4) {
            attacksN = _rays[NORTH][sq];
            attacksN &= ~_rays[NORTH][index];
        }
    }

    U64 attacksS = 0ULL;
    if (_rays[SOUTH][sq] & victims) {
        index = _bitscanreverse(victims & _rays[SOUTH][sq]);
        if (piece_type_at(index) == 3 || piece_type_at(index) == 4) {
            attacksS = _rays[SOUTH][sq];
            attacksS &= ~_rays[SOUTH][index];
        }
    }

    U64 attacksE = 0ULL;
    if (_rays[EAST][sq] & victims) {
        index = _bitscanforward(victims & _rays[EAST][sq]);
        if (piece_type_at(index) == 3 || piece_type_at(index) == 4) {
            attacksE = _rays[EAST][sq];
            attacksE &= ~_rays[EAST][index];
        }
    }

    U64 attacksW = 0ULL;
    if (_rays[WEST][sq] & victims) {
        index = _bitscanreverse(victims & _rays[WEST][sq]);
        if (piece_type_at(index) == 3 || piece_type_at(index) == 4) {
            attacksW = _rays[WEST][sq];
            attacksW &= ~_rays[WEST][index];
        }
    }
	
	if (popcount(attacksN & blockers) == 1) {
		pin_hv |= attacksN;
	}
    if (popcount(attacksS & blockers) == 1) {
        pin_hv |= attacksS;
    }
    if (popcount(attacksE & blockers) == 1) {
        pin_hv |= attacksE;
    }
    if (popcount(attacksW & blockers) == 1) {
        pin_hv |= attacksW;
    }
	
    U64 bishop_moveNW = 0ULL;
    if (_rays[NORTH_WEST][sq] & victims) {
        index = _bitscanforward(victims & _rays[NORTH_WEST][sq]);
        if (piece_type_at(index) == 2 || piece_type_at(index) == 4) {
            bishop_moveNW = _rays[NORTH_WEST][sq];
            bishop_moveNW &= ~_rays[NORTH_WEST][index];
        }
    }

    U64 bishop_moveNE = 0ULL;
    if (_rays[NORTH_EAST][sq] & victims) {
        index = _bitscanforward(victims & _rays[NORTH_EAST][sq]);
        if (piece_type_at(index) == 2 || piece_type_at(index) == 4) {
            bishop_moveNE = _rays[NORTH_EAST][sq];
            bishop_moveNE &= ~_rays[NORTH_EAST][index];
        }
    }

    U64 bishop_moveSW = 0ULL;
    if (_rays[SOUTH_WEST][sq] & victims) {
        index = _bitscanreverse(victims & _rays[SOUTH_WEST][sq]);
        if (piece_type_at(index) == 2 || piece_type_at(index) == 4) {
            bishop_moveSW = _rays[SOUTH_WEST][sq];
            bishop_moveSW &= ~_rays[SOUTH_WEST][index];
        }
    }

    U64 bishop_moveSE = 0ULL;
    if (_rays[SOUTH_EAST][sq] & victims) {
        index = _bitscanreverse(victims & _rays[SOUTH_EAST][sq]);
        if (piece_type_at(index) == 2 || piece_type_at(index) == 4) {
            bishop_moveSE = _rays[SOUTH_EAST][sq];
            bishop_moveSE &= ~_rays[SOUTH_EAST][index];
        }
    }
    if (popcount(bishop_moveNW & blockers) == 1) {
        pin_dg |= bishop_moveNW;
    }
    if (popcount(bishop_moveNE & blockers) == 1) {
        pin_dg |= bishop_moveNE;
    }
    if (popcount(bishop_moveSW & blockers) == 1) {
        pin_dg |= bishop_moveSW;
    }
    if (popcount(bishop_moveSE & blockers) == 1) {
        pin_dg |= bishop_moveSE;
    }
}

// creates the checkmask
U64 Board::create_checkmask(bool IsWhite, int8_t sq) {
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
U64 Board::would_be_attack(bool IsWhite, int8_t sq) {
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

bool Board::gives_check(Move& move) {
    make_move(move);
    bool inCheck = is_square_attacked(side_to_move, King(side_to_move));
    unmake_move(move);
	return inCheck;
}

// detects if the piece is pinned horizontal or vertical and returns the mask
U64 Board::is_pinned_hv(bool IsWhite, int8_t sq) {
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

// detects if the piece is pinned diagonal and returns the mask
U64 Board::is_pinned_dg(bool IsWhite, int8_t sq) {
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

// initialises variables for move generation
void Board::init(bool IsWhite) {
    int king_sq = _bitscanforward(King(IsWhite));
    U64 r = create_checkmask(IsWhite, king_sq);
    checkmask = r ? r : 18446744073709551615ULL;
    create_pin_masks(IsWhite);
}

void Board::make_move(Move& move) {

    int from_square = move.from_square;
    int to_square = move.to_square;
    int promotion_piece = move.promotion;
    int piece = move.piece;

    if (move.piece == -1) {
        piece = piece_at_square(from_square);
    }
    else {
        piece = piece + (side_to_move * 6);
    }

    int8_t captured_piece = -1;
    bool IsWhite = side_to_move ? 0 : 1;
    bool enemy = side_to_move ^ 1;

    // piece needs to be set at its bitboard remove this for performance if you are 100% theres a piece at that square
    if (not _test_bit(bitboards[piece], to_square)) {
        save_board_state();
        // Capture
        captured_piece = piece_at_square(to_square);
        if (captured_piece == WROOK) {
            if (to_square == 7) {
                if (castling_rights & wk) {
                    castling_rights ^= wk;
                    board_hash ^= RANDOM_ARRAY[768];
                }
            }
            if (to_square == 0) {
                if (castling_rights & wq) {
                    castling_rights ^= wq;
                    board_hash ^= RANDOM_ARRAY[768 + 1];
                }
            }
        }
        if (captured_piece == BROOK) {
            if (to_square == 63) {
                if (castling_rights & bk) {
                    castling_rights ^= bk;
                    board_hash ^= RANDOM_ARRAY[768 + 2];
                }
            }
            if (to_square == 56) {
                if (castling_rights & bq) {
                    castling_rights ^= bq;
                    board_hash ^= RANDOM_ARRAY[768 + 3];
                }
            }
        }
        // Halfmove
        if (piece == WPAWN or piece == BPAWN or captured_piece != -1) {
            half_moves = 0;
        }
        else {
            half_moves++;
        }
        // King move loses castle rights
        if (piece == WKING) {
            if (to_square != 6 and to_square != 2) {
                if (castling_rights & 1) {
                    castling_rights &= ~(1);
                    board_hash ^= RANDOM_ARRAY[768];
                }
                if (castling_rights & 2) {
                    castling_rights &= ~(2);
                    board_hash ^= RANDOM_ARRAY[768 + 1];
                }
            }
        }
        if (piece == BKING) {
            if (to_square != 62 and to_square != 58) {
                if (castling_rights & 4) {
                    castling_rights &= ~(4);
                    board_hash ^= RANDOM_ARRAY[768 + 2];
                }
                if (castling_rights & 8) {
                    castling_rights &= ~(8);
                    board_hash ^= RANDOM_ARRAY[768 + 3];
                }
            }

        }
        // Rook move loses castle rights
        if (piece == WROOK) {
            if (from_square == 7 and castling_rights & wk) {
                castling_rights ^= wk;
                board_hash ^= RANDOM_ARRAY[768];
            }
            if (from_square == 0 and castling_rights & wq) {
                castling_rights ^= wq;
                board_hash ^= RANDOM_ARRAY[768 + 1];
            }
        }
        if (piece == BROOK) {
            if (from_square == 63 and castling_rights & bk) {
                castling_rights ^= bk;
                board_hash ^= RANDOM_ARRAY[768 + 2];
            }
            if (from_square == 56 and castling_rights & bq) {
                castling_rights ^= bq;
                board_hash ^= RANDOM_ARRAY[768 + 3];
            }
        }

        // Actual castling
        if (piece == WKING and square_distance(from_square, to_square) == 2) {
            if (to_square == 6 and _test_bit(bitboards[WROOK], 7)) {
                if (castling_rights & wk) {
                    if (castling_rights & wq) {
                        board_hash ^= RANDOM_ARRAY[768 + 1];
                    }
                    castling_rights &= ~(1);
                    castling_rights &= ~(2);
                    bitboards[WROOK] &= ~(1ULL << 7);
                    bitboards[WROOK] |= (1ULL << 5);
                    board_pieces[7] = -1;
                    board_pieces[5] = WROOK;
                    board_hash ^= RANDOM_ARRAY[768];
                    board_hash ^= RANDOM_ARRAY[64 * (2 * 3 + 1) + 7];
                    board_hash ^= RANDOM_ARRAY[64 * (2 * 3 + 1) + 5];
                }
            }
            if (to_square == 2 and _test_bit(bitboards[WROOK], 0)) {
                if (castling_rights & wq) {
                    if (castling_rights & wk) {
                        board_hash ^= RANDOM_ARRAY[768];
                    }
                    castling_rights &= ~(1);
                    castling_rights &= ~(2);
                    bitboards[WROOK] &= ~(1ULL << 0);
                    bitboards[WROOK] |= (1ULL << 3);
                    board_pieces[0] = -1;
                    board_pieces[3] = WROOK;
                    board_hash ^= RANDOM_ARRAY[768 + 1];

                    board_hash ^= RANDOM_ARRAY[64 * (2 * 3 + 1) + 0];
                    board_hash ^= RANDOM_ARRAY[64 * (2 * 3 + 1) + 3];
                }
            }
        }
        if (piece == BKING and square_distance(from_square, to_square) == 2) {
            if (to_square == 62 and _test_bit(bitboards[BROOK], 63)) {
                if (castling_rights & bk) {
                    if (castling_rights & bq) {
                        board_hash ^= RANDOM_ARRAY[768 + 3];
                    }
                    castling_rights &= ~(4);
                    castling_rights &= ~(8);
                    bitboards[BROOK] &= ~(1ULL << 63);
                    bitboards[BROOK] |= (1ULL << 61);
                    board_pieces[63] = -1;
                    board_pieces[61] = BROOK;
                    board_hash ^= RANDOM_ARRAY[768 + 2];
                    board_hash ^= RANDOM_ARRAY[64 * (2 * 3) + 63];
                    board_hash ^= RANDOM_ARRAY[64 * (2 * 3) + 61];
                }
            }
            if (to_square == 58 and _test_bit(bitboards[BROOK], 56)) {
                if (castling_rights & bq) {
                    if (castling_rights & bk) {
                        board_hash ^= RANDOM_ARRAY[768 + 2];
                    }
                    castling_rights &= ~(4);
                    castling_rights &= ~(8);
                    bitboards[BROOK] &= ~(1ULL << 56);
                    bitboards[BROOK] |= (1ULL << 59);
                    board_pieces[56] = -1;
                    board_pieces[59] = BROOK;
                    board_hash ^= RANDOM_ARRAY[768 + 3];
                    board_hash ^= RANDOM_ARRAY[64 * (2 * 3) + 56];
                    board_hash ^= RANDOM_ARRAY[64 * (2 * 3) + 59];
                }
            }
        }

        // Remove enemy piece if en passant capture
        if ((abs(from_square - to_square) == 7 or abs(from_square - to_square) == 9)) {
            if (piece == WPAWN and square_rank(to_square) == 5 and piece_at_square(to_square - 8) == BPAWN and piece_at_square(to_square) == -1) { //
                en_passant_square = no_sq;
                bitboards[BPAWN] &= ~(1ULL << (to_square - 8));
                board_pieces[to_square - 8] = -1;
                board_hash ^= RANDOM_ARRAY[64 * (2 * 0) + to_square - 8];
                board_hash ^= RANDOM_ARRAY[772 + square_file(to_square)];

            }
            if (piece == BPAWN and square_rank(to_square) == 2 and piece_at_square(to_square + 8) == WPAWN and piece_at_square(to_square) == -1) {
                en_passant_square = no_sq;
                bitboards[WPAWN] &= ~(1ULL << (to_square + 8));
                board_pieces[to_square + 8] = -1;
                board_hash ^= RANDOM_ARRAY[64 * (2 * 0 + 1) + to_square + 8];
                board_hash ^= RANDOM_ARRAY[772 + square_file(to_square)];
            }
        }
        // remove en passant if it wasnt played immediately
        if (en_passant_square != no_sq) {
            board_hash ^= RANDOM_ARRAY[772 + square_file(en_passant_square)];
            en_passant_square = no_sq;
        }

        // set en passant square if pawns double move
        if (piece == WPAWN and (from_square - to_square) == -16) {
            U64 ep_square = 1ULL << (from_square + 8);
            U64 ep_mask = ((Pawn_AttackLeft(IsWhite, ep_square)) & not_h_file) | ((Pawn_AttackRight(IsWhite, ep_square) & not_a_file));
            if (ep_mask & bitboards[BPAWN]) {
                en_passant_square = from_square + 8;
                board_hash ^= RANDOM_ARRAY[772 + square_file(en_passant_square)];
            }
        }
        if (piece == BPAWN and (from_square - to_square) == 16) {
            U64 ep_square = 1ULL << (from_square - 8);
            U64 ep_mask = ((Pawn_AttackLeft(IsWhite, ep_square)) & not_h_file) | ((Pawn_AttackRight(IsWhite, ep_square) & not_a_file));
            if (ep_mask & bitboards[WPAWN]) {
                en_passant_square = from_square - 8;
                board_hash ^= RANDOM_ARRAY[772 + square_file(en_passant_square)];
            }
        }

        // Remove piece
        bitboards[piece] &= ~(1ULL << from_square);
        // Set piece
        bitboards[piece] |= (1ULL << to_square);
        // Remove
        board_pieces[from_square] = -1;
        // Set
        board_pieces[to_square] = piece;
        // Remove
        board_hash ^= RANDOM_ARRAY[64 * (2 * move.piece + IsWhite) + from_square];
        // Set
        board_hash ^= RANDOM_ARRAY[64 * (2 * move.piece + IsWhite) + to_square];
        // Remove captured piece
        if (captured_piece != -1) {
            bitboards[captured_piece] &= ~(1ULL << to_square);
            board_hash ^= RANDOM_ARRAY[64 * (2 * (captured_piece - (6 * enemy)) + !IsWhite) + to_square];
        }

        // Promotion
        if (promotion_piece > 0 and promotion_piece < 7) {
            board_hash ^= RANDOM_ARRAY[64 * (2 * promotion_piece + IsWhite) + to_square];
            board_hash ^= RANDOM_ARRAY[64 * (2 * move.piece + IsWhite) + to_square];

            promotion_piece = promotion_piece + (side_to_move * 6);
            bitboards[piece] &= ~(1ULL << to_square);
            bitboards[promotion_piece] |= (1ULL << to_square);
            board_pieces[to_square] = promotion_piece;
        }

        // Swap color
        side_to_move ^= 1;
        board_hash ^= RANDOM_ARRAY[780];
        //Update
        update_occupancies();
        add_repetition(board_hash);
        full_moves++;
    }
    else {
        // Not valid move
        print_board();
        std::cout << "piece " << piece_type(piece) << " from " << from_square << " to " << to_square << "\n";
        std::cout << "Not valid move" << castling_rights << std::endl;
    }
};

void Board::unmake_move(Move& move) {
    if (move_stack.size() > 0) {
        BoardState board;
        board = move_stack.top();
		
        en_passant_square = board.en_passant;
        castling_rights = board.castle_rights;
        remove_repetition(board_hash);
        board_hash = board.board_hash;
        half_moves = board.half_move;

        
        
        side_to_move ^= 1;		
        int piece = move.piece + (side_to_move * 6);
		
        bitboards[piece] |= (1ULL << move.from_square);
        bitboards[piece] &= ~(1ULL << move.to_square);
        board_pieces[move.from_square] = piece;
        board_pieces[move.to_square] = -1;

        if (move.to_square == en_passant_square && piece == 0) {
			bitboards[6] |= (1ULL << (move.to_square - 8));
            board_pieces[move.to_square - 8] = 6;
        }
        if (move.to_square == en_passant_square && piece == 6) {
            bitboards[0] |= (1ULL << (move.to_square + 8));
            board_pieces[move.to_square + 8] = 0;
        }
		
        if (move.capture != -1) {
            bitboards[move.capture] |= (1ULL << move.to_square);
            board_pieces[move.to_square] = move.capture;
        }

        if (move.promotion != -1) {
            bitboards[move.promotion + (6 * side_to_move)] &= ~(1ULL << move.to_square);
        }
		
        if (piece == WKING) {
            if (move.to_square == 6 && move.from_square == 4) {
                bitboards[WROOK] &= ~(1ULL << 5);
                bitboards[WROOK] |= (1ULL << 7);
                board_pieces[7] = WROOK;
                board_pieces[5] = -1;
            }
            if (move.to_square == 2 && move.from_square == 4) {
                bitboards[WROOK] &= ~(1ULL << 3);
                bitboards[WROOK] |= (1ULL << 0);
                board_pieces[0] = WROOK;
                board_pieces[3] = -1; 
            }
        }
        if (piece == BKING) {
            if (move.to_square == 62 && move.from_square == 60) {
                bitboards[BROOK] |= (1ULL << 63);
                bitboards[BROOK] &= ~(1ULL << 61);
                board_pieces[63] = BROOK;
                board_pieces[61] = -1;
            }
            if (move.to_square == 58 && move.from_square == 60) {
                bitboards[BROOK] |= (1ULL << 56);
                bitboards[BROOK] &= ~(1ULL << 59);
                board_pieces[56] = BROOK;
                board_pieces[59] = -1;
                
            }
        }

        update_occupancies();
        move_stack.pop();
        full_moves--;

    }
    else {
        //Tried to unmake a move although theres no previous move
        std::cout << "no entry" << std::endl;
    }
}

void Board::make_null_move() {
    save_board_state();
    side_to_move ^= 1;
    board_hash ^= RANDOM_ARRAY[780];
    en_passant_square = 64;
    full_moves++;
}

void Board::unmake_null_move() {
    if (move_stack.size() > 0) {
        BoardState board;
        board = move_stack.top();
        side_to_move ^= 1;
        board_hash ^= RANDOM_ARRAY[780];
        en_passant_square = board.en_passant;
        full_moves--;
        move_stack.pop();
    }
}

MoveList Board::generate_legal_moves() {
    Move move;
    MoveList possible_moves{};
    possible_moves.size = 0;
    int from_index = 0;
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
            move_mask = legal_pawn_moves(IsWhite, from_index, en_passant_square);
            while (move_mask) {
                int to_index = pop_lsb(move_mask);
                move.piece = PAWN;
                move.from_square = from_index;
                move.to_square = to_index;
                move.promotion = -1;
				move.capture = piece_at_square(to_index);
				
                if (square_rank(to_index) == 7 or square_rank(to_index) == 0) {
                    move.promotion = QUEEN;
                    possible_moves.movelist[possible_moves.size] = move;
                    possible_moves.size++;
                    move.promotion = ROOK;
                    possible_moves.movelist[possible_moves.size] = move;
                    possible_moves.size++;
                    move.promotion = KNIGHT;
                    possible_moves.movelist[possible_moves.size] = move;
                    possible_moves.size++;
                    move.promotion = BISHOP;
                    possible_moves.movelist[possible_moves.size] = move;
                    possible_moves.size++;
                }
                else {
                    move.promotion = -1;
                    possible_moves.movelist[possible_moves.size] = move;
                    possible_moves.size++;
                }
            }
            pop_lsb(pawn_mask);
        }
        while (knight_mask) {
            from_index = _bitscanforward(knight_mask);
            move_mask = legal_knight_moves(IsWhite, from_index);
            while (move_mask) {
                int to_index = pop_lsb(move_mask);
                move.piece = KNIGHT;
                move.from_square = from_index;
                move.to_square = to_index;
                move.promotion = -1;
                move.capture = piece_at_square(to_index);
                possible_moves.movelist[possible_moves.size] = move;
                possible_moves.size++;
            }
            pop_lsb(knight_mask);
        }
        while (bishop_mask) {
            from_index = _bitscanforward(bishop_mask);
            move_mask = legal_bishop_moves(IsWhite, from_index);
            while (move_mask) {
                int to_index = pop_lsb(move_mask);
                move.piece = BISHOP;
                move.from_square = from_index;
                move.to_square = to_index;
                move.promotion = -1;
                move.capture = piece_at_square(to_index);
                possible_moves.movelist[possible_moves.size] = move;
                possible_moves.size++;
            }
            pop_lsb(bishop_mask);
        }
        while (rook_mask) {
            from_index = _bitscanforward(rook_mask);
            move_mask = legal_rook_moves(IsWhite, from_index);
            while (move_mask) {
                int to_index = pop_lsb(move_mask);
                move.piece = ROOK;
                move.from_square = from_index;
                move.to_square = to_index;
                move.promotion = -1;
                move.capture = piece_at_square(to_index);
                possible_moves.movelist[possible_moves.size] = move;
                possible_moves.size++;
            }
            pop_lsb(rook_mask);
        }
        while (queen_mask) {
            from_index = _bitscanforward(queen_mask);
            move_mask = legal_queen_moves(IsWhite, from_index);
            while (move_mask) {
                int to_index = pop_lsb(move_mask);
                move.piece = QUEEN;
                move.from_square = from_index;
                move.to_square = to_index;
                move.promotion = -1;
                move.capture = piece_at_square(to_index);
                possible_moves.movelist[possible_moves.size] = move;
                possible_moves.size++;
            }
            pop_lsb(queen_mask);
        }
    }

    move_mask = legal_king_moves(IsWhite, king_sq);
    while (move_mask) {
        int to_index = pop_lsb(move_mask);
        move.piece = KING;
        move.from_square = king_sq;
        move.to_square = to_index;
        move.promotion = -1;
        move.capture = piece_at_square(to_index);
        possible_moves.movelist[possible_moves.size] = move;
        possible_moves.size++;  
    }
    
    return possible_moves;
}

MoveList Board::generate_capture_moves() {
    Move move;
    MoveList possible_moves{};
    possible_moves.size = 0;

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
    bool inCheck = checkmask == 18446744073709551615ULL ? false : true;
    U64 move_mask = 0ULL;

    if (doublecheck < 2) {
        while (pawn_mask) {
            int from_index = _bitscanforward(pawn_mask);
            U64 all_moves = legal_pawn_moves(IsWhite, from_index, en_passant_square);
            move_mask = inCheck ? all_moves : all_moves & (Enemy(IsWhite) | rank_8_mask);
            while (move_mask) {
                int to_index = pop_lsb(move_mask);
                move.piece = PAWN;
                move.from_square = from_index;
                move.to_square = to_index;
                move.capture = piece_at_square(to_index);
                move.promotion = -1;
				
                if (to_index == en_passant_square) {
                    continue;
                }
                
                if (square_rank(to_index) == 7 or square_rank(to_index) == 0) {
                    move.promotion = QUEEN;
                    possible_moves.movelist[possible_moves.size] = move;
                    possible_moves.size++;
                    move.promotion = ROOK;
                    possible_moves.movelist[possible_moves.size] = move;
                    possible_moves.size++;
                    move.promotion = KNIGHT;
                    possible_moves.movelist[possible_moves.size] = move;
                    possible_moves.size++;
                    move.promotion = BISHOP;
                    possible_moves.movelist[possible_moves.size] = move;
                    possible_moves.size++;
                }
                else {
                    possible_moves.movelist[possible_moves.size] = move;
                    possible_moves.size++;
                }
            }
            pop_lsb(pawn_mask);
        }
        while (knight_mask) {
            int from_index = _bitscanforward(knight_mask);
            U64 all_moves = legal_knight_moves(IsWhite, from_index);
            move_mask = inCheck ? all_moves : all_moves & Enemy(IsWhite);
            while (move_mask) {
                int to_index = pop_lsb(move_mask);
                move.piece = KNIGHT;
                move.from_square = from_index;
                move.to_square = to_index;
                move.promotion = -1;
                move.capture = piece_at_square(to_index);
                possible_moves.movelist[possible_moves.size] = move;
                possible_moves.size++;
            }
            pop_lsb(knight_mask);
        }
        while (bishop_mask) {
            int from_index = _bitscanforward(bishop_mask);
            U64 all_moves = legal_bishop_moves(IsWhite, from_index);
            move_mask = inCheck ? all_moves : all_moves & Enemy(IsWhite);
            while (move_mask) {
                int to_index = pop_lsb(move_mask);
                move.piece = BISHOP;
                move.from_square = from_index;
                move.to_square = to_index;
                move.promotion = -1;
                move.capture = piece_at_square(to_index);
                possible_moves.movelist[possible_moves.size] = move;
                possible_moves.size++;
            }
            pop_lsb(bishop_mask);
        }
        while (rook_mask) {
            int from_index = _bitscanforward(rook_mask);
            U64 all_moves = legal_rook_moves(IsWhite, from_index);
            move_mask = inCheck ? all_moves : all_moves & Enemy(IsWhite);
            while (move_mask) {
                int to_index = pop_lsb(move_mask);
                move.piece = ROOK;
                move.from_square = from_index;
                move.to_square = to_index;
                move.promotion = -1;
                move.capture = piece_at_square(to_index);
                possible_moves.movelist[possible_moves.size] = move;
                possible_moves.size++;
            }
            pop_lsb(rook_mask);
        }
        while (queen_mask) {
            int from_index = _bitscanforward(queen_mask);
            U64 all_moves = legal_queen_moves(IsWhite, from_index);
            move_mask = inCheck ? all_moves : all_moves & Enemy(IsWhite);
            while (move_mask) {
                int to_index = pop_lsb(move_mask);
                move.piece = QUEEN;
                move.from_square = from_index;
                move.to_square = to_index;
                move.promotion = -1;
                move.capture = piece_at_square(to_index);
                possible_moves.movelist[possible_moves.size] = move;
                possible_moves.size++;
            }
            pop_lsb(queen_mask);
        }
    }

    U64 all_moves = legal_king_moves(IsWhite, king_sq);
    move_mask = inCheck ? all_moves : all_moves & Enemy(IsWhite);
    while (move_mask) {
        int to_index = pop_lsb(move_mask);
        move.piece = KING;
        move.from_square = king_sq;
        move.to_square = to_index;
        move.promotion = -1;
        move.capture = piece_at_square(to_index);
        possible_moves.movelist[possible_moves.size] = move;
        possible_moves.size++;
    }
    return possible_moves;
}

U64 Perft::speed_test_perft(int depth, int max) {
    U64 nodes = 0;
    MA info_array;
    Pertft_Info pf;
    if (depth == 0) {
        return 1;
    }
    else {
        MoveList n_moves = this_board->generate_legal_moves();
        int count = n_moves.size;
        for (int i = 0; i < count; i++) {
            Move move = n_moves.movelist[i];
            this_board->make_move(move);
            nodes += speed_test_perft(depth - 1, depth);
            this_board->unmake_move(move);
            if (depth == max) {
                pf.from_square = move.from_square;
                pf.to_square = move.to_square;
                pf.promotion_piece = move.promotion;
                pf.nodes = nodes;
                info_array.info_array.push_back(pf);
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
    for (size_t i = 0; i < info_array.info_array.size(); i++)
    {
        std::cout << square_to_coordinates[info_array.info_array[i].from_square];
        std::cout << square_to_coordinates[info_array.info_array[i].to_square];
        std::cout << " ";

        std::cout << info_array.info_array[i].nodes;
        std::cout << std::endl;
        c += info_array.info_array[i].nodes;

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
    MA info_array;
    Pertft_Info pf;
    MoveList n_moves = this_board->generate_legal_moves();
    int count = n_moves.size;
    if (depth == 1) {
        return count;
    }
    else {
        for (int i = 0; i < count; i++) {
            Move move = n_moves.movelist[i];
            this_board->make_move(move);
            nodes += bulk_test_perft(depth - 1, depth);
            this_board->unmake_move(move);
            if (depth == max) {
                pf.from_square = move.from_square;
                pf.to_square = move.to_square;
                pf.promotion_piece = move.promotion;
                pf.nodes = nodes;
                info_array.info_array.push_back(pf);
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
    for (size_t i = 0; i < info_array.info_array.size(); i++)
    {
        std::cout << square_to_coordinates[info_array.info_array[i].from_square];
        std::cout << square_to_coordinates[info_array.info_array[i].to_square];
        std::cout << " ";

        std::cout << info_array.info_array[i].nodes;
        std::cout << std::endl;
        c += info_array.info_array[i].nodes;

    }
    if (depth != max) {
        return nodes;
    }
    else {
        return c;
    }
};

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