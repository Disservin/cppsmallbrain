#pragma once

#include <string>
#include <vector>
#include <bitset>
#include <iostream>
#include <stack>
#include <unordered_map>

#include "general.h"

#define get_bit(bitboard, index) (bitboard & 1ULL << index))
#define set_bit(bitboard, index) (bitboard |= (1ULL << index))
#define pop_bit(bitboard, index) (get_bit(bitboard, index) ? bitboard ^= (1ULL << index):0)

std::vector<std::string> split_input(std::string fen);

extern int zpieces[12];
extern U64 RANDOM_ARRAY[781];

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

struct MoveList {
    Move movelist[256];
    int e=0;
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
    int piece_loc[64];
    U64 board_hash;
};

class Board
{
public:
    // Move directions
    enum Dir {
        NORTH,
        SOUTH,
        EAST,
        WEST,
        NORTH_EAST,
        NORTH_WEST,
        SOUTH_EAST,
        SOUTH_WEST
    };
    // Piece
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
    // Piece Types
    enum {
        PAWN,
        KNIGHT,
        BISHOP,
        ROOK,
        QUEEN,
        KING
    };

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

    // Squares to str
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

    //WhitePawns, WhiteKnights, WhiteBishops, WhiteRooks, WhiteQueens, WhiteKing,
    //BlackPawns, BlackKnights, BlackBishops, BlackRooks, BlackQueens, BlackKing
    U64 bitboards[12] = { 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL };

    U64 White = bitboards[WPAWN] | bitboards[WKNIGHT] | bitboards[WBISHOP] | bitboards[WROOK] | bitboards[WQUEEN] | bitboards[WKING];
    U64 Black = bitboards[BPAWN] | bitboards[BKNIGHT] | bitboards[BBISHOP] | bitboards[BROOK] | bitboards[BQUEEN] | bitboards[BKING];

    // U64 0 = AllWhitePieces 1 = AllBlackPieces
    U64 occupancies[2] = { White, Black };

    // All pieces
    U64 Occ = White | Black;

    // 0 White 1 Black
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

    static constexpr  U64 not_a_file = 0xfefefefefefefefe;

    // not H file constant
    static constexpr  U64 not_h_file = 0x7f7f7f7f7f7f7f7f;

    // not HG file constant
    static constexpr  U64 not_hg_file = 4557430888798830399ULL;

    // not AB file constant
    static constexpr  U64 not_ab_file = 18229723555195321596ULL;

    //Ranks
    static constexpr  U64 rank_1_mask = 255ULL;
    static constexpr  U64 rank_2_mask = 18446744069431296255ULL;
    static constexpr  U64 rank_7_mask = 18374966859431673855ULL;
    static constexpr  U64 rank_8_mask = 18374686479671623680ULL;

    static constexpr  U64 knightattacks[64] = { 132096ULL, 329728ULL, 659712ULL, 1319424ULL, 2638848ULL, 5277696ULL, 10489856ULL, 4202496ULL, 33816580ULL, 84410376ULL, 168886289ULL, 337772578ULL, 675545156ULL, 1351090312ULL, 2685403152ULL, 1075839008ULL, 8657044482ULL, 21609056261ULL, 43234889994ULL, 86469779988ULL, 172939559976ULL, 345879119952ULL, 687463207072ULL, 275414786112ULL, 2216203387392ULL, 5531918402816ULL, 11068131838464ULL, 22136263676928ULL, 44272527353856ULL, 88545054707712ULL, 175990581010432ULL, 70506185244672ULL, 567348067172352ULL, 1416171111120896ULL, 2833441750646784ULL, 5666883501293568ULL, 11333767002587136ULL, 22667534005174272ULL, 45053588738670592ULL, 18049583422636032ULL, 145241105196122112ULL, 362539804446949376ULL, 725361088165576704ULL, 1450722176331153408ULL, 2901444352662306816ULL, 5802888705324613632ULL, 11533718717099671552ULL, 4620693356194824192ULL, 288234782788157440ULL, 576469569871282176ULL, 1224997833292120064ULL, 2449995666584240128ULL, 4899991333168480256ULL, 9799982666336960512ULL, 1152939783987658752ULL, 2305878468463689728ULL, 1128098930098176ULL, 2257297371824128ULL, 4796069720358912ULL, 9592139440717824ULL, 19184278881435648ULL, 38368557762871296ULL, 4679521487814656ULL, 9077567998918656ULL };
    static constexpr U64 _rays[8][64] = { {72340172838076672ULL, 144680345676153344ULL, 289360691352306688ULL, 578721382704613376ULL, 1157442765409226752ULL, 2314885530818453504ULL, 4629771061636907008ULL, 9259542123273814016ULL, 72340172838076416ULL, 144680345676152832ULL, 289360691352305664ULL, 578721382704611328ULL, 1157442765409222656ULL, 2314885530818445312ULL, 4629771061636890624ULL, 9259542123273781248ULL, 72340172838010880ULL, 144680345676021760ULL, 289360691352043520ULL, 578721382704087040ULL, 1157442765408174080ULL, 2314885530816348160ULL, 4629771061632696320ULL, 9259542123265392640ULL, 72340172821233664ULL, 144680345642467328ULL, 289360691284934656ULL, 578721382569869312ULL, 1157442765139738624ULL, 2314885530279477248ULL, 4629771060558954496ULL, 9259542121117908992ULL, 72340168526266368ULL, 144680337052532736ULL, 289360674105065472ULL, 578721348210130944ULL, 1157442696420261888ULL, 2314885392840523776ULL, 4629770785681047552ULL, 9259541571362095104ULL, 72339069014638592ULL, 144678138029277184ULL, 289356276058554368ULL, 578712552117108736ULL, 1157425104234217472ULL, 2314850208468434944ULL, 4629700416936869888ULL, 9259400833873739776ULL, 72057594037927936ULL, 144115188075855872ULL, 288230376151711744ULL, 576460752303423488ULL, 1152921504606846976ULL, 2305843009213693952ULL, 4611686018427387904ULL, 9223372036854775808ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL}, {0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 1ULL, 2ULL, 4ULL, 8ULL, 16ULL, 32ULL, 64ULL, 128ULL, 257ULL, 514ULL, 1028ULL, 2056ULL, 4112ULL, 8224ULL, 16448ULL, 32896ULL, 65793ULL, 131586ULL, 263172ULL, 526344ULL, 1052688ULL, 2105376ULL, 4210752ULL, 8421504ULL, 16843009ULL, 33686018ULL, 67372036ULL, 134744072ULL, 269488144ULL, 538976288ULL, 1077952576ULL, 2155905152ULL, 4311810305ULL, 8623620610ULL, 17247241220ULL, 34494482440ULL, 68988964880ULL, 137977929760ULL, 275955859520ULL, 551911719040ULL, 1103823438081ULL, 2207646876162ULL, 4415293752324ULL, 8830587504648ULL, 17661175009296ULL, 35322350018592ULL, 70644700037184ULL, 141289400074368ULL, 282578800148737ULL, 565157600297474ULL, 1130315200594948ULL, 2260630401189896ULL, 4521260802379792ULL, 9042521604759584ULL, 18085043209519168ULL, 36170086419038336ULL}, {254ULL, 252ULL, 248ULL, 240ULL, 224ULL, 192ULL, 128ULL, 0ULL, 65024ULL, 64512ULL, 63488ULL, 61440ULL, 57344ULL, 49152ULL, 32768ULL, 0ULL, 16646144ULL, 16515072ULL, 16252928ULL, 15728640ULL, 14680064ULL, 12582912ULL, 8388608ULL, 0ULL, 4261412864ULL, 4227858432ULL, 4160749568ULL, 4026531840ULL, 3758096384ULL, 3221225472ULL, 2147483648ULL, 0ULL, 1090921693184ULL, 1082331758592ULL, 1065151889408ULL, 1030792151040ULL, 962072674304ULL, 824633720832ULL, 549755813888ULL, 0ULL, 279275953455104ULL, 277076930199552ULL, 272678883688448ULL, 263882790666240ULL, 246290604621824ULL, 211106232532992ULL, 140737488355328ULL, 0ULL, 71494644084506624ULL, 70931694131085312ULL, 69805794224242688ULL, 67553994410557440ULL, 63050394783186944ULL, 54043195528445952ULL, 36028797018963968ULL, 0ULL, 18302628885633695744ULL, 18158513697557839872ULL, 17870283321406128128ULL, 17293822569102704640ULL, 16140901064495857664ULL, 13835058055282163712ULL, 9223372036854775808ULL, 0ULL}, {0ULL, 1ULL, 3ULL, 7ULL, 15ULL, 31ULL, 63ULL, 127ULL, 0ULL, 256ULL, 768ULL, 1792ULL, 3840ULL, 7936ULL, 16128ULL, 32512ULL, 0ULL, 65536ULL, 196608ULL, 458752ULL, 983040ULL, 2031616ULL, 4128768ULL, 8323072ULL, 0ULL, 16777216ULL, 50331648ULL, 117440512ULL, 251658240ULL, 520093696ULL, 1056964608ULL, 2130706432ULL, 0ULL, 4294967296ULL, 12884901888ULL, 30064771072ULL, 64424509440ULL, 133143986176ULL, 270582939648ULL, 545460846592ULL, 0ULL, 1099511627776ULL, 3298534883328ULL, 7696581394432ULL, 16492674416640ULL, 34084860461056ULL, 69269232549888ULL, 139637976727552ULL, 0ULL, 281474976710656ULL, 844424930131968ULL, 1970324836974592ULL, 4222124650659840ULL, 8725724278030336ULL, 17732923532771328ULL, 35747322042253312ULL, 0ULL, 72057594037927936ULL, 216172782113783808ULL, 504403158265495552ULL, 1080863910568919040ULL, 2233785415175766016ULL, 4539628424389459968ULL, 9151314442816847872ULL}, {9241421688590303744ULL, 36099303471055872ULL, 141012904183808ULL, 550831656960ULL, 2151686144ULL, 8404992ULL, 32768ULL, 0ULL, 4620710844295151616ULL, 9241421688590303232ULL, 36099303471054848ULL, 141012904181760ULL, 550831652864ULL, 2151677952ULL, 8388608ULL, 0ULL, 2310355422147510272ULL, 4620710844295020544ULL, 9241421688590041088ULL, 36099303470530560ULL, 141012903133184ULL, 550829555712ULL, 2147483648ULL, 0ULL, 1155177711056977920ULL, 2310355422113955840ULL, 4620710844227911680ULL, 9241421688455823360ULL, 36099303202095104ULL, 141012366262272ULL, 549755813888ULL, 0ULL, 577588851233521664ULL, 1155177702467043328ULL, 2310355404934086656ULL, 4620710809868173312ULL, 9241421619736346624ULL, 36099165763141632ULL, 140737488355328ULL, 0ULL, 288793326105133056ULL, 577586652210266112ULL, 1155173304420532224ULL, 2310346608841064448ULL, 4620693217682128896ULL, 9241386435364257792ULL, 36028797018963968ULL, 0ULL, 144115188075855872ULL, 288230376151711744ULL, 576460752303423488ULL, 1152921504606846976ULL, 2305843009213693952ULL, 4611686018427387904ULL, 9223372036854775808ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL}, {0ULL, 256ULL, 66048ULL, 16909312ULL, 4328785920ULL, 1108169199616ULL, 283691315109888ULL, 72624976668147712ULL, 0ULL, 65536ULL, 16908288ULL, 4328783872ULL, 1108169195520ULL, 283691315101696ULL, 72624976668131328ULL, 145249953336262656ULL, 0ULL, 16777216ULL, 4328521728ULL, 1108168671232ULL, 283691314053120ULL, 72624976666034176ULL, 145249953332068352ULL, 290499906664136704ULL, 0ULL, 4294967296ULL, 1108101562368ULL, 283691179835392ULL, 72624976397598720ULL, 145249952795197440ULL, 290499905590394880ULL, 580999811180789760ULL, 0ULL, 1099511627776ULL, 283673999966208ULL, 72624942037860352ULL, 145249884075720704ULL, 290499768151441408ULL, 580999536302882816ULL, 1161999072605765632ULL, 0ULL, 281474976710656ULL, 72620543991349248ULL, 145241087982698496ULL, 290482175965396992ULL, 580964351930793984ULL, 1161928703861587968ULL, 2323857407723175936ULL, 0ULL, 72057594037927936ULL, 144115188075855872ULL, 288230376151711744ULL, 576460752303423488ULL, 1152921504606846976ULL, 2305843009213693952ULL, 4611686018427387904ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL}, {0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 2ULL, 4ULL, 8ULL, 16ULL, 32ULL, 64ULL, 128ULL, 0ULL, 516ULL, 1032ULL, 2064ULL, 4128ULL, 8256ULL, 16512ULL, 32768ULL, 0ULL, 132104ULL, 264208ULL, 528416ULL, 1056832ULL, 2113664ULL, 4227072ULL, 8388608ULL, 0ULL, 33818640ULL, 67637280ULL, 135274560ULL, 270549120ULL, 541097984ULL, 1082130432ULL, 2147483648ULL, 0ULL, 8657571872ULL, 17315143744ULL, 34630287488ULL, 69260574720ULL, 138521083904ULL, 277025390592ULL, 549755813888ULL, 0ULL, 2216338399296ULL, 4432676798592ULL, 8865353596928ULL, 17730707128320ULL, 35461397479424ULL, 70918499991552ULL, 140737488355328ULL, 0ULL, 567382630219904ULL, 1134765260439552ULL, 2269530520813568ULL, 4539061024849920ULL, 9078117754732544ULL, 18155135997837312ULL, 36028797018963968ULL, 0ULL}, {0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 1ULL, 2ULL, 4ULL, 8ULL, 16ULL, 32ULL, 64ULL, 0ULL, 256ULL, 513ULL, 1026ULL, 2052ULL, 4104ULL, 8208ULL, 16416ULL, 0ULL, 65536ULL, 131328ULL, 262657ULL, 525314ULL, 1050628ULL, 2101256ULL, 4202512ULL, 0ULL, 16777216ULL, 33619968ULL, 67240192ULL, 134480385ULL, 268960770ULL, 537921540ULL, 1075843080ULL, 0ULL, 4294967296ULL, 8606711808ULL, 17213489152ULL, 34426978560ULL, 68853957121ULL, 137707914242ULL, 275415828484ULL, 0ULL, 1099511627776ULL, 2203318222848ULL, 4406653222912ULL, 8813306511360ULL, 17626613022976ULL, 35253226045953ULL, 70506452091906ULL, 0ULL, 281474976710656ULL, 564049465049088ULL, 1128103225065472ULL, 2256206466908160ULL, 4512412933881856ULL, 9024825867763968ULL, 18049651735527937ULL} };

    static constexpr  uint64_t File1 = 0b1000000010000000100000001000000010000000100000001000000010000000ul;
    static constexpr  uint64_t File8 = 0b0000000100000001000000010000000100000001000000010000000100000001ul;
    static constexpr  uint64_t Rank2 = 0b0000000000000000000000000000000000000000000000001111111100000000ul;
    static constexpr  uint64_t Rank7 = 0b0000000011111111000000000000000000000000000000000000000000000000ul;
    static constexpr  uint64_t RankMid = 0x0000FFFFFFFF0000;
    static constexpr  uint64_t Rank_18 = 0xFF000000000000FF;

    // All bits set to 1 if theres no check / on startup
    uint64_t checkmask = 18446744073709551615ULL;
    
    uint64_t attacked_squares = 0ULL;
    
    uint64_t pin_hv = 0ULL;
    
    uint64_t pin_dg = 0ULL;

    int doublecheck = 0;

    std::stack<BoardState> move_stack = {};
    
    std::unordered_map<U64, int> repetition_table;

    int board_pieces[64] = {-1};

    U64 board_hash = 0ULL;

    void apply_fen(std::string fen);

    int get_en_passant_square();

    U64 generate_zhash();

    void safe_board_state();

    void update_occupancies();

    void add_repetition(U64 hash);

    void remove_repetition(U64 hash);

    bool is_threefold_rep();

    bool is_threefold_rep3();

    void make_move(Move& move);

    void unmake_move();

    void print_bitboard(std::bitset<64> bitset);

    void print_board();

    int piece_at_square(int sq) {
        return board_pieces[sq];
    }
    // returns color specific int for piece
    int piece_at(int sq, int given = -1) {
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
    int piece_type_at(int sq) {
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

    std::string piece_type(int piece);

    U64 Pawn_Forward(bool IsWhite, U64 mask);

    U64 Pawn_Forward2(bool IsWhite, U64 mask);

    U64 Pawn_Backward(bool IsWhite, U64 mask);

    U64 Pawn_Backward2(bool IsWhite, U64 mask);

    U64 Pawn_AttackLeft(bool IsWhite, U64 mask);

    U64 Pawn_AttackRight(bool IsWhite, U64 mask);

    U64 King(bool IsWhite);

    U64 EnemyKing(bool IsWhite);

    U64 EnemyPawn(bool IsWhite);

    U64 Pawns(bool IsWhite);

    U64 OwnColor(bool IsWhite);

    U64 Enemy(bool IsWhite);

    U64 EnemyRookQueen(bool IsWhite);

    U64 RookQueen(bool IsWhite);

    U64 EnemyBishopQueen(bool IsWhite);

    U64 BishopQueen(bool IsWhite);

    U64 KingPawn(bool IsWhite);

    U64 EnemyKingPawn(bool IsWhite);

    U64 EnemyOrEmpty(bool IsWhite);

    U64 Empty();

    U64 Knights(bool IsWhite);

    U64 Rooks(bool IsWhite);

    U64 Bishops(bool IsWhite);

    U64 Queens(bool IsWhite);

    U64 create_checkmask(bool IsWhite, int sq);

    U64 would_be_attack(bool IsWhite, int sq);

    U64 is_pinned_hv(bool IsWhite, int sq);

    U64 is_pinned_dg(bool IsWhite, int sq);

    U64 seen_by_pawn(bool IsWhite, int sq, int ep);

    U64 seen_by_bishop(bool IsWhite, int sq);

    U64 seen_by_knight(int sq);

    U64 seen_by_rook(bool IsWhite, int sq);

    U64 seen_by_king(int sq);

    U64 valid_pawn_moves(bool IsWhite, int sq, int ep = 64);

    U64 valid_knight_moves(bool IsWhite, int sq);

    U64 valid_bishop_moves(bool IsWhite, int sq);

    U64 valid_rook_moves(bool IsWhite, int sq);

    U64 valid_queen_moves(bool IsWhite, int sq);

    U64 valid_king_moves(bool IsWhite, int sq);

    bool is_square_attacked(bool IsWhite, int sq);

    void create_pin_masks(bool IsWhite);

    bool is_checkmate(bool IsWhite);

    bool is_stalemate(bool IsWhite);

    int is_game_over(bool IsWhite);

    void init(bool IsWhite);

    MoveList generate_legal_moves();

    MoveList generate_capture_moves();
};

class Perft {
public:
    Board* this_board;
    Perft(Board* x) {
        this_board = x;
    }
    U64 speed_test_perft(int depth, int max);

    U64 bulk_test_perft(int depth, int max);

    void test();
};
