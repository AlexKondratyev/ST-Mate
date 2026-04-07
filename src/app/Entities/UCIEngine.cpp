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

    // Обработка опций
    if (name == "Hash") {
        // Размер хеш-таблицы (можно проигнорировать для STM32)
        // int hashSize = std::stoi(value);
    }
    else if (name == "Skill Level") {
        int level = std::stoi(value);
        // Уровень сложности (1-20)
        searchDepth = 2 + (level * 3) / 20; // маппинг 1-20 -> 2-5
    }
    else if (name == "Threads") {
        // Количество потоков (игнорируем, у нас 1 поток)
    }
}

void UCIEngine::parsePosition(const std::string& cmd) {
    std::stringstream ss(cmd);
    std::string token;
    ss >> token; // "position"

    currentMoves.clear();

    ss >> token;
    if (token == "startpos") {
        // Начальная позиция
        currentFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        ss >> token; // "moves" или конец
        if (token == "moves") {
            std::string move;
            while (ss >> move) {
                currentMoves.push_back(move);
            }
        }
    }
    else if (token == "fen") {
        // Парсим FEN (до 6 полей)
        std::string fen;
        int fieldCount = 0;
        while (ss >> token && fieldCount < 6) {
            if (fieldCount > 0) fen += " ";
            fen += token;
            fieldCount++;

            // Проверяем, не начались ли ходы
            if (fieldCount == 6 && ss.peek() == ' ') {
                std::string next;
                ss >> next;
                if (next == "moves") {
                    while (ss >> next) {
                        currentMoves.push_back(next);
                    }
                    break;
                } else {
                    // Это не "moves", значит это часть FEN (возможно, поле номер хода)
                    // Продолжаем парсить
                }
            }
        }
        currentFEN = fen;
    }

    // Применяем позицию к движку
    if (!currentFEN.empty()) {
        // engine->setFEN(currentFEN.c_str());
        // Применяем все ходы из списка
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

    // Останавливаем предыдущий поиск
    stop();
    if (searchThread.joinable()) searchThread.join();

    // Запускаем новый поиск
    stopSearch = false;
    searching = true;

    searchThread = std::thread([this]() {
        searchAndOutput();
    });
}

void UCIEngine::searchAndOutput() {
    // Определяем ограничение по времени
    auto startTime = std::chrono::steady_clock::now();
    int timeLimit = 0;

    if (searchMovetime > 0) {
        timeLimit = searchMovetime;
    } else if (searchWTime > 0 || searchBTime > 0) {
        // Определяем, чей ход
        bool whiteToMove = (currentFEN.find(" w ") != std::string::npos);
        int myTime = whiteToMove ? searchWTime : searchBTime;
        int myInc = whiteToMove ? searchWInc : searchBInc;

        // Вычисляем время на ход (примерная формула)
        int movesLeft = searchMovesToGo > 0 ? searchMovesToGo : 40;
        timeLimit = myTime / movesLeft + myInc / 2;
        if (timeLimit < 100) timeLimit = 100; // минимум 0.1 секунды
        if (timeLimit > 5000) timeLimit = 5000; // максимум 5 секунд
    }

    // Адаптируем глубину под время
    int maxDepth = searchDepth;
    if (timeLimit > 0) {
        // Приблизительная эвристика: 100 мс на глубину
        maxDepth = std::min(searchDepth, (int)(timeLimit / 100) + 2);
        if (maxDepth < 2) maxDepth = 2;
        if (maxDepth > 6) maxDepth = 6; // ограничение для STM32
    }

    // engine->setDepth(maxDepth);

    // Получаем лучший ход
    std::string bestMove = engine->getHelp();

    // Если нужно, получаем оценку позиции
    // int evaluation = engine->getEvaluation();

    // Вычисляем затраченное время
    auto endTime = std::chrono::steady_clock::now();
    int elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    // Отправляем информацию (опционально)
    // sendInfo(maxDepth, evaluation, bestMove, 0, elapsedMs);

    // Отправляем результат
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

    // UCI идентификация
    std::cout << "id name ST-Mate 1.0" << std::endl;
    std::cout << "id author Zaharov (based on micro-Max by H.G. Muller)" << std::endl;
    std::cout << "option name Hash type spin default 1 min 1 max 16" << std::endl;
    std::cout << "option name Skill Level type spin default 10 min 0 max 20" << std::endl;
    std::cout << "uciok" << std::endl;
    std::cout.flush();

    while (std::getline(std::cin, line)) {

        std::ofstream log("logfile.txt", std::ios_base::app | std::ios_base::out);
        log << line.c_str() << std::endl;
        // Удаляем пробелы в конце
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
            // Начинаем новую игру
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
