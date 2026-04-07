#ifndef UCIENGINE_H_
#define UCIENGINE_H_

#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include "ChessEngine.h"

class UCIEngine {
private:
    ChessEngine* engine;
    std::thread searchThread;
    std::atomic<bool> stopSearch;
    std::atomic<bool> searching;
    std::atomic<bool> quitFlag;

    // Параметры поиска
    int searchDepth;
    int searchMovetime;
    int searchWTime, searchBTime;
    int searchWInc, searchBInc;
    int searchMovesToGo;

    // Текущая позиция
    std::string currentFEN;
    std::vector<std::string> currentMoves;

    void parsePosition(const std::string& cmd);
    void parseGo(const std::string& cmd);
    void parseSetOption(const std::string& cmd);
    void searchAndOutput();
    void sendBestMove(const std::string& move);
    void sendInfo(int depth, int score, const std::string& pv, int nodes = 0, int time = 0);
    void sendReady();

public:
    UCIEngine();
    ~UCIEngine();

    void loop();           // основной цикл UCI
    void stop();           // остановка поиска
    bool isSearching() const { return searching; }
};

#endif
