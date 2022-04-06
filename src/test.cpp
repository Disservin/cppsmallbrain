// #include "general.h"
// #include <bitset>
// int main(){
//     uint16_t move = 0;
//     set_promotion(move, 1);
//     set_piece(move, 5);
//     set_from_square(move, 20);
//     set_to_square(move, 63);
//     std::cout << "sq: "<< std::bitset<16>(move) << std::endl;
//     std::cout << "sq: "<< signed(get_promotion(move)) << std::endl;
//     std::cout << "sq: "<< signed(get_piece(move)) << std::endl;
//     std::cout << "sq: "<< signed(get_from_square(move)) << std::endl;
//     std::cout << "sq: "<< signed(get_to_square(move)) << std::endl;
// }

#include <cstdint>
#include <iostream>
#include <ostream>

struct Move
{
    int piece;
    int from;
    int to;
    bool promotion;
};

/*constexpr*/ auto pack_move(const Move m)
{
    return
        static_cast<std::uint_least16_t>(
            ((m.piece     &  7) <<  0) |
            ((m.from      & 63) <<  3) |
            ((m.to        & 63) <<  9) |
            ((m.promotion ? 1U : 0U) << 15)
        );
}

/*constexpr*/ Move unpack_move(const std::uint_least16_t bits)
{
    return {
        static_cast<int>((bits >>  0) &  7),
        static_cast<int>((bits >>  3) & 63),
        static_cast<int>((bits >>  9) & 63),
        static_cast<int>((bits >> 15) &  1) != 0
    };
}

struct MoveList {
    std::uint_least16_t arr[2]{};
    std::uint_least8_t size = 0;
};

int main(){
    MoveList moves;
    moves.arr[0] = pack_move(Move{5, 20, 63, true});
    moves.size++;
    moves.arr[1] = pack_move(Move{2, 10, 42, false});
    moves.size++;
    for (int e = 0; e < moves.size; e++){
        auto move = unpack_move(moves.arr[e]);
        std::cout << "piece: " <<move.piece << std::endl;
        std::cout << "from: " <<move.from << std::endl;
        std::cout << "to: " <<move.to << std::endl;
        std::cout << "promotion: " <<move.promotion << std::endl;
        std::cout << "\n" << std::endl;
    }
}