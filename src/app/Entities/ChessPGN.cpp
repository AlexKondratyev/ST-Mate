#include "ChessPGN.h"

ChessPGN::ChessPGN()
    : event("?"), site("?"), date("?"), round("?"),
    white("?"), black("?"), gameResult(GameResult::ONGOING)
{
}

ChessPGN::ChessPGN(const std::string& whiteName, const std::string& blackName)
    : event("?"), site("?"), date("?"), round("?"),
    white(whiteName), black(blackName), gameResult(GameResult::ONGOING)
{
}

std::string ChessPGN::formatTag(const std::string& name, const std::string& value) {
    return "[" + name + " \"" + value + "\"]\n";
}

std::string ChessPGN::resultToString(GameResult res) {
    switch(res) {
    case GameResult::WHITE_WINS: return "1-0";
    case GameResult::BLACK_WINS: return "0-1";
    case GameResult::DRAW: return "1/2-1/2";
    case GameResult::ONGOING: return "*";
    default: return "*";
    }
}

void ChessPGN::setWhiteWin() {
    gameResult = GameResult::WHITE_WINS;
}

void ChessPGN::setBlackWin() {
    gameResult = GameResult::BLACK_WINS;
}

void ChessPGN::setDraw() {
    gameResult = GameResult::DRAW;
}

void ChessPGN::addMove(const std::string& move) {
    moves.push_back(move);
}

//void ChessPGN::addMoveWithComment(const std::string& move, const std::string& comment) {
//    moves.push_back(move);
//    comments.push_back("{" + comment + "}");
//}

//void ChessPGN::addComment(const std::string& comment) {
//    comments.push_back("{" + comment + "}");
//}

std::string ChessPGN::formatMoves() {
    std::string ss;
    int moveNumber = 1;
//    int commentIndex = 0;
    for (size_t i = 0; i < moves.size(); i++) {
        // Add move number for white (odd moves)
        if (i % 2 == 0) {
            char str[8];
            sprintf(str, "%d. ", moveNumber++);
            ss.append(str);
        }
        ss.append(moves[i]);
        // Add comments if any
//        while (commentIndex < comments.size()) {
//            ss.append(" ").append(comments[commentIndex]);
//            commentIndex++;
//        }
        ss.append(" ");
        // Add line break every 10 moves for readability
        if (i % 10 == 9) {
            ss.append("\n");
        }
    }
    // Add result at the end
    ss.append(resultToString(gameResult));
    return ss;
}

std::string ChessPGN::generatePGN() {
    std::string pgn;
    // Required tags
    pgn.append(formatTag("Event", event));
    pgn.append(formatTag("Site", site));
    pgn.append(formatTag("Date", date));
    pgn.append(formatTag("Round", round));
    pgn.append(formatTag("White", white));
    pgn.append(formatTag("Black", black));
    pgn.append(formatTag("Result", resultToString(gameResult)));
    if (!fen.empty())
        pgn.append(formatTag("FEN", fen));
    pgn.append("\n");
    // Game moves
    pgn.append(formatMoves());
    pgn.append("\n\n");
    return pgn;
}

void ChessPGN::clear() {
    event = "?";
    site = "?";
    date = "?";
    round = "?";
    white = "?";
    black = "?";
    moves.clear();
//    comments.clear();
    gameResult = GameResult::ONGOING;
}
