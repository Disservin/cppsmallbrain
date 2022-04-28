#include <sstream>
#include <map>

#include "chess.h"
#include "search.h"
#include "threadmanager.h"

using namespace Chess;

std::atomic<bool> stopped;
ThreadManager thread;

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

Move convert_uci_to_Move(std::string input, Board& board) {
    Move move;
    if (input.length() == 4) {
        std::string from = input.substr(0, 2);
        std::string to = input.substr(2);
        int from_index;
        int to_index;
        char letter;
        letter = from[0];
        int file = letter - 96;
        int rank = from[1] - 48;
        from_index = (rank - 1) * 8 + file - 1;
        Square source = Square(from_index);
        letter = to[0];
        file = letter - 96;
        rank = to[1] - 48;
        to_index = (rank - 1) * 8 + file - 1;
        Square target = Square(to_index);
        PieceType piece = PieceType(board.getPiece(source)%6);
        return Move(source, target, piece, 0);
    }
    if (input.length() == 5) {
        std::string from = input.substr(0, 2);
        std::string to = input.substr(2,2);
        int from_index;
        int to_index;
        char letter;
        letter = from[0];
        int file = letter - 96;
        int rank = from[1] - 48;
        from_index = (rank - 1) * 8 + file - 1;

        Square source = Square(from_index);
        letter = to[0];
        file = letter - 96;
        rank = to[1] - 48;
        to_index = (rank - 1) * 8 + file - 1;
        Square target = Square(to_index);
        std::map<char, int> piece_to_int = 
        {
        { 'n', 1 },
        { 'b', 2 },
        { 'r', 3 },
        { 'q', 4 }
        };
        char prom = input.at(4);
        PieceType piece = PieceType(piece_to_int[prom]);
        return Move(source, target, piece, 1);
    }
    else {
        std::cout << "FALSE INPUT" << std::endl;
        return Move(Square(0), Square(0), PieceType(0), 0);
    }
}

int main(int argc, char** argv){
    Board board = Board();
    stopped = false;
    while (true){
        if (argc > 1) {
            if (argv[1] == std::string("bench")) {
                // auto t1 = std::chrono::high_resolution_clock::now();
                ThreadManager threads;
                threads.begin(board, 5, true);
                // auto t2 = std::chrono::high_resolution_clock::now();
                // auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
                // unsigned long long nodes = search.nodes;
                // std::cout << nodes <<" nodes " << nodes << signed((nodes/(ms+1))*1000) << " nps"<< std::endl;
                // printMove(search.bestMove);
                threads.stop();
                return 0;
            }
        }		
        std::string input;
        std::getline(std::cin, input);		
        if (input == "uci"){
            std::cout << "id name Chess\n" <<
            "id author Disservin\n"        <<
            "uciok\n"                      << std::endl;

        }
        if (input == "isready"){
            std::cout << "readyok\n" << std::endl;
        }
        if (input == "ucinewgame"){
            board = Board();
        }
        if (input == "quit"){
            return 0;
        }
        if (input == "stop"){
            thread.stop();
        }
        if (input == "print"){
            board.print();
            // search.startsearch(depth);
        }
        if (input == "moves"){
            if (board.sideToMove == White){
                Moves moveList = board.generateLegalMoves<White>();
                for (int i = 0; i < moveList.count; i++){
                    Move move = moveList.moves[i];
                    std::cout << board.printUciMove(move) << std::endl;
                }
            }else{
                Moves moveList = board.generateLegalMoves<Black>();
                for (int i = 0; i < moveList.count; i++){
                    Move move = moveList.moves[i];
                    std::cout << board.printUciMove(move) << std::endl;
                }
            }
                
        }
        if (input.find("position") != std::string::npos){
            std::vector<std::string> tokens = split_input(input);
            if (tokens[1] == "startpos"){
                board = Board();
            }
            if (tokens[1] == "fen"){
                std::size_t start_index = input.find("fen");
                std::string fen = input.substr(start_index + 4);
                board = Board(fen);
            }
            if (input.find("moves") != std::string::npos){
                std::size_t index = std::find(tokens.begin(), tokens.end(), "moves") - tokens.begin();
                index ++;
                Color moveright = board.getSideToMove();
				for ( ; index < tokens.size(); index++) {
					Move move = convert_uci_to_Move(tokens[index], board);
                    if (moveright == White){
                        board.makemove<White>(move);
                    }
                    else {
                        board.makemove<Black>(move);
                    }
                    moveright = board.getSideToMove();
				}
            }
        }
        if (input.find("go depth") != std::string::npos){
            std::vector<std::string> tokens = split_input(input);
            int depth = std::stoi(tokens[2]);
            thread.begin(board, depth);
        }
        if (input.find("go infinite") != std::string::npos){
            std::vector<std::string> tokens = split_input(input);
            int depth = 120;
            thread.begin(board, depth);
            // search.startsearch(depth);
        }
        if (input.find("go wtime") != std::string::npos) {
            std::vector<std::string> param = split_input(input);
            int movetime = board.sideToMove ? std::stoi(param[4]) : std::stoi(param[2]);
            int inc = -1;
            int movestogo = 0;
            if (param.size() > 5) {
                inc = board.sideToMove ? std::stoi(param[8]) : std::stoi(param[6]);
            }
            if (param.size() > 9) {
                movestogo = std::stoi(param[10]);
            }
            unsigned long long time_given = movetime / 20;
            thread.begin(board, 60, false, time_given);
        }

    }
}