#include <iostream>

using namespace std;

#include "Converter.h"
#include "ChessEngine.h"
#include "UCIEngine.h"
#include <QDebug>
#include <QThread>
#include <QElapsedTimer>

QElapsedTimer timer;
int playCount = 100;
ChessEngineInterface* ew; // Движок играет за белых
ChessEngineInterface* eb; // Движок играет за чёрных

void fullTest()
{
    qDebug() << "=====Подсказка за белых=====";
    eb = new ChessEngine(BLACK, 0);
    try {
        std::string helpMove = eb->getHelp();
        if (helpMove != "e2e4") throw helpMove;
    } catch (...) {
        qDebug() << "not pass";
    }
    delete eb;

    qDebug() << "=====Детский мат за чёрных=====";
    eb = new ChessEngine(PVP);
    timer.start();
    eb->setMove("g2g4");    eb->setMove("e7e5");
    eb->setMove("f2f3");    eb->setMove("d8h4");
    qDebug() << "Time: " <<  timer.nsecsElapsed()/1e6 << "ms.";
    try {
        if (eb->getGameResult() != BLACK_WINS) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    delete eb;

    qDebug() << "=====Детский мат за белых=====";
    eb = new ChessEngine(PVP);
    timer.start();
    eb->setMove("e2e4");    eb->setMove("e7e5");
    eb->setMove("f1c4");    eb->setMove("f8c5");
    eb->setMove("d1f3");    eb->setMove("b8c6");
    eb->setMove("f3f7");
    qDebug() << "Time: " <<  timer.nsecsElapsed()/1e6 << "ms.";
    try {
        if (eb->getGameResult() != WHITE_WINS) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    delete eb;

    qDebug() << "=====Пат=====";
    eb = new ChessEngine(PVP);      // Пат
    timer.start();
    eb->setMove("e2e3"); eb->setMove("a7a5");
    eb->setMove("d1h5"); eb->setMove("a8a6");
    eb->setMove("h5a5"); eb->setMove("h7h5");
    eb->setMove("a5c7"); eb->setMove("a6h6");
    eb->setMove("h2h4"); eb->setMove("f7f6");
    eb->setMove("c7d7"); eb->setMove("e8f7");
    eb->setMove("d7b7"); eb->setMove("d8d3");
    eb->setMove("b7b8"); eb->setMove("d3h7");
    eb->setMove("b8c8"); eb->setMove("f7g6");
    eb->setMove("c8e6");
    qDebug() << "Time: " <<  timer.nsecsElapsed()/1e6 << "ms.";
    try {
        if (eb->getGameResult() != DRAW) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    delete eb;

    qDebug() << "=====Проверка связывания белого ферзя=====";
    eb = new ChessEngine(PVP);
    timer.start();
    eb->setMove("d2d4"); eb->setMove("c7c6");
    eb->setMove("d1d2"); eb->setMove("d8a5");
    try {
        eb->setMove("d2d3"); // not valid
        if (eb->getIsLastMoveValid()) throw;
        eb->setMove("d2b4"); // valid
        if (!eb->getIsLastMoveValid()) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    qDebug() << "Time: " <<  timer.nsecsElapsed()/1e6 << "ms.";
    delete eb;


    qDebug() << "=====set FEN Test=====";
    eb = new ChessEngine(PVP);
    eb->setFEN("rnbqkbnr/pppp1ppp/4p3/8/6P1/5P2/PPPPP2P/RNBQKBNR b KQkq - 0 1");
    std::string move = eb->getHelp();
    try {
        eb->setMove("d8h4"); // valid
        if (!eb->getIsLastMoveValid()) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    delete eb;

    qDebug() << "===== get FEN Test=====";
    eb = new ChessEngine(PVP);
    eb->setMove("e2e4");    eb->setMove("e7e5");
    eb->setMove("f1c4");    eb->setMove("f8c5");
    eb->setMove("d1f3");    eb->setMove("b8c6");
    try {
        auto fen = eb->getFEN();
        if (fen != "r1bqk1nr/pppp1ppp/2n5/2b1p3/2B1P3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 4 4") throw; // счётчик ходов  не тикает
    } catch(...) {
        qDebug() << "not pass";
    }
    delete eb;

    qDebug() << "=====Check Promotion Test=====";
    eb = new ChessEngine(BLACK);
    eb->setFEN("8/3k3P/8/8/8/8/2p5/4K3 w - - 0 1");
    eb->setMove("h7h8"); // add =Q
    delete eb;

    qDebug() << "=====Casteling Test=====";
    eb = new ChessEngine(PVP);
    eb->setFEN("r3k2r/pbpqppbp/np1p2pn/8/8/NP1P2PN/PBPQPPBP/R3K2R w KQkq - 0 1");
    try {
        bool isAllow = eb->checkMove("e1f1");
        qDebug() << "Move allow?:" << isAllow;
        if (!isAllow) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    try {
        bool isAllow = eb->checkMove("e1g1");
        qDebug() << "Casteling allow?:" << isAllow;
        if (!isAllow) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    try {
        bool isAllow = eb->checkMove("e1c1");
        qDebug() << "Casteling allow?:" << isAllow;
        if (!isAllow) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    try {
        eb->setMove("e1c1"); // valid
        if (!eb->getIsLastMoveValid()) throw;
    } catch(...) {
        qDebug() << "not pass";
    }

    try {
        bool isAllow = eb->checkMove("e8g8");
        qDebug() << "Move allow?:" << isAllow;
        if (!isAllow) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    try {
        bool isAllow = eb->checkMove("e8c8");
        qDebug() << "Casteling allow?:" << isAllow;
        if (!isAllow) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    try {
        eb->setMove("e8c8"); // valid
        if (!eb->getIsLastMoveValid()) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    // qDebug() << eb->showBoard().c_str();
    delete eb;

    qDebug() << "=====Rock avalible move Test=====";
    eb = new ChessEngine(PVP);
    eb->setFEN("r3k2r/8/8/8/8/8/8/R3K2R w - - 0 1");

    try {
        bool isAllow = eb->checkMove("h1g1");
        qDebug() << "check rock move allow?:" << isAllow;
        if (!isAllow) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    try {
        bool isAllow = eb->checkMove("h1f1");
        qDebug() << "check rock move allow?:" << isAllow;
        if (!isAllow) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    // qDebug() << eb->showBoard().c_str();
    delete eb;

    qDebug() << "=====Move King and castling=====";
    eb = new ChessEngine(PVP);
    eb->setFEN("r3k2r/ppp2ppp/2nq1n2/2bppb2/2BPPB2/2NQ1N2/PPP2PPP/R3K2R w KQkq - 6 8");

    try {
        eb->setMove("e1d2"); // move w king
        if (!eb->getIsLastMoveValid()) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    try {
        eb->setMove("e8c8"); // black make castling
        if (!eb->getIsLastMoveValid()) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    try {
        eb->setMove("d2e1"); // move w king back
        if (!eb->getIsLastMoveValid()) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    try {
        eb->setMove("a7a6"); // move b king back
        if (!eb->getIsLastMoveValid()) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    try {
        bool isAllow = eb->checkMove("e1g1");
        if (isAllow) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    try {
        eb->setMove("e1g1"); // make castling, invalid move
        if (eb->getIsLastMoveValid()) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    delete eb;

    qDebug() << "=====en passant Test=====";
    eb = new ChessEngine(PVP);
    eb->setMove("e2e4"); eb->setMove("b8c6");
    eb->setMove("e4e5"); eb->setMove("d7d5");
    try {
        bool isAllow = eb->checkMove("e5d6");
        qDebug() << "e5d6 en passant allow?  must true:" <<  isAllow;
        if (!isAllow) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    eb->setMove("e5d6"); eb->setMove("a7a6");
    delete eb;

    qDebug() << "=====invalid en passant Test=====";
    eb = new ChessEngine(PVP);
    eb->setFEN("r3kb1r/pp1n1ppp/1q2pn2/2pP4/2P5/5N2/PP1PQPPP/RNB1K2R w KQkq - 2 8");
    eb->setMove("e1g1"); // тут рокеровка
    eb->setMove("e8c8");  // тут рокеровка
    try {
        bool isAllow = eb->checkMove("d5c6");
        qDebug() << "check pawn move allow? must false:" << isAllow; // тут нельзя сделать взятие на проходе, до этого была рокировка
        if (isAllow) throw;
    } catch(...) {
        qDebug() << "not pass";
    }

    try {
        bool isAllow = eb->checkMove("d5d6");
        qDebug() << "check pawn move allow? must true:" << isAllow; // тут нельзя сделать взятие на проходе, до этого была рокировка
        if (!isAllow) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    try {
        bool isAllow = eb->checkMove("d5e6");
        qDebug() << "check pawn move allow? must true:" << isAllow; // тут нельзя сделать взятие на проходе, до этого была рокировка
        if (!isAllow) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
    delete eb;

    qDebug() << "=====Time test=====";
    float time = 0;
    const int rounds = 50;
    for (int tests = 0; tests < rounds; ++tests) {
        eb = new ChessEngine(PVP, 0, 3);
        timer.start();
        eb->setMove("b1c3"); for (int i = 0; i < 64; i++) eb->checkMove("b1"+Converter::convert(i));
        eb->setMove("b8c6"); for (int i = 0; i < 64; i++) eb->checkMove("b8"+Converter::convert(i));
        eb->setMove("g1f3"); for (int i = 0; i < 64; i++) eb->checkMove("g1"+Converter::convert(i));
        eb->setMove("g8f6"); for (int i = 0; i < 64; i++) eb->checkMove("g8"+Converter::convert(i));
        eb->setMove("d2d4"); for (int i = 0; i < 64; i++) eb->checkMove("d2"+Converter::convert(i));
        eb->setMove("d7d5"); for (int i = 0; i < 64; i++) eb->checkMove("d7"+Converter::convert(i));
        eb->setMove("e2e4"); for (int i = 0; i < 64; i++) eb->checkMove("e2"+Converter::convert(i));
        eb->setMove("e7e5"); for (int i = 0; i < 64; i++) eb->checkMove("e7"+Converter::convert(i));
        eb->setMove("f1c4"); for (int i = 0; i < 64; i++) eb->checkMove("f1"+Converter::convert(i));
        eb->setMove("f8c5"); for (int i = 0; i < 64; i++) eb->checkMove("f8"+Converter::convert(i));
        eb->setMove("c1f4"); for (int i = 0; i < 64; i++) eb->checkMove("c1"+Converter::convert(i));
        eb->setMove("c8f5"); for (int i = 0; i < 64; i++) eb->checkMove("c8"+Converter::convert(i));
        eb->setMove("d1d3"); for (int i = 0; i < 64; i++) eb->checkMove("d1"+Converter::convert(i));
        eb->setMove("d8d6"); for (int i = 0; i < 64; i++) eb->checkMove("d8"+Converter::convert(i));
        float gameTime = timer.elapsed()/1e3;
        qDebug() << "Game time: " << gameTime;
        time += gameTime;
        delete eb;
    }
    time = time / (float)rounds;
    qDebug() << "Time: " << time << "s.";
    try {
        if (time > 1) throw;
    } catch(...) {
        qDebug() << "not pass";
    }
}

int main(int argc, char* argv[])
{
    // Если запущен с аргументом "uci" или без аргументов, запускаем UCI режим
    if (argc == 2 && std::string(argv[1]) == "uci") {
        UCIEngine uci;
        uci.loop();
        return 0;
    }

    fullTest();

    // сам с собой в pvp
    playCount = 1;
    while(playCount--)
    {
        eb = new ChessEngine(PVP, playCount, 4);
        while(eb->isGameActive())
        {
            auto moveW = eb->getHelp();
            eb->setMove(moveW);

            auto moveB = eb->getHelp();
            eb->setMove(moveB);
        }
        cout << eb->showBoard() << endl; // вывод доски
        delete eb;
    }

    // сам с собой за чёрных
    playCount = 1;
    while(playCount--)
    {
        eb = new ChessEngine(BLACK, 25, 4);
        while(eb->isGameActive())
        {
            auto moveW = eb->getHelp();
            eb->setMove(moveW);
        }
        cout << eb->showBoard() << endl; // вывод доски
        delete eb;
    }
    return 0;

    // MANUAL GAME
    char user_move[5];
    eb = new ChessEngine(BLACK, 25, 4);   // Движок играет за чёрных
    cout << eb->showBoard() << endl; // вывод доски
    while(1)
    {
        cin >> user_move;   // ввод хода
        eb->setMove(user_move);
        cout << eb->showBoard() << endl; // вывод доски
    }

    return 0;
}
