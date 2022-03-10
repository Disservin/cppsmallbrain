#pragma once
#include <string>
#include <vector>
#include <bitset>
#include <iostream>

#define U64 unsigned __int64
#define get_bit(bitboard, index) (bitboard & 1ULL << index))
#define set_bit(bitboard, index) (bitboard |= (1ULL << index))
#define pop_bit(bitboard, index) (get_bit(bitboard, index) ? bitboard ^= (1ULL << index):0)

struct Pertft_Info {
    int from_square;
    int to_square;
    int promotion_piece;
    U64 nodes;
};
struct MA {
    //std::vector<std::vector<long long>> myarray;
    std::vector<Pertft_Info> myarray;
};
struct Move {
    int piece;
    int from_square;
    int to_square;
    int promotion;
    int capture;
    int en_passant;
    int castle;
};
struct MoveList {
    //std::vector<Move> movelist;
    Move movelist[256];
};
struct BoardState {
    U64 wpawn;
    U64 wknight;
    U64 wbishop;
    U64 wrook;
    U64 wqueen;
    U64 wking;
    U64 bpawn;
    U64 bknight;
    U64 bbishop;
    U64 brook;
    U64 bqueen;
    U64 bking;
    int en_passant;
    int castle_rights;
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
    //std::vector<std::vector<U64> >  move_stack;
    BoardState move_stack[256]={};
    int move_stack_index = 0;
    int count = 0;
    /*WhitePawns, WhiteKnights, WhiteBishops, WhiteRooks, WhiteQueens, WhiteKing,
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

    // 0 White 
    // 1 Black
    int side_to_move = 0;

    /*
    1  white king can castle to the king side
    2  white king can castle to the queen side
    4  black king can castle to the king side
    8  black king can castle to the queen side
    */
    enum { wk = 1, wq = 2, bk = 4, bq = 8 };
    int castling_rights = 0;
    //en passant square
    int en_passant_square = no_sq;

    //half_moves
    int half_moves = 0;

    //full_moves
    int full_moves = 0;

    // not A file constant
    static constexpr U64 not_a_file = 0xfefefefefefefefe;

    // not H file constant
    static constexpr U64 not_h_file = 0x7f7f7f7f7f7f7f7f;

    // not HG file constant
    static constexpr U64 not_hg_file = 4557430888798830399ULL;

    // not AB file constant
    static constexpr U64 not_ab_file = 18229723555195321596ULL;

    //Ranks
    static constexpr U64 rank_1_mask = 255ULL;
    static constexpr U64 rank_2_mask = 18446744069431296255ULL;
    static constexpr U64 rank_7_mask = 18374966859431673855ULL;
    static constexpr U64 rank_8_mask = 18374686479671623680ULL;

    std::vector<std::string> split_fen(std::string fen);
    void apply_fen(std::string fen);

    void get_side_to_move();
    void get_castling_rights();
    void get_en_passant_square();
    void get_half_moves();
    void get_full_moves();

    void update_occupancies();

    bool piece_color(int sq);
    int piece_at(int sq, int given = -1);

    int square_file(int sq);
    int square_rank(int sq);
    int square_distance(int a, int b);
    bool is_square_attacked(int sq, int cast_check = -1);
    inline int is_pinned(int sq, int piece);
    inline int is_pinned_horizontal_verti(int sq, int piece);
    inline int is_pinned_diag(int sq, int piece);
    int distance_to_edge_right(int sq);
    int distance_to_edge_left(int sq);
    BoardState encode_board_state(U64 wpawn, U64 wknight, U64 wbishop, U64 wrook, U64 wqueen, U64 wking,
                                  U64 bpawn, U64 bknight, U64 bbishop, U64 brook, U64 bqueen, U64 bking,
                                  int ep, int castle);

    //Pseudo Legal Moves
    U64 sliding_attacks(int square, int deltas[4]);
    U64 pawn_attacks(int sq);
    U64 Valid_Moves_Pawn(int sq);
    U64 Valid_Moves_Knight(int sq);
    U64 Valid_Moves_Bishop(int sq);
    U64 Valid_Moves_Rook(int sq);
    U64 Valid_Moves_Queen(int sq);
    U64 Valid_Moves_King(int sq);

    void make_move(Move move);
    void unmake_move(Move move);

    MoveList generate_moves();
    void print_bitboard(std::bitset<64> bitset);
};

class Perft {
public:
    board this_board;
    Perft(const board& x) {
        this_board = x;
    }
    U64 speed_test_perft(int x);
    U64 bulk_perft(int x, int depth);
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
};

int perft_test(std::string fen, int depth);
int test();