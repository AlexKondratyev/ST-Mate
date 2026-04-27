#ifndef CHESSPGN_H
#define CHESSPGN_H

#include <string>
#include <vector>

enum class GameResult {
    WHITE_WINS,
    BLACK_WINS,
    DRAW,
    ONGOING
};

class ChessPGN {
private:
    // PGN tags
    std::string event;      // Tournament or event name
    std::string site;       // Venue
    std::string date;       // Date in YYYY.MM.DD format
    std::string round;      // Round number
    std::string white;      // White player name
    std::string black;      // Black player name
    std::string result;     // Result ("1-0", "0-1", "1/2-1/2", "*")
    std::string fen;       // FEN

    // Game moves
    std::vector<std::string> moves;
    // Comments
//    std::vector<std::string> comments;
    // Game result
    GameResult gameResult;

    // Helper functions
    std::string formatTag(const std::string& name, const std::string& value);
    std::string formatMoves();
    std::string resultToString(GameResult res);

public:
    // Constructors
    ChessPGN();
    ChessPGN(const std::string& whiteName, const std::string& blackName);

    // Setters for tags
    void setEvent(const std::string& e) { event = e; }
    void setSite(const std::string& s) { site = s; }
    void setDate(const std::string& d) { date = d; }
    void setRound(const std::string& r) { round = r; }
    void setWhite(const std::string& w) { white = w; }
    void setBlack(const std::string& b) { black = b; }
    void setResult(GameResult res) { gameResult = res; }
    void setFEN(const std::string& f) { fen = f; }

    // Add moves
    void addMove(const std::string& move);
//    void addMoveWithComment(const std::string& move, const std::string& comment);
//    void addComment(const std::string& comment);

    // Set result
    void setWhiteWin();
    void setBlackWin();
    void setDraw();

    // Generate PGN string
    std::string generatePGN();

    // Clear
    void clear();

    // Getters
    std::string getResult() const { return result; }
    GameResult getResultEnum() const { return gameResult; }
    std::vector<std::string> getMoves() const { return moves; }
};


#endif // CHESSPGN_H
