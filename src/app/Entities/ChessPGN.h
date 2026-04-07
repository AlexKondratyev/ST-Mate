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
    // PGN теги
    std::string event;      // Название турнира или события
    std::string site;       // Место проведения
    std::string date;       // Дата в формате YYYY.MM.DD
    std::string round;      // Номер тура
    std::string white;      // Имя игрока белыми
    std::string black;      // Имя игрока черными
    std::string result;     // Результат ("1-0", "0-1", "1/2-1/2", "*")
    std::string fen;       // FEN

    // Ходы игры
    std::vector<std::string> moves;
    // Комментарии
//    std::vector<std::string> comments;
    // Результат игры
    GameResult gameResult;

    // Вспомогательные функции
    std::string formatTag(const std::string& name, const std::string& value);
    std::string formatMoves();
    std::string resultToString(GameResult res);

public:
    // Конструкторы
    ChessPGN();
    ChessPGN(const std::string& whiteName, const std::string& blackName);

    // Сеттеры для тегов
    void setEvent(const std::string& e) { event = e; }
    void setSite(const std::string& s) { site = s; }
    void setDate(const std::string& d) { date = d; }
    void setRound(const std::string& r) { round = r; }
    void setWhite(const std::string& w) { white = w; }
    void setBlack(const std::string& b) { black = b; }
    void setResult(GameResult res) { gameResult = res; }
    void setFEN(const std::string& f) { fen = f; }

    // Добавление ходов
    void addMove(const std::string& move);
//    void addMoveWithComment(const std::string& move, const std::string& comment);
//    void addComment(const std::string& comment);

    // Установка результата
    void setWhiteWin();
    void setBlackWin();
    void setDraw();

    // Генерация PGN строки
    std::string generatePGN();

    // Очистка
    void clear();

    // Геттеры
    std::string getResult() const { return result; }
    GameResult getResultEnum() const { return gameResult; }
    std::vector<std::string> getMoves() const { return moves; }
};


#endif // CHESSPGN_H
