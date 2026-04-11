/*
 * ChessEngineInterface.h
 *
 *  Created on: Feb 23, 2026
 *      Author: alex
 */

#ifndef ENTITIES_CHESSENGINEINTERFACE_H_
#define ENTITIES_CHESSENGINEINTERFACE_H_

enum EngineMode{ WHITE = 8, BLACK = 16, PVP = 32};    // side to move
enum Result {WHITE_WINS, BLACK_WINS, DRAW };

class ChessEngineInterface
{
public:
    virtual ~ChessEngineInterface() = default;
    virtual void setDate(int y,int m,int d) = 0;

    virtual void setFEN(const std::string& fen) = 0;
    virtual const std::string getFEN() = 0;
    virtual const std::string getPGN() = 0;
    virtual bool isGameActive() = 0;                // игра активна
    virtual Result getGameResult() = 0;

    virtual bool isCastlingMove(const std::string& move, std::string& rockMove) = 0;
    virtual bool checkMove(const std::string& move) = 0;   // проверка хода, вызов функции D для проверки
    virtual std::string getHelp() = 0;              // получить подсказку
    virtual void setMove(const std::string& move) = 0;     // ввод хода
    virtual std::string getMove() = 0;              // получить ход движка

    virtual int getFigureCollor(const std::string& field) = 0;
    virtual bool getIsLastMoveValid() = 0;
    virtual int getCurrentSide() = 0;

    virtual int getWinningSide() = 0;
    virtual std::string getKingPosition(int color) = 0;
    virtual std::string isCheck() = 0;

    virtual std::string showBoard() = 0;            // вывести доску в консоль
};


#endif /* ENTITIES_CHESSENGINEINTERFACE_H_ */
