#include "UCIEngine.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <cmath>

UCIEngine::UCIEngine()
    : stopSearch(false)
    , searching(false)
    , quitFlag(false)
    , searchDepth(3)
    , searchMovetime(0)
    , searchWTime(0)
    , searchBTime(0)
    , searchWInc(0)
    , searchBInc(0)
    , searchMovesToGo(0)
{
    engine = new ChessEngine(PVP, 0, searchDepth);
}

UCIEngine::~UCIEngine() {
    stop();
    if (searchThread.joinable()) searchThread.join();
    delete engine;
}

void UCIEngine::sendBestMove(const std::string& move) {
    std::cout << "bestmove " << move << std::endl;
    std::cout.flush();
}

void UCIEngine::sendInfo(int depth, int score, const std::string& pv, int nodes, int time) {
    std::cout << "info depth " << depth
              << " score cp " << score
              << " nodes " << nodes
              << " time " << time
              << " pv " << pv << std::endl;
    std::cout.flush();
}

void UCIEngine::sendReady() {
    std::cout << "readyok" << std::endl;
    std::cout.flush();
}

void UCIEngine::parseSetOption(const std::string& cmd) {
    std::stringstream ss(cmd);
    std::string token;
    ss >> token; // "setoption"
    ss >> token; // "name"

    std::string name;
    while (ss >> token && token != "value") {
        if (!name.empty()) name += " ";
        name += token;
    }

    std::string value;
    while (ss >> token) {
        if (!value.empty()) value += " ";
        value += token;
    }

    // Process options
    if (name == "Hash") {
        // Hash table size (can ignore for STM32)
        // int hashSize = std::stoi(value);
    }
    else if (name == "Skill Level") {
        int level = std::stoi(value);
        // Skill level (1-20)
        searchDepth = 2 + (level * 3) / 20; // mapping 1-20 -> 2-5
    }
    else if (name == "Threads") {
        // Number of threads (ignored, we have 1 thread)
    }
}

void UCIEngine::parsePosition(const std::string& cmd) {
    std::stringstream ss(cmd);
    std::string token;
    ss >> token; // "position"

    currentMoves.clear();

    ss >> token;
    if (token == "startpos") {
        // Starting position
        currentFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        ss >> token; // "moves" or end
        if (token == "moves") {
            std::string move;
            while (ss >> move) {
                currentMoves.push_back(move);
            }
        }
    }
    else if (token == "fen") {
        // Parse FEN (up to 6 fields)
        std::string fen;
        int fieldCount = 0;
        while (ss >> token && fieldCount < 6) {
            if (fieldCount > 0) fen += " ";
            fen += token;
            fieldCount++;

            // Check whether moves have started
            if (fieldCount == 6 && ss.peek() == ' ') {
                std::string next;
                ss >> next;
                if (next == "moves") {
                    while (ss >> next) {
                        currentMoves.push_back(next);
                    }
                    break;
                } else {
                    // This is not "moves", so it may be part of FEN (possibly the move number field)
                    // Continue parsing
                }
            }
        }
        currentFEN = fen;
    }

    // Apply position to engine
    if (!currentFEN.empty()) {
        // engine->setFEN(currentFEN.c_str());
        // Apply all moves from the list
        for (const auto& move : currentMoves) {
            engine->setMove(move);
        }
    }
}

void UCIEngine::parseGo(const std::string& cmd) {
    std::stringstream ss(cmd);
    std::string token;
    ss >> token; // "go"

    searchDepth = 4;
    searchMovetime = 0;
    searchWTime = 0;
    searchBTime = 0;
    searchWInc = 0;
    searchBInc = 0;
    searchMovesToGo = 0;
    bool infinite = false;

    while (ss >> token) {
        if (token == "depth") {
            ss >> searchDepth;
        } else if (token == "movetime") {
            ss >> searchMovetime;
        } else if (token == "wtime") {
            ss >> searchWTime;
        } else if (token == "btime") {
            ss >> searchBTime;
        } else if (token == "winc") {
            ss >> searchWInc;
        } else if (token == "binc") {
            ss >> searchBInc;
        } else if (token == "movestogo") {
            ss >> searchMovesToGo;
        } else if (token == "infinite") {
            infinite = true;
        }
    }

    // Stop the previous search
    stop();
    if (searchThread.joinable()) searchThread.join();

    // Start a new search
    stopSearch = false;
    searching = true;

    searchThread = std::thread([this]() {
        searchAndOutput();
    });
}

void UCIEngine::searchAndOutput() {
    // Determine the time limit
    auto startTime = std::chrono::steady_clock::now();
    int timeLimit = 0;

    if (searchMovetime > 0) {
        timeLimit = searchMovetime;
    } else if (searchWTime > 0 || searchBTime > 0) {
        // Determine whose move it is
        bool whiteToMove = (currentFEN.find(" w ") != std::string::npos);
        int myTime = whiteToMove ? searchWTime : searchBTime;
        int myInc = whiteToMove ? searchWInc : searchBInc;

        // Calculate time per move (approximate formula)
        int movesLeft = searchMovesToGo > 0 ? searchMovesToGo : 40;
        timeLimit = myTime / movesLeft + myInc / 2;
        if (timeLimit < 100) timeLimit = 100; // minimum 0.1 seconds
        if (timeLimit > 5000) timeLimit = 5000; // maximum 5 seconds
    }

    // Adapt depth to the time
    int maxDepth = searchDepth;
    if (timeLimit > 0) {
        // Rough heuristic: 100 ms per depth
        maxDepth = std::min(searchDepth, (int)(timeLimit / 100) + 2);
        if (maxDepth < 2) maxDepth = 2;
        if (maxDepth > 6) maxDepth = 6; // limit for STM32
    }

    // engine->setDepth(maxDepth);

    // Get the best move
    std::string bestMove = engine->getHelp();

    // If needed, get the position evaluation
    // int evaluation = engine->getEvaluation();

    // Calculate elapsed time
    auto endTime = std::chrono::steady_clock::now();
    int elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    // Send info (optional)
    // sendInfo(maxDepth, evaluation, bestMove, 0, elapsedMs);

    // Send the result
    sendBestMove(bestMove);

    searching = false;
}

void UCIEngine::stop() {
    if (searching) {
        stopSearch = true;
        if (searchThread.joinable()) {
            searchThread.join();
        }
    }
}
#include  <fstream>
void UCIEngine::loop() {
    std::string line;

    // UCI identification
    std::cout << "id name ST-Mate 1.0" << std::endl;
    std::cout << "id author Zaharov (based on micro-Max by H.G. Muller)" << std::endl;
    std::cout << "option name Hash type spin default 1 min 1 max 16" << std::endl;
    std::cout << "option name Skill Level type spin default 10 min 0 max 20" << std::endl;
    std::cout << "uciok" << std::endl;
    std::cout.flush();

    while (std::getline(std::cin, line)) {

        std::ofstream log("logfile.txt", std::ios_base::app | std::ios_base::out);
        log << line.c_str() << std::endl;
        // Trim trailing spaces
        line.erase(line.find_last_not_of(" \n\r\t") + 1);

        if (line.empty()) continue;

        if (line == "uci") {
            std::cout << "id name ST-Mate 1.0" << std::endl;
            std::cout << "id author Zaharov" << std::endl;
            std::cout << "uciok" << std::endl;
            std::cout.flush();
        }
        else if (line == "isready") {
            sendReady();
        }
        else if (line == "ucinewgame") {
            // Start a new game
            delete engine;
            engine = new ChessEngine(PVP, 0, searchDepth);
            // engine->setFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            currentMoves.clear();
        }
        else if (line.find("position") == 0) {
            parsePosition(line);
        }
        else if (line.find("go") == 0) {
            parseGo(line);
        }
        else if (line == "stop") {
            stop();
        }
        else if (line.find("setoption") == 0) {
            parseSetOption(line);
        }
        else if (line == "quit") {
            quitFlag = true;
            stop();
            break;
        }
    }
}
