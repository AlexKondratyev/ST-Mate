#include "ChessEngine.h"
#include "../../Drivers/timestamp.h"
#include <memory.h>

#define SHOW_MOVE true
#define SHOW_THINK_TIME true
#define SHOW_HELP_MOVE true
#define SHOW_ESTIMATE false
#define SHOW_BOARD false

#ifndef STM32F103xB
const char *sym[] = {                      // print unicode characters to represent pieces on board
    ".", "-",
    "\u2659", // WHITE CHESS PAWN
    "\u2658", // WHITE CHESS KNIGHT
    "\u2654", // WHITE CHESS KING
    "\u2657", // WHITE CHESS BISHOP
    "\u2656", // WHITE CHESS ROOK
    "\u2655", // WHITE CHESS QUEEN
    "-",
    "\u265F", // BLACK CHESS PAWN
    "-",
    "\u265E", // BLACK CHESS KNIGHT
    "\u265A", // BLACK CHESS KING
    "\u265D", // BLACK CHESS BISHOP
    "\u265C", // BLACK CHESS ROOK
    "\u265B", // BLACK CHESS QUEEN
};
#else
const char sym[17] = { ".-pnkbrq-P-NKBRQ" };
#endif

ChessEngine::ChessEngine(EngineMode side, unsigned int seed, int depth)
{
    engineSide = side;
    this->seed = seed;
    searchDepth = depth;
    epSquare = 0;

    switch (side) {
    case WHITE:
        pgn = new ChessPGN("micro-Max 4.8", settingsPGN.black);
        break;
    case BLACK:
        pgn = new ChessPGN(settingsPGN.white, "micro-Max 4.8");
        break;
    case PVP:
        pgn = new ChessPGN(settingsPGN.white, settingsPGN.black);
        break;
    }
    pgn->setEvent(settingsPGN.event);
    pgn->setSite(settingsPGN.site);
    pgn->setDate("2026.03.14");
    pgn->setRound(std::to_string(settingsPGN.round));

    init();

    // start game
    isGame = true;
    // if engine side white: make first move
    if(engineSide == WHITE)
    {
        c[0]='\n';
        K=I;                                            /* invalid move       */
        N=0;
        if(*c-10)K=*c-16*c[1]+799,L=c[2]-16*c[3]+799;   /* parse entered move */
        D(-I,I,Q,O,1,searchDepth);                                /* think or check & do*/
        moveNumber++;
        startThink = getCurrentTimeMs();
        kLast = k;
#if SHOW_MOVE
        printf("%d. %s\n", moveNumber, getMove().c_str());
#endif
        pgn->addMove(getMove());
    }
    else
    {
#if SHOW_BOARD
        printf("%s", showBoard().c_str());
#endif
    }
}

ChessEngine::~ChessEngine()
{
    delete pgn;
}

void ChessEngine::setDate(int y,int m,int d)
{
    char date[11];
    sprintf(date,"%4d.%02d.%02d", y,m,d);
    pgn->setDate(date);
}

void ChessEngine::loop()
{
    if (!isGame) return;

    // ==========================================
    // ХОД ЧЕЛОВЕКА
    // ==========================================
    // Сохраняем индексы ДО вызова D (пока доска не изменена)
    int fromIdx = c[0] - 16 * c[1] + 799;
    int toIdx   = c[2] - 16 * c[3] + 799;
    int pieceType = b[c[0] - 16 * c[1] + 799] & 7;
    bool isCapture = (b[c[2] - 16 * c[3] + 799] != 0);

    makeBackup();
    K=I;
    if(*c-10) K=*c-16*c[1]+799,L=c[2]-16*c[3]+799;
    int est = D(-I,I,Q,O,1,searchDepth);
    if(!isMoveValid()) // Если ход нелегальный, ждем другой
    {
        applyBackup();
        return;
    }
    // Ход легальный – обновляем счётчик 50 ходов
    updateFiftyMoveClock(pieceType, isCapture);  // ← используем ДО изменения доски
    updateEpSquare(c);
    std::string playerMove = formatPGNMove(fromIdx, toIdx, isCapture, pieceType);
    // Проверка, не поставил ли человек мат
    makeBackup();
    c[0]='\n'; K=I; N=0; // Сбрасываем N
    nodeLimit = 8;
    est = D(-I,I,Q,O,1, searchDepth);
    nodeLimit = maxNodeLimit;
#if SHOW_ESTIMATE
    printf("est: %d\n", est);
#endif
    applyBackup();

    // Проверка на Шах (+) и Мат (#)
    if (!(est > -I + 1)) playerMove += "#"; // Мат
    else if (!isCheck().empty()) playerMove += "+"; // Шах

    unsigned int elapsed = 0;
    if (startThink != -1) elapsed = getCurrentTimeMs() - startThink;
    startThink = getCurrentTimeMs();

    if (k == WHITE)
    {
        moveNumber++;
#if SHOW_MOVE
        printf("%d. %s\n", moveNumber, playerMove.c_str());
#endif
#if SHOW_THINK_TIME
        if (elapsed) // not first move
        {
            whiteThinkTime += elapsed / 1000.0;
            printf("White think time: %.3f s\n", elapsed / 1000.0);
        }
#endif
    } else {
#if SHOW_MOVE
        printf("%d. %s\n", moveNumber, playerMove.c_str());
#endif
#if SHOW_THINK_TIME
        if (elapsed) // not first move
        {
            blackThinkTime += elapsed / 1000.0;
            printf("Black think time: %.3f s\n", elapsed / 1000.0);
        }
#endif

    }
    pgn->addMove(playerMove);
#if SHOW_BOARD
    printf("%s", showBoard().c_str());
#endif

    if(checkGameRule(est)) return;

    if (engineSide == PVP) return; // Если это режим наблюдения/PVP, движок сам не делает ответный ход

    // ==========================================
    // ХОД ДВИЖКА
    // ==========================================
    makeBackup();  // сохраняем доску перед поиском
    startThink = getCurrentTimeMs();
    c[0]='\n'; // Очищаем буфер, чтобы движок искал лучший ход
    K=I;
    N=0; // КРИТИЧЕСКИ ВАЖНО: сброс лимита узлов перед поиском!
    est = D(-I,I,Q,O,1,searchDepth);
    isMoveValid(); // Синхронизируем флаг последнего хода

    // Получаем from, to из cE
    int fromIdxE = cE[0] - 16 * cE[1] + 799;
    int toIdxE   = cE[2] - 16 * cE[3] + 799;
    // Из сохранённой доски (bBackup) получаем тип фигуры и было ли взятие
    int pieceTypeE = bBackup[fromIdxE] & 7;
    bool isCaptureE = (bBackup[toIdxE] != 0);
    updateEpSquare(cE);
    // Форматируем ход
    std::string engineMove = formatPGNMove(fromIdxE, toIdxE, isCaptureE, pieceTypeE);
    // Ход легальный – обновляем счётчик 50 ходов
    updateFiftyMoveClock(pieceType, isCapture);  // ← используем ДО изменения доски

    // Проверка на мат после хода движка
    makeBackup();
    c[0]='\n'; K=I; N=0;
    nodeLimit = 8;
    est = D(-I, I, Q, O, 1, searchDepth);
    nodeLimit = maxNodeLimit;
#if SHOW_ESTIMATE
    printf("est: %d\n", est);
#endif
    applyBackup();

    // Проверка на Шах (+) и Мат (#)
    if (!(est > -I + 1)) engineMove += "#"; // Мат
    else if (!isCheck().empty()) engineMove += "+"; // Шах

    elapsed = getCurrentTimeMs() - startThink;
    startThink = getCurrentTimeMs();
    // Вывод хода движка
    if (k == WHITE)
    {
        moveNumber++;
#if SHOW_MOVE
        printf("%d. %s\n", moveNumber, engineMove.c_str());
#endif
    } else {
#if SHOW_MOVE
        printf("%d. %s\n", moveNumber, engineMove.c_str());
#endif
    }
    pgn->addMove(engineMove);
#if SHOW_BOARD
    printf("%s", showBoard().c_str());
#endif
#if SHOW_THINK_TIME
    printf("Engine think time: %.3f s\n", elapsed / 1000.0);
#endif

    checkGameRule(est);
}

const std::string ChessEngine::getPGN()
{
	return pgn->generatePGN();
}

bool ChessEngine::isGameActive()
{
    return isGame;
}

Result ChessEngine::getGameResult()
{
    return (Result)pgn->getResultEnum();
}

bool ChessEngine::isCastlingMove(const std::string& move, std::string& rockMove)
{
    int t = move[2] - 16 * move[3] + 799;
    int pieceType = b[t] & 7;
    if (pieceType != 4) // king?
        return false;

    // Король белых
    if (move == "e1g1")
    {
        rockMove = "h1f1";
        return true;
    }
    if (move == "e1c1")
    {
        rockMove = "a1d1";
        return true;
    }
    // Король черных
    if (move == "e8g8")
    {
        rockMove = "h8f8";
        return true;
    }
    if (move == "e8c8")
    {
        rockMove = "a8d8";
        return true;
    }
    return false;
}

bool ChessEngine::checkMove(const std::string& move)
{
    if (move.length() < 4) return false;

    int f = move[0] - 16 * move[1] + 799;
    int t = move[2] - 16 * move[3] + 799;

    int pieceFull = b[f];
    int pieceType = pieceFull & 7;
    int pieceColor = pieceFull & 24;

    // Если клетка пуста или пытаемся съесть свою фигуру
    if (!pieceColor) return false;
    if ((b[t] & 24) == pieceColor) return false;

    bool physicallyPossible = false;
    bool isCastling = false;
    int diff = t - f;

    // ==========================================
    // ИСПРАВЛЕНИЕ: Вынесли dir на общий уровень!
    // ==========================================
    int dir = (pieceColor == 16) ? 16 : -16;

    // 1. БЫСТРЫЙ ФИЛЬТР ГЕОМЕТРИИ (Без поиска D)
    if (pieceType < 3) { // пешка
        int startRank = (pieceColor == 16) ? 1 : 6;

        if (diff == dir && b[t] == 0) {
            physicallyPossible = true;
        }
        else if (diff == 2 * dir && (f >> 4) == startRank && b[t] == 0 && b[f+dir] == 0) {
            physicallyPossible = true;
        }
        else if (std::abs(diff - dir) == 1) {
            if (b[t] != 0) {
                physicallyPossible = true;
            }
            else if (t == epSquare) { // взятие на проходе
                int epPawnIdx = t - dir; // вражеская пешка находится позади
                if (epPawnIdx >= 0 && epPawnIdx < 128 && !(epPawnIdx & 0x88)) {
                    if (b[epPawnIdx] != 0 && (b[epPawnIdx] & 7) < 3 && (b[epPawnIdx] & 24) != pieceColor) {
                        physicallyPossible = true;
                    }
                }
            }
        }
    }
    else { // ОСТАЛЬНЫЕ ФИГУРЫ
        int j = o[pieceType + 16];
        int r = pieceType;

        while (true) {
            r = (pieceType > 2 && r < 0) ? -r : -o[++j];
            if (!r) break;

            if (diff % r == 0 && ((diff > 0 && r > 0) || (diff < 0 && r < 0))) {
                if (pieceType == 3 || pieceType == 4) { // Конь или Король
                    if (diff == r) physicallyPossible = true;
                    // Проверка рокировки
                    if (pieceType == 4) {
                        if (f == 116) {
                            if (t == 118) physicallyPossible = canCastle(8, true);
                            if (t == 114) physicallyPossible = canCastle(8, false);
                        } else if (f == 4) {
                            if (t == 6) physicallyPossible = canCastle(16, true);
                            if (t == 2) physicallyPossible = canCastle(16, false);
                        }
                        if (physicallyPossible && std::abs(diff) == 2) isCastling = true;
                    }
                } else { // Слон, Ладья, Ферзь
                    int p = f + r;
                    while (p != t && b[p] == 0 && !(p & 0x88)) p += r;
                    if (p == t) physicallyPossible = true;
                }
            }
            if (physicallyPossible) break;
        }
    }

    // Если геометрически ход невозможен - отсекаем сразу
    if (!physicallyPossible && !isCastling) return false;

    // ==========================================
    // 2. БЫСТРАЯ ПРОВЕРКА НА ШАХ (Виртуальный ход)
    // ==========================================
    char backupT = b[t];
    char backupF = b[f];

    // Делаем виртуальный ход
    b[t] = b[f];
    b[f] = 0;

    int epCaptureIdx = -1;
    char epBackup = 0;

    // Это и есть то самое временное удаление вражеской пешки при En Passant
    if (pieceType < 3 && std::abs(diff - dir) == 1 && backupT == 0 && t == epSquare) {
        epCaptureIdx = t - dir;   // ← возвращаем t - dir
        epBackup = b[epCaptureIdx];
        b[epCaptureIdx] = 0;
    }

    // Проверяем шахи
    int kBackup = k;
    k = 24 - pieceColor;
    bool inCheck = !isCheck().empty();
    k = kBackup;

    // Откат виртуального хода
    b[f] = backupF;
    b[t] = backupT;
    if (epCaptureIdx != -1) b[epCaptureIdx] = epBackup; // Возвращаем врага на место

    return !inCheck;
}

std::string ChessEngine::getHelp()
{
    makeBackup();
    c[0]='\n';
    K=I;                                            /* invalid move       */
    N=0;
    if(*c-10)K=*c-16*c[1]+799,L=c[2]-16*c[3]+799;
    int est = D(-I, I, Q, O, 1, searchDepth); /* Check & do the human movement */
#if SHOW_ESTIMATE
    printf("est: %d\n", est);
#endif
    std::string helpMove = cE;
    applyBackup();

#if SHOW_HELP_MOVE
    printf("Help move: %s\n", helpMove.c_str());
#endif
    return helpMove;
}

void ChessEngine::setMove(const std::string& move)
{
    if (move.size() < 4)
        return;
    memcpy(c, move.data(), move.length());
    loop();
}

std::string ChessEngine::getMove()
{
    return cE;
}

bool ChessEngine::getIsLastMoveValid()
{
    return isLastMoveValid;
}

int ChessEngine::getCurrentSide()
{
    return 24-k;
}

int ChessEngine::getWinningSide()
{
    return k;
}

std::string ChessEngine::getKingPosition(int color)
{
    // color должен быть либо WHITE (8), либо BLACK (16)
    for (int i = 0; i < 8; i++) // i - это номер горизонтали (0 для '1', 7 для '8')
    {
        for (int j = 0; j < 8; j++) // j - это номер вертикали (0 для 'a', 7 для 'h')
        {
            // Формула индексации должна совпадать с getFigure:
            int idx = 16 * (7 - i) + j;

            // (b[idx] & color) проверяет принадлежность стороне
            // (b[idx] & 7) извлекает тип фигуры. Король — это 4.
            if ((b[idx] & color) && (b[idx] & 7) == 4)
            {
                std::string pos = "";
                pos += (char)('a' + j);
                pos += (char)('1' + i);
                return pos;
            }
        }
    }
    return "";
}

std::string ChessEngine::isCheck()
{
    int kingPos = -1;
    int side = 24 - k;          // Сторона, которую проверяем (чей сейчас ход)
    int opponent =  k; // Противник (8 для белых, 16 для черных)

    // 1. Находим позицию короля текущего игрока
    for (int i = 0; i < 128; i++)
    {
        // !(i & 0x88) — проверка, что индекс находится в игровом поле 8x8
        // (b[i] & 7) == 4 — тип фигуры "Король"
        if (!(i & 0x88) && (b[i] & side) && (b[i] & 7) == 4)
        {
            kingPos = i;
            break;
        }
    }

    if (kingPos == -1) return ""; // В норме король всегда должен быть на доске

    // 2. Ищем фигуру противника, атакующую kingPos
    for (int i = 0; i < 128; i++)
    {
        if (!(i & 0x88) && (b[i] & opponent))
        {
            int piece = b[i] & 7;
            bool attacks = false;
            int diff = kingPos - i;

            // Логика атаки в зависимости от типа фигуры (согласно вашему массиву o[])
            if (piece == 3) // Конь (Knight)
            {
                int ad = std::abs(diff);
                if (ad == 14 || ad == 18 || ad == 31 || ad == 33) attacks = true;
            }
            else if (piece == 1 || piece == 2) // Пешка (Pawn)
            {
                // В вашей индексации Rank 1 имеет индекс 112, Rank 8 — 0.
                // Белые бьют "вверх" (вычитание из индекса), черные "вниз" (сложение).
                if (opponent == 8) // Атакует белая пешка
                {
                    if (diff == -15 || diff == -17) attacks = true;
                }
                else // Атакует черная пешка
                {
                    if (diff == 15 || diff == 17) attacks = true;
                }
            }
            else if (piece == 4) // Король (проверка соседних клеток)
            {
                int ad = std::abs(diff);
                if (ad == 1 || ad == 15 || ad == 16 || ad == 17) attacks = true;
            }
            else // Скользящие фигуры: 5 (Слон), 6 (Ладья), 7 (Ферзь)
            {
                int dir = 0;
                if (diff % 16 == 0) dir = (diff > 0 ? 16 : -16); // Вертикаль
                else if (std::abs(diff) < 8 && (i >> 4 == kingPos >> 4)) dir = (diff > 0 ? 1 : -1); // Горизонталь
                else if (diff % 15 == 0) dir = (diff > 0 ? 15 : -15); // Диагональ /
                else if (diff % 17 == 0) dir = (diff > 0 ? 17 : -17); // Диагональ \

                if (dir != 0)
                {
                    // Проверяем, соответствует ли направление типу фигуры
                    bool canMove = false;
                    if (piece == 7) canMove = true; // Ферзь — во все стороны
                    else if (piece == 6 && (std::abs(dir) == 1 || std::abs(dir) == 16)) canMove = true; // Ладья
                    else if (piece == 5 && (std::abs(dir) == 15 || std::abs(dir) == 17)) canMove = true; // Слон

                    if (canMove)
                    {
                        // Проверка преград (blockers) между фигурой и королем
                        int p = i + dir;
                        while (p != kingPos)
                        {
                            if (b[p] & 31) break; // Наткнулись на любую фигуру
                            p += dir;
                        }
                        if (p == kingPos) attacks = true;
                    }
                }
            }

            if (attacks)
            {
                // Лямбда для перевода индекса (0-127) в строку (например, "e2")
                auto toCoord = [](int idx) {
                    std::string s = "";
                    s += (char)('a' + (idx & 7));
                    s += (char)('1' + (7 - (idx >> 4)));
                    return s;
                };
                return toCoord(i) + toCoord(kingPos);
            }
        }
    }
    return ""; // Шаха нет
}

int ChessEngine::getFigureCollor(const std::string& field)
{
    if (field.size() < 2) return 0;
    // 1. Быстро переводим координаты в индекс доски
    int j = (field[0] - 'a');
    int i = (field[1] - '1');
    int idx = 16 * (7 - i) + j;
    // 2. Достаем биты цвета напрямую из памяти (8 или 16)
    int pieceColor = b[idx] & 24;
    // 3. Конвертируем внутренний цвет движка в значения вашего enum
    if (pieceColor == 16) return BLACK; // 8 в движке = черные -> возвращаем ваш BLACK (16)
    if (pieceColor == 8) return WHITE; // 16 в движке = белые -> возвращаем ваш WHITE (8)
    return 0; // Клетка пуста
}

bool ChessEngine::isMoveValid()
{
    // проверка связаной фигуры
    k ^= 24;
    auto check = isCheck();
    k ^= 24;

    // валидного хода и связаной фигуры проверка связаной фигуры
    if (k != kLast && check.empty())
    {
        // ход разрешён
        isLastMoveValid = true;
        kLast = k;
    }
    else
    {
        std::string s = (k != 8?"White":"Black");
        s.append(" make invalid move ").append(c).push_back(' ');
        s.push_back('\n');
        printf(s.c_str());
        isLastMoveValid = false;
    }
    return isLastMoveValid;
}

bool ChessEngine::checkGameRule(int estimate)
{
    if (!(estimate > -I + 1))  // Очень большое отрицательное значение = мат
    {
        // Переменная k сейчас указывает на того, кому поставили мат
        if (k == WHITE)
            pgn->setWhiteWin();
        else
            pgn->setBlackWin();
        printf("\n%s",pgn->generatePGN().c_str());
        isGame = false;
        return true;
    }
    if (estimate == 1)      // Пат
    {
        pgn->setDraw();
        printf("\n%s",pgn->generatePGN().c_str());
        isGame = false;
    }

    if (fiftyMoveClock >= 100)
    {
        // printf("Draw %d move rule!\n", maxMove);
        pgn->setDraw();
        printf("\n%s",pgn->generatePGN().c_str());
        isGame = false;
        return true;
    }
    return false;
}

void ChessEngine::makeBackup()
{
    memcpy(bBackup, b, 129);
    kBk = k;
    NBk = N;
    JBk = J;
    ZBk = Z;
    LBk = L;
    OBk = O;
    memcpy(cEBk, cE, 9);
}

void ChessEngine::applyBackup()
{
    memcpy(b, bBackup, 129);
    k = kBk;
    N = NBk;
    J = JBk;
    Z = ZBk;
    L = LBk;
    O = OBk;
    memcpy(cE, cEBk, 9);
}

char ChessEngine::getFigure(std::string field)
{
    int j = (field[0] - 'a');
    int i = (field[1] - '1');
#ifndef STM32F103xB
    return b[16 * (7 - i) + j] & 15;
#else
    return sym[b[16 * (7 - i) + j] & 15];
#endif
}

std::string ChessEngine::toCoord(int idx)
{
    if (idx < 0 || (idx & 0x88))
        return "??";

    int file = idx & 7;      // Младшие 3 бита — это столбец (0=a, 7=h)
    int rank = idx >> 4;     // Старшие биты — это строка (0=8-я горизонталь, 7=1-я)

    std::string res = "";
    res += (char)('a' + file);   // Превращаем 0-7 в 'a'-'h'
    res += (char)('8' - rank);   // Превращаем 0 в '8', а 7 в '1'
    return res;
}

std::string ChessEngine::formatPGNMove(std::string move)
{
    if (move.length() < 4) return move;

    int f = move[0] - 16 * move[1] + 799;
    int t = move[2] - 16 * move[3] + 799;
    int piece = b[f] & 7;				// Получаем тип фигуры (в MicroMax это b[index] & 7)

    // 1. Проверка на рокировку
    if (piece == 4) { // Король
        if (move == "e1g1" || move == "e8g8") return "O-O";
        if (move == "e1c1" || move == "e8c8") return "O-O-O";
    }

    std::string pgnM = "";
    bool isCapture = (b[t] != 0); // Если на целевой клетке что-то есть

    // Взятие на проходе (пешка ходит по диагонали на пустую клетку)
    if ((piece == 1 || piece == 2) && (move[0] != move[2]) && b[t] == 0) {
        isCapture = true;
    }

    // 2. Формируем строку взятия
    if (isCapture) {
        pgnM = move.substr(0, 2) + "x" + move.substr(2, 2);
    } else {
        pgnM = move; // Обычный ход e2e4
    }

    return pgnM;
}

std::string ChessEngine::formatPGNMove(int from, int to, bool isCapture, int pieceType, bool isPromotion) {
    std::string pgnMove;
    int fromFile = from & 7;
    int fromRank = from >> 4;
    int toFile = to & 7;
    int toRank = to >> 4;

    // Рокировка
    if (pieceType == 4) {
        if (from == 116 && to == 118) return "O-O";
        if (from == 116 && to == 114) return "O-O-O";
        if (from == 4 && to == 6) return "O-O";
        if (from == 4 && to == 2) return "O-O-O";
    }

    // Превращение пешки
    if (pieceType < 3 && (toRank == 0 || toRank == 7)) {
        isPromotion = true;
    }

    // Определяем обозначение фигуры
    char pieceChar = ' ';
    switch (pieceType) {
    case 1: case 2: pieceChar = ' '; break; // пешка – без буквы
    case 3: pieceChar = 'N'; break;
    case 4: pieceChar = 'K'; break;
    case 5: pieceChar = 'B'; break;
    case 6: pieceChar = 'R'; break;
    case 7: pieceChar = 'Q'; break;
    }

    // Для фигур (кроме пешек) всегда добавляем начальную клетку в длинной нотации
    if (pieceChar != ' ') {
        pgnMove += pieceChar;
        pgnMove += 'a' + fromFile;
        pgnMove += '1' + (7 - fromRank);
    } else {
        // Для пешки: при взятии указываем вертикаль
        if (isCapture) {
            pgnMove += 'a' + fromFile;
        }
    }

    // Взятие
    if (isCapture) {
        pgnMove += 'x';
    }

    // Клетка назначения
    pgnMove += 'a' + toFile;
    pgnMove += '1' + (7 - toRank);

    // Превращение
    if (isPromotion) {
        pgnMove += "=Q";
    }

    return pgnMove;
}

void ChessEngine::updateFiftyMoveClock(int piece, bool isCapture) {
    if (piece == 1 || piece == 2 || isCapture) {
        fiftyMoveClock = 0;
    } else {
        fiftyMoveClock++;
    }
}

bool ChessEngine::canCastle(int side, bool shortCastle) {
    // 1. Проверяем, двигался ли король
    int kingIdx = (side == 8) ? 116 : 4; // 116 = e1 (0x74), 4 = e8 (0x04)
    if (b[kingIdx] & 32) return false; // король уже двигался

    // 2. Проверяем, двигалась ли ладья
    int rookIdx;
    if (side == 8) { // белые
        rookIdx = shortCastle ? 119 : 112; // h1 (0x77) или a1 (0x70)
    } else { // чёрные
        rookIdx = shortCastle ? 7 : 0;     // h8 (0x07) или a8 (0x00)
    }
    if (b[rookIdx] & 32) return false; // ладья уже двигалась

    // 3. Проверяем, не находится ли король под шахом сейчас
    if (!isCheck().empty()) return false;

    // 4. Проверяем физическую пустоту полей между фигурами
    if (side == 8) { // WHITE
        if (shortCastle) {
            // f1(117), g1(118)
            return (b[117] == 0 && b[118] == 0);
        } else {
            // d1(115), c1(114), b1(113)
            return (b[115] == 0 && b[114] == 0 && b[113] == 0);
        }
    } else { // BLACK
        if (shortCastle) {
            // f8(5), g8(6)
            return (b[5] == 0 && b[6] == 0);
        } else {
            // d8(3), c8(2), b8(1)
            return (b[3] == 0 && b[2] == 0 && b[1] == 0);
        }
    }
}

void ChessEngine::updateEpSquare(const std::string& move) {
    if (move.length() < 4) {
        epSquare = 0;
        return;
    }
    int from = move[0] - 16 * move[1] + 799;
    int to   = move[2] - 16 * move[3] + 799;
    int pieceFull = b[to];
    int pieceType = pieceFull & 7;
    // Если разница по вертикали равна 2 и это пешка
    if (abs((to >> 4) - (from >> 4)) == 2 && pieceType < 3) {
        epSquare = (from + to) / 2;   // клетка между from и to
    } else {
        epSquare = 0;
    }
}

std::string ChessEngine::showBoard()
{
    std::string chessboard;
    chessboard.reserve(256);

    chessboard.append("\n  +-----------------+\n");
    for (int i = 0; i < 8; i++)
    {
        chessboard.append(" ");
        char str[2];
        if (engineSide == BLACK || engineSide == PVP)
            sprintf(str, "%d", 8 - i);
        else
            sprintf(str, "%d", i + 1);
        chessboard.append(str);
        chessboard.append("| ");
        for (int j = 0; j < 8; j++)
        {
#ifndef STM32F103xB
            char* c;
            if (this->engineSide==WHITE)
                c = (char*)sym[b[16*(7-i)+(7-j)]&15];
            else
                c = (char*)sym[b[16*i+j]&15];
            chessboard.append(c);
#else
            char c;
            if (this->engineSide == WHITE)
                c = sym[b[16 * (7 - i) + (7 - j)] & 15];
            else
                c = sym[b[16 * i + j] & 15];
            chessboard.push_back(c);
#endif
            chessboard.append(" ");
        }
        chessboard.append("|\n");
    }
    chessboard.append("  +-----------------+\n");
    if (this->engineSide == WHITE)
        chessboard.append("    h g f e d c b a\n");
    else
        chessboard.append("    a b c d e f g h\n");

    return chessboard;
}

