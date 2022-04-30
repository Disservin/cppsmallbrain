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
        if (piece_at(sq) != -1) {
            board_pieces[sq] = piece_at(sq);
        }
        else {
            board_pieces[sq] = -1;
        }
    }
    // Updates board hash
    board_hash = generate_zobrist_hash();

    initializeLookupTables();
}

void Board::initializeLookupTables() {
    //initialize squares between table
    U64 sqs;
    for (int8_t sq1 = 0; sq1 <= 63; ++sq1) {
        for (int8_t sq2 = 0; sq2 <= 63; ++sq2) {
            sqs = (1ULL << sq1) | (1ULL << sq2);
            if (square_file(sq1) == square_file(sq2) || square_rank(sq1) == square_rank(sq2))
                SQUARES_BETWEEN_BB[sq1][sq2] = GetRookAttacks(sq1, sqs) & GetRookAttacks(sq2, sqs);
            else if (diagonal_of(sq1) == diagonal_of(sq2) || anti_diagonal_of(sq1) == anti_diagonal_of(sq2))
                SQUARES_BETWEEN_BB[sq1][sq2] = GetBishopAttacks(sq1, sqs) & GetBishopAttacks(sq2, sqs);
        }
    }
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

// true if >= 2 repetitions
bool Board::is_threefold_rep() {
    int8_t count = 0;
    for (int i = !side_to_move; i < full_moves && i + 2 < 1024; i += 2) {
        if (gameHistory[i] == board_hash) {
			count++;
        }
		if (count >= 2) return true;	
    }
    return false;
}

// true if >= 3 repetitions
bool Board::is_threefold_rep3() {
    int8_t count = 0;
    for (int i = !side_to_move; i < full_moves && i + 2 < 1024 ; i += 2) {
        if (gameHistory[i] == board_hash) {
            count++;
        }
        if (count >= 3) return true;
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
        U64 color_p = IsWhite ? White : Black;
        if (GetPawnAttacks(!IsWhite, en_passant_square) & (bitboards[WPAWN] | bitboards[BPAWN]) & color_p) {
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
    move_stack[move_stack_index] = board;
    move_stack_index++;
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
    return piece_to_piece_type(piece_at_square(sq));
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

void Board::removePiece(int8_t piece, int8_t sq) {
	bitboards[piece] &= ~(1ULL << sq);
	board_pieces[sq] = -1;
}
void Board::placePiece(int8_t piece, int8_t sq) {
	bitboards[piece] |= (1ULL << sq);
	board_pieces[sq] = piece;
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

inline U64 Board::PawnPush(bool IsWhite, int8_t sq) {
	if (IsWhite) return (U64)1 << (sq + 8);
	return (U64)1 << (sq - 8);
}

// Hyperbola Quintessence algorithm (for sliding pieces)
U64 Board::hyp_quint(int8_t square, U64 occ, U64 mask) {
    return (((mask & occ) - (1ULL << square) * 2) ^
        reverse(reverse(mask & occ) - reverse((1ULL << square)) * 2)) & mask;
}

U64 Board::GetPawnAttacks(bool IsWhite, int8_t sq) {
    if (IsWhite) return GetPawnAttacksS_TABLE[0][sq];
    return GetPawnAttacksS_TABLE[1][sq];
}

// get bishop attacks using Hyperbola Quintessence
U64 Board::GetBishopAttacks(int8_t square, U64 occ) {
    return hyp_quint(square, occ, MASK_DIAGONAL[diagonal_of(square)]) |
        hyp_quint(square, occ, MASK_ANTI_DIAGONAL[anti_diagonal_of(square)]);
}

// get rook attacks using Hyperbola Quintessence
U64 Board::GetRookAttacks(int8_t square, U64 occ) {
    return hyp_quint(square, occ, MASK_FILE[square_file(square)]) |
            hyp_quint(square, occ, MASK_RANK[square_rank(square)]);
}

// get queen attacks using Hyperbola Quintessence
U64 Board::GetQueenAttacks(int8_t square, U64 occ) {
    return GetBishopAttacks(square, occ) | GetRookAttacks(square, occ);
}

// squares seen by a knight
inline U64 Board::GetKnightAttacks(int8_t sq) {
    return knightattacks[sq];
}

// squares seen by a king
inline U64 Board::GetKingAttacks(int8_t sq) {
    return KING_ATTACKS_TABLE[sq];
}

// legal pawn moves
inline U64 Board::legal_pawn_moves(bool IsWhite, int8_t sq, uint8_t ep) {
    U64 enemy = (IsWhite) ? Black : White;
    // If we are pinned diagonally we can only do captures which are on the pin_dg and on the checkmask
    if (pin_dg & (1ULL << sq)) return GetPawnAttacks(IsWhite, sq) & pin_dg & checkmask & enemy;
    // Calculate pawn pushs
    U64 push = PawnPush(IsWhite, sq) & ~Occ;
    push |= (IsWhite) ? square_rank(sq) == 1 ? (push << 8) & ~Occ : 0ULL : square_rank(sq) == 6 ? (push >> 8) & ~Occ : 0ULL;
    // If we are pinned horizontally we can do no moves but if we are pinned vertically we can only do pawn pushs
    if (pin_hv & (1ULL << sq)) return push & pin_hv & checkmask;
    int8_t offset = (IsWhite) ? -8 : 8;
    U64 attacks = GetPawnAttacks(IsWhite, sq);
    // If we are in check and  the en passant square lies on our attackmask and the en passant piece gives check
    // return the ep mask as a move square
    if (checkmask != 18446744073709551615ULL && attacks & (1ULL << ep) && checkmask & (1ULL << (ep + offset))) return (attacks & (1ULL << ep));
    // If we are in check we can do all moves that are on the checkmask
    if (checkmask != 18446744073709551615ULL) return ((attacks & enemy) | push) & checkmask;

    U64 moves = ((attacks & enemy) | push) & checkmask;
    // We need to make extra sure that ep moves dont leave the king in check
    // 7k/8/8/K1Pp3r/8/8/8/8 w - d6 0 1 
    // Horizontal rook pins our pawn through another pawn, our pawn can push but not take enpassant 
    // remove both the pawn that made the push and our pawn that could take in theory and check if that would give check
    if (ep != no_sq && square_distance(sq, ep) == 1 && (1ULL << ep) & attacks) {
        int8_t ourPawn = IsWhite ? WPAWN : BPAWN;
		int8_t theirPawn = IsWhite ? BPAWN : WPAWN;
        int8_t KingSQ = _bitscanforward(King(IsWhite));
        removePiece(ourPawn, sq);
        removePiece(theirPawn, ep + offset);
        placePiece(ourPawn, ep);
        White = bitboards[WPAWN] | bitboards[WKNIGHT] | bitboards[WBISHOP] | bitboards[WROOK] | bitboards[WQUEEN] | bitboards[WKING];
        Black = bitboards[BPAWN] | bitboards[BKNIGHT] | bitboards[BBISHOP] | bitboards[BROOK] | bitboards[BQUEEN] | bitboards[BKING];
        Occ = Black | White;
        if (!is_square_attacked(IsWhite, KingSQ)) moves |= (1ULL << ep);
        placePiece(ourPawn, sq);
        placePiece(theirPawn, ep + offset);
        removePiece(ourPawn, ep);
        White = bitboards[WPAWN] | bitboards[WKNIGHT] | bitboards[WBISHOP] | bitboards[WROOK] | bitboards[WQUEEN] | bitboards[WKING];
        Black = bitboards[BPAWN] | bitboards[BKNIGHT] | bitboards[BBISHOP] | bitboards[BROOK] | bitboards[BQUEEN] | bitboards[BKING];
        Occ = Black | White;
    }
    return moves;
}

// legal knight moves
inline U64 Board::legal_knight_moves(bool IsWhite, int8_t sq) {
    if (_test_bit(pin_dg, sq) or _test_bit(pin_hv, sq)) {
        return 0ULL;
    }
    return GetKnightAttacks(sq) & EnemyOrEmpty(IsWhite) & checkmask;
}

// legal bishop moves
inline U64 Board::legal_bishop_moves(bool IsWhite, int8_t sq) {
    if (_test_bit(pin_dg, sq)) {
        return GetBishopAttacks(sq, Occ) & EnemyOrEmpty(IsWhite) & checkmask & pin_dg;
    }
    if (_test_bit(pin_hv, sq)) {
        return 0ULL;
    }

    return GetBishopAttacks(sq, Occ) & EnemyOrEmpty(IsWhite) & checkmask;
}

// legal rook moves
inline U64 Board::legal_rook_moves(bool IsWhite, int8_t sq) {
    if (_test_bit(pin_hv, sq)) {
        return GetRookAttacks(sq, Occ) & EnemyOrEmpty(IsWhite) & checkmask & pin_hv;
    }
    if (_test_bit(pin_dg, sq)) {
        return 0ULL;
    }
    return GetRookAttacks(sq, Occ) & EnemyOrEmpty(IsWhite) & checkmask;
}

// legal queen moves
inline U64 Board::legal_queen_moves(bool IsWhite, int8_t sq) {
    return legal_rook_moves(IsWhite, sq) | legal_bishop_moves(IsWhite, sq);
}

// legal king moves
inline U64 Board::legal_king_moves(bool IsWhite, int8_t sq) {
    U64 moves = GetKingAttacks(sq) & EnemyOrEmpty(IsWhite);
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
    if (18446744073709551615ULL == checkmask) {
        if (IsWhite) {
            if (castling_rights & wk
                && !(Occ & (1ULL << SQ_F1))
                && !(Occ & (1ULL << SQ_G1))
                && (bitboards[WROOK] & (1ULL << SQ_H1))
                && !is_square_attacked(IsWhite, SQ_F1)
                && !is_square_attacked(IsWhite, SQ_G1)) {
				final_moves |= (1ULL << SQ_G1);
            }
            if (castling_rights & wq
                && !(Occ & (1ULL << SQ_D1))
                && !(Occ & (1ULL << SQ_C1))
                && !(Occ & (1ULL << SQ_B1))
                && (bitboards[WROOK] & (1ULL << SQ_A1))
                && !is_square_attacked(IsWhite, SQ_D1)
                && !is_square_attacked(IsWhite, SQ_C1)) {
                final_moves |= (1ULL << SQ_C1);
            }
        }
        else {
            if (castling_rights & bk
                && !(Occ & (1ULL << SQ_F8))
                && !(Occ & (1ULL << SQ_G8))
                && (bitboards[BROOK] & (1ULL << SQ_H8))
                && !is_square_attacked(IsWhite, SQ_F8)
                && !is_square_attacked(IsWhite, SQ_G8)) {
                final_moves |= (1ULL << SQ_G8);
            }
            if (castling_rights & bq
                && !(Occ & (1ULL << SQ_D8))
                && !(Occ & (1ULL << SQ_C8))
                && !(Occ & (1ULL << SQ_B8))
                && (bitboards[BROOK] & (1ULL << SQ_A8))
                && !is_square_attacked(IsWhite, SQ_D8)
                && !is_square_attacked(IsWhite, SQ_C8)) {
                final_moves |= (1ULL << SQ_C8);
            }
        }
    }

    return final_moves;
}

// bool if square is attacked
inline bool Board::is_square_attacked(bool IsWhite, int8_t sq) {
    U64 mask = (1ULL << sq);
	if (GetPawnAttacks(IsWhite, sq) & Pawns(!IsWhite)) return true;
    if (knightattacks[sq] & Knights(!IsWhite)) return true;
    if (GetKingAttacks(sq) & King(!IsWhite)) return true;
	if (GetBishopAttacks(sq, OwnColor(IsWhite) | Enemy(IsWhite)) & (Bishops(!IsWhite) | Queens(!IsWhite))) return true;
    if (GetRookAttacks(sq, OwnColor(IsWhite) | Enemy(IsWhite)) & (Rooks(!IsWhite) | Queens(!IsWhite))) return true;
    return false;
}

// creates the pin mask
inline void Board::create_pin_masks(bool IsWhite) {
    int8_t sq = _bitscanforward(King(IsWhite));
    U64 them = (IsWhite) ? Black : White;
    U64 rook_mask = (Rooks(!IsWhite) | Queens(!IsWhite)) & GetRookAttacks(sq, them);
    U64 bishop_mask = (Bishops(!IsWhite) | Queens(!IsWhite)) & GetBishopAttacks(sq, them);
    U64 rook_pin = 0ULL;
    U64 bishop_pin = 0ULL;
    while (rook_mask) {
        int8_t index = pop_lsb(rook_mask);
        U64 possible_pin = (SQUARES_BETWEEN_BB[sq][index] | (1ULL << index));
        U64 us = (IsWhite) ? White : Black;
        if (popcount(possible_pin & us) == 1)
            rook_pin |= possible_pin;
    }
    while (bishop_mask) {
        int8_t index = pop_lsb(bishop_mask);
        U64 possible_pin = (SQUARES_BETWEEN_BB[sq][index] | (1ULL << index));
        U64 us = (IsWhite) ? White : Black;
        if (popcount(possible_pin & us) == 1)
            bishop_pin |= possible_pin;
    }
    pin_hv = rook_pin;
    pin_dg = bishop_pin;

}

// creates the checkmask
U64 Board::create_checkmask(bool IsWhite, int8_t sq) {
    U64 checks = 0ULL;
    U64 pawn_mask   = Pawns(!IsWhite) & GetPawnAttacks(IsWhite, sq);
    U64 knight_mask = Knights(!IsWhite) & GetKnightAttacks(sq);
    U64 bishop_mask = (Bishops(!IsWhite) | Queens(!IsWhite)) & GetBishopAttacks(sq, Occ) & ~OwnColor(IsWhite);
    U64 rook_mask   = (Rooks(!IsWhite) | Queens(!IsWhite)) & GetRookAttacks(sq, Occ) & ~OwnColor(IsWhite);
    doublecheck = 0;
    if (pawn_mask) {
        checks |= pawn_mask;
        doublecheck++;
    }
    if (knight_mask) {
        checks |= knight_mask;
        doublecheck++;
    }
    if (bishop_mask) {
        if (popcount(bishop_mask) > 1)
            doublecheck++;

        int8_t index = _bitscanforward(bishop_mask);
        checks |= SQUARES_BETWEEN_BB[sq][index] | (1ULL << index);
        doublecheck++;
    }
    if (rook_mask) {
        if (popcount(rook_mask) > 1)
            doublecheck++;

        int8_t index = _bitscanforward(rook_mask);
        checks |= SQUARES_BETWEEN_BB[sq][index] | (1ULL << index);
        doublecheck++;
    }
    return checks;
}

bool Board::gives_check(Move& move) {
    make_move(move);
    bool inCheck = is_square_attacked(side_to_move, _bitscanforward(King(side_to_move)));
    unmake_move(move);
	return inCheck;
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
    int8_t piece = move.piece + (side_to_move * 6);
    //int8_t captured_piece = move.capture;
    int8_t captured_piece = piece_at_square(to_square);
	
    bool IsWhite = side_to_move ? 0 : 1;
    bool enemy = side_to_move ^ 1;

    save_board_state();
    // Capture
    if (captured_piece == WROOK) {
        if (to_square == 7 && castling_rights & wk) {
            castling_rights ^= wk;
            board_hash ^= RANDOM_ARRAY[768];
        }
        if (to_square == 0 && castling_rights & wq) {
            castling_rights ^= wq;
            board_hash ^= RANDOM_ARRAY[768 + 1];
        }
    }
    if (captured_piece == BROOK) {
        if (to_square == 63 && castling_rights & bk) {
            castling_rights ^= bk;
            board_hash ^= RANDOM_ARRAY[768 + 2];
        }
        if (to_square == 56 && castling_rights & bq) {
            castling_rights ^= bq;
            board_hash ^= RANDOM_ARRAY[768 + 3];
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
        // Actual castling
        if (square_distance(from_square, to_square) == 2) {
            if (to_square == 6 and _test_bit(bitboards[WROOK], 7)) {
                if (castling_rights & wk) {
                    bitboards[WROOK] &= ~(1ULL << 7);
                    bitboards[WROOK] |= (1ULL << 5);
					
                    board_pieces[7] = -1;
                    board_pieces[5] = WROOK;
					
                    board_hash ^= RANDOM_ARRAY[64 * (2 * 3 + 1) + 7];
                    board_hash ^= RANDOM_ARRAY[64 * (2 * 3 + 1) + 5];
                }
            }
            if (to_square == 2 and _test_bit(bitboards[WROOK], 0)) {
                if (castling_rights & wq) {
                    bitboards[WROOK] &= ~(1ULL << 0);
                    bitboards[WROOK] |= (1ULL << 3);
					
                    board_pieces[0] = -1;
                    board_pieces[3] = WROOK;


                    board_hash ^= RANDOM_ARRAY[64 * (2 * 3 + 1) + 0];
                    board_hash ^= RANDOM_ARRAY[64 * (2 * 3 + 1) + 3];
                }
            }
        }
        if (castling_rights & wk)
            board_hash ^= RANDOM_ARRAY[768];
		
        if (castling_rights & wq)
            board_hash ^= RANDOM_ARRAY[768 + 1];
		
        castling_rights &= ~(1);
        castling_rights &= ~(2);
    }
    if (piece == BKING) {
        if (square_distance(from_square, to_square) == 2) {
            if (to_square == 62 and _test_bit(bitboards[BROOK], 63)) {
                if (castling_rights & bk) {
                    bitboards[BROOK] &= ~(1ULL << 63);
                    bitboards[BROOK] |= (1ULL << 61);
					
                    board_pieces[63] = -1;
                    board_pieces[61] = BROOK;

                    board_hash ^= RANDOM_ARRAY[64 * (2 * 3) + 63];
                    board_hash ^= RANDOM_ARRAY[64 * (2 * 3) + 61];
                }
            }
            if (to_square == 58 and _test_bit(bitboards[BROOK], 56)) {
                if (castling_rights & bq) {
                    bitboards[BROOK] &= ~(1ULL << 56);
                    bitboards[BROOK] |= (1ULL << 59);
					
                    board_pieces[56] = -1;
                    board_pieces[59] = BROOK;

                    board_hash ^= RANDOM_ARRAY[64 * (2 * 3) + 56];
                    board_hash ^= RANDOM_ARRAY[64 * (2 * 3) + 59];
                }
            }
        }
        if (castling_rights & bq)
            board_hash ^= RANDOM_ARRAY[768 + 3];
        
        if (castling_rights & bk)
            board_hash ^= RANDOM_ARRAY[768 + 2];
        
        castling_rights &= ~(4);
        castling_rights &= ~(8);
    }
    // Rook move loses castle rights
    if (piece == WROOK) {
        if (from_square == 7 && castling_rights & wk) {
            castling_rights ^= wk;
            board_hash ^= RANDOM_ARRAY[768];
        }
        if (from_square == 0 && castling_rights & wq) {
            castling_rights ^= wq;
            board_hash ^= RANDOM_ARRAY[768 + 1];
        }
    }
    if (piece == BROOK) {
        if (from_square == 63 && castling_rights & bk) {
            castling_rights ^= bk;
            board_hash ^= RANDOM_ARRAY[768 + 2];
        }
        if (from_square == 56 && castling_rights & bq) {
            castling_rights ^= bq;
            board_hash ^= RANDOM_ARRAY[768 + 3];
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
        if (GetPawnAttacks(true, from_square + 8) & bitboards[BPAWN]) {
            en_passant_square = from_square + 8;
            board_hash ^= RANDOM_ARRAY[772 + square_file(en_passant_square)];
        }
    }
    if (piece == BPAWN and (from_square - to_square) == 16) {
        if (GetPawnAttacks(false, from_square - 8) & bitboards[WPAWN]) {
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

    gameHistory[full_moves] = board_hash;
    full_moves++;
};

void Board::unmake_move(Move& move) {
    if (move_stack_index >= 0) {
        move_stack_index--;
        BoardState board = move_stack[move_stack_index];
		
        en_passant_square = board.en_passant;
        castling_rights = board.castle_rights;
        gameHistory[full_moves] = 0;
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
    if (move_stack_index >= 0) {
        move_stack_index--;
        BoardState board = move_stack[move_stack_index];
        side_to_move ^= 1;
        board_hash ^= RANDOM_ARRAY[780];
        en_passant_square = board.en_passant;
        full_moves--;
    }
}

MoveList Board::generate_legal_moves() {
    Move move{};
    MoveList possible_moves{};
    possible_moves.size = 0;
    int from_index = 0;
    U64 we = side_to_move ? Black : White;
    bool IsWhite = side_to_move ? false : true;

    init(IsWhite);
    U64 move_mask = 0ULL;
    
    if (doublecheck < 2) {
        U64 pawn_mask = (bitboards[BPAWN] | bitboards[WPAWN]) & we;
        U64 knight_mask = (bitboards[BKNIGHT] | bitboards[WKNIGHT]) & we;
        U64 bishop_mask = (bitboards[BBISHOP] | bitboards[WBISHOP]) & we;
        U64 rook_mask = (bitboards[BROOK] | bitboards[WROOK]) & we;
        U64 queen_mask = (bitboards[BQUEEN] | bitboards[WQUEEN]) & we;
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
	
    int king_sq = _bitscanforward(King(IsWhite));
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

MoveList Board::generate_non_quite_moves() {
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
            move_mask = inCheck ? all_moves : all_moves & (Enemy(IsWhite) | rank_8_mask | rank_1_mask);
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

// returns diagonal of given square
uint8_t Board::diagonal_of(int8_t sq) {
    return 7 + square_rank(sq) - square_file(sq);
}

// returns anti diagonal of given square
uint8_t Board::anti_diagonal_of(int8_t sq) {
    return square_rank(sq) + square_file(sq);
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