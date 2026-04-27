/*
 * ChessEngine.h
 *
 *  Created on: Nov 8, 2025
 *      Author: alex
 */

#ifndef CHESSENGINE_H_
#define CHESSENGINE_H_

#include <string>
#include "ChessEngineInterface.h"
#include "ChessPGN.h"

#pragma pack(push, 1)
#ifndef STM32F103xB
static struct SettingsPGN {
#else
extern struct SettingsPGN {
#endif
    char event[32] = "Casual Game";
    char site[32] = "Iserlohn, NRW GER";
    short round = 1;
    char white[32] = "Player 1";
    char black[32] = "Player 2";
} settingsPGN;
#pragma pack(pop)

class ChessEngine : public ChessEngineInterface
{
private:
    bool isGame = false;
    EngineMode engineSide;
    ChessPGN* pgn;
    int searchDepth = 3;
    bool isLastMoveValid = true;
    unsigned int seed = 0;

    int moveNumber = 0;
    int fiftyMoveClock = 0;
    int epSquare = 0;      // square for en passant capture

    unsigned int startThink = -1;
    float whiteThinkTime = 0;
    float blackThinkTime = 0;

    const int maxNodeLimit = 64;
    int nodeLimit = 64;
    int kLast = BLACK;

    int k = BLACK;

    int M=136,S=128,I=8e3,Q=0,O=0,K=I,N=0,R=0,J=0,Z=0;

    char L = 0;
    const int w[8]={0,2,2,7,-1,8,12,23};                      /* relative piece values    */
    const int o[32]={-16,-15,-17,0,1,16,0,1,16,15,17,0,14,18,31,33,0, /* step-vector lists */
                       7,-1,11,6,8,3,6,                          /* 1st dir. in o[] per piece*/
                       6,3,5,7,4,5,3,6};                         /* initial piece setup      */
    char b[129] = {0};                                        /* board: half of 16x8+dummy*/
    int T[1035] = {0};                                       /* hash translation table   */

    char c[9] = {0};
    char cE[9] = {0};

    // backup variable
    short kBk = BLACK;
    int  NBk = 0, JBk = 0, ZBk = 0, LBk = 0, OBk = 0;
    char cEBk[9] = {0};
    char bBackup[129];

    int D(int q, int l, int e, int E, int z, int n);
    void init();
    void loop();

    bool isMoveValid();
    bool checkGameRule(int estimate);
    bool canCastle(int side, bool shortCastle);
    void updateEpSquare(const std::string& move);

    void makeBackup();
    void applyBackup();

    char getFigure(std::string field);
    std::string toCoord(int idx);
    std::string formatPGNMove(std::string move);
    std::string formatPGNMove(int from, int to, bool isCapture, int pieceType, bool isPromotion = false);
    void check50MoveRule();
    void updateFiftyMoveClock(int piece, bool isCapture);
public:
    ChessEngine(EngineMode side = BLACK, unsigned int seed = 0, int depth = 3);
    ~ChessEngine();

    void setDate(int y,int m,int d) override;
    void setFEN(const std::string& fen) override;
    const std::string getFEN() override;
    const std::string getPGN() override;
    bool isGameActive() override;
    Result getGameResult() override;
    bool isCastlingMove(const std::string& move, std::string& rockMove) override;
    bool checkMove(const std::string& move) override;
    std::string getHelp() override;
    void setMove(const std::string& move) override;
    std::string getMove() override;

    int getFigureCollor(const std::string& field) override;
    bool getIsLastMoveValid() override;
    int getCurrentSide() override;

    int getWinningSide() override;
    std::string getKingPosition(int color) override;
    std::string isCheck() override;

    std::string showBoard() override;

};

#endif /* CHESSENGINE_H_ */
