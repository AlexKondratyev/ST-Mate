/*
 * FEN.cpp
 *
 *  Created on: Mar 31, 2026
 *      Author: alex
 */

#include "ChessEngine.h"

void ChessEngine::setFEN(const std::string &fen)
{
    if (fen.empty()) return;

    epSquare = 0;

    pgn->setFEN(fen);

    // 1. Очищаем только игровую часть доски (левые 8x8),
    // не трогая правую часть, где хранится таблица оценки позиции.
    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            b[r * 16 + f] = 0;
        }
    }

    int rank = 0; // 0 = 8-я горизонталь
    int file = 0; // 0 = вертикаль 'a'
    int i = 0;

    // 2. Парсим расположение фигур
    while (fen[i] != ' ' && fen[i] != '\0') {
        char c = fen[i++];
        if (c == '/') {
            rank++;
            file = 0;
        } else if (c >= '1' && c <= '8') {
            file += (c - '0');
        } else {
            // Определяем цвет (Белые = 8, Черные = 16)
            int color = (c >= 'A' && c <= 'Z') ? 8 : 16;

            // Переводим в нижний регистр для свитча
            char lower = (c >= 'A' && c <= 'Z') ? (c + 32) : c;

            int type = 0;
            switch (lower) {
            case 'p': type = (color == 8) ? 1 : 2; break; // У пешек разные типы в Micro-Max
            case 'n': type = 3; break;
            case 'k': type = 4; break;
            case 'b': type = 5; break;
            case 'r': type = 6; break;
            case 'q': type = 7; break;
            }

            // Ставим флаг 32 (non-virgin) всем фигурам по умолчанию,
            // чтобы запретить нелегальные рокировки.
            b[rank * 16 + file] = color | type | 32;
            file++;
        }
    }

    if (fen[i] == '\0') return;
    if (fen[i] == ' ') i++; // Пропускаем пробел

    // 3. Парсим чей ход (инвертировано)
    if (fen[i] == 'b') {
        k = WHITE;  // Ход белых
    } else if (fen[i] == 'w') {
        k = BLACK; // Ход черных
    }
    kLast = k; // Синхронизируем для корректной валидации
    i++;

    if (fen[i] == '\0') return;
    if (fen[i] == ' ') i++; // Пропускаем пробел

    // 4. Парсим права на рокировку (снимаем флаг 32 с девственных фигур)
    while (fen[i] != ' ' && fen[i] != '\0') {
        switch (fen[i]) {
        case 'K':
            b[7 * 16 + 4] &= ~32; // Белый король e1
            b[7 * 16 + 7] &= ~32; // Белая ладья h1
            break;
        case 'Q':
            b[7 * 16 + 4] &= ~32; // Белый король e1
            b[7 * 16 + 0] &= ~32; // Белая ладья a1
            break;
        case 'k':
            b[0 * 16 + 4] &= ~32; // Черный король e8
            b[0 * 16 + 7] &= ~32; // Черная ладья h8
            break;
        case 'q':
            b[0 * 16 + 4] &= ~32; // Черный король e8
            b[0 * 16 + 0] &= ~32; // Черная ладья a8
            break;
        }
        i++;
    }
}

const std::string ChessEngine::getFEN()
{
    std::string fen;

    // 1. Расположение фигур (8 рядов)
    for (int rank = 7; rank >= 0; rank--) {
        int emptyCount = 0;

        for (int file = 0; file < 8; file++) {
            int idx = (7 - rank) * 16 + file; // индекс в формате 0x88 (ряд 0 = 8-я горизонталь)
            int piece = b[idx];

            if (piece == 0) {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    fen += std::to_string(emptyCount);
                    emptyCount = 0;
                }

                int color = piece & 24;
                int type = piece & 7;
                char pieceChar = 0;

                // Определяем символ фигуры
                switch (type) {
                case 1: pieceChar = 'P'; break; // белая пешка
                case 2: pieceChar = 'P'; break; // чёрная пешка (будет строчной)
                case 3: pieceChar = 'N'; break;
                case 4: pieceChar = 'K'; break;
                case 5: pieceChar = 'B'; break;
                case 6: pieceChar = 'R'; break;
                case 7: pieceChar = 'Q'; break;
                }

                // Белые (8) – заглавные, чёрные (16) – строчные
                if (color == 8) {
                    fen += pieceChar;
                } else {
                    fen += tolower(pieceChar);
                }
            }
        }

        if (emptyCount > 0) {
            fen += std::to_string(emptyCount);
        }

        if (rank > 0) fen += '/';
    }


    // 2. Чей ход
    fen += ' ';
    if (getCurrentSide() == WHITE) {
        fen += 'w';
    } else {
        fen += 'b';
    }

    // 3. Права на рокировку
    fen += ' ';
    bool hasCastling = false;

    // Белые: проверяем, что король и ладьи не двигались (флаг 32 не установлен)
    if (!(b[116] & 32)) { // белый король e1 (0x74)
        if (!(b[119] & 32)) { // белая ладья h1 (0x77)
            fen += 'K';
            hasCastling = true;
        }
        if (!(b[112] & 32)) { // белая ладья a1 (0x70)
            fen += 'Q';
            hasCastling = true;
        }
    }

    // Чёрные
    if (!(b[4] & 32)) { // чёрный король e8 (0x04)
        if (!(b[7] & 32)) { // чёрная ладья h8 (0x07)
            fen += 'k';
            hasCastling = true;
        }
        if (!(b[0] & 32)) { // чёрная ладья a8 (0x00)
            fen += 'q';
            hasCastling = true;
        }
    }

    if (!hasCastling) {
        fen += '-';
    }

    // 4. Взятие на проходе
    fen += ' ';
    if (epSquare != 0) {
        int file = epSquare & 7;
        int rank = epSquare >> 4;
        fen += (char)('a' + file);
        fen += (char)('8' - rank);
    } else {
        fen += '-';
    }

    // 5. Полуходовой счётчик (правило 50 ходов)
    fen += ' ';
    fen += std::to_string(fiftyMoveClock); // в FEN считаются полуходы

    // 6. Номер хода (начинается с 1)
    fen += ' ';
    fen += std::to_string(moveNumber + 1);

    return fen;
}


