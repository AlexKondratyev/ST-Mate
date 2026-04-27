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

    // 1. Clear only the playable part of the board (left 8x8),
    // leaving the right side intact where the evaluation table is stored.
    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            b[r * 16 + f] = 0;
        }
    }

    int rank = 0; // 0 = 8th rank
    int file = 0; // 0 = file 'a'
    int i = 0;

    // 2. Parse piece placement
    while (fen[i] != ' ' && fen[i] != '\0') {
        char c = fen[i++];
        if (c == '/') {
            rank++;
            file = 0;
        } else if (c >= '1' && c <= '8') {
            file += (c - '0');
        } else {
            // Determine color (White = 8, Black = 16)
            int color = (c >= 'A' && c <= 'Z') ? 8 : 16;

            // Convert to lowercase for switch
            char lower = (c >= 'A' && c <= 'Z') ? (c + 32) : c;

            int type = 0;
            switch (lower) {
            case 'p': type = (color == 8) ? 1 : 2; break; // Pawns have different types in Micro-Max
            case 'n': type = 3; break;
            case 'k': type = 4; break;
            case 'b': type = 5; break;
            case 'r': type = 6; break;
            case 'q': type = 7; break;
            }

            // Set flag 32 (non-virgin) to all pieces by default,
            // to prevent illegal castling.
            b[rank * 16 + file] = color | type | 32;
            file++;
        }
    }

    if (fen[i] == '\0') return;
    if (fen[i] == ' ') i++; // Skip space

    // 3. Parse whose turn (inverted)
    if (fen[i] == 'b') {
        k = WHITE;  // White to move
    } else if (fen[i] == 'w') {
        k = BLACK; // Black to move
    }
    kLast = k; // Synchronize for correct validation
    i++;

    if (fen[i] == '\0') return;
    if (fen[i] == ' ') i++; // Skip space

    // 4. Parse castling rights (remove flag 32 from virgin pieces)
    while (fen[i] != ' ' && fen[i] != '\0') {
        switch (fen[i]) {
        case 'K':
            b[7 * 16 + 4] &= ~32; // White king e1
            b[7 * 16 + 7] &= ~32; // White rook h1
            break;
        case 'Q':
            b[7 * 16 + 4] &= ~32; // White king e1
            b[7 * 16 + 0] &= ~32; // White rook a1
            break;
        case 'k':
            b[0 * 16 + 4] &= ~32; // Black king e8
            b[0 * 16 + 7] &= ~32; // Black rook h8
            break;
        case 'q':
            b[0 * 16 + 4] &= ~32; // Black king e8
            b[0 * 16 + 0] &= ~32; // Black rook a8
            break;
        }
        i++;
    }
}

const std::string ChessEngine::getFEN()
{
    std::string fen;

    // 1. Piece placement (8 ranks)
    for (int rank = 7; rank >= 0; rank--) {
        int emptyCount = 0;

        for (int file = 0; file < 8; file++) {
            int idx = (7 - rank) * 16 + file; // index in 0x88 format (row 0 = rank 8)
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

                // Determine the piece symbol
                switch (type) {
                case 1: pieceChar = 'P'; break; // white pawn
                case 2: pieceChar = 'P'; break; // black pawn (will be lowercase)
                case 3: pieceChar = 'N'; break;
                case 4: pieceChar = 'K'; break;
                case 5: pieceChar = 'B'; break;
                case 6: pieceChar = 'R'; break;
                case 7: pieceChar = 'Q'; break;
                }

                // White (8) uses uppercase, black (16) uses lowercase
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


    // 2. Side to move
    fen += ' ';
    if (getCurrentSide() == WHITE) {
        fen += 'w';
    } else {
        fen += 'b';
    }

    // 3. Castling rights
    fen += ' ';
    bool hasCastling = false;

    // White: check that king and rooks have not moved (flag 32 not set)
    if (!(b[116] & 32)) { // white king e1 (0x74)
        if (!(b[119] & 32)) { // white rook h1 (0x77)
            fen += 'K';
            hasCastling = true;
        }
        if (!(b[112] & 32)) { // white rook a1 (0x70)
            fen += 'Q';
            hasCastling = true;
        }
    }

    // Black
    if (!(b[4] & 32)) { // black king e8 (0x04)
        if (!(b[7] & 32)) { // black rook h8 (0x07)
            fen += 'k';
            hasCastling = true;
        }
        if (!(b[0] & 32)) { // black rook a8 (0x00)
            fen += 'q';
            hasCastling = true;
        }
    }

    if (!hasCastling) {
        fen += '-';
    }

    // 4. En passant target square
    fen += ' ';
    if (epSquare != 0) {
        int file = epSquare & 7;
        int rank = epSquare >> 4;
        fen += (char)('a' + file);
        fen += (char)('8' - rank);
    } else {
        fen += '-';
    }

    // 5. Halfmove clock (50-move rule)
    fen += ' ';
    fen += std::to_string(fiftyMoveClock); // FEN counts halfmoves

    // 6. Move number (starts from 1)
    fen += ' ';
    fen += std::to_string(moveNumber + 1);

    return fen;
}


