/*
 * Presenter.cpp
 *
 *  Created on: Feb 2, 2026
 *      Author: alex
 */

#include "Presenter.h"
#include <string.h>


Presenter::Presenter(FieldHighlightInterface* highlight,
                     FlashDriverInterface*    flash)
    : highlight(highlight), flash(flash)
{
    if (flash->isInit())
        flash->read(300, reinterpret_cast<uint8_t*>(&curThemeId), sizeof(curThemeId));
    loadTheme(curThemeId);
    clearAllHighlights();
}

uint8_t Presenter::getWave(uint32_t period, uint8_t maxVal) const
{
    uint32_t t    = tick % period;
    uint32_t half = period / 2;
    if (t < half) return static_cast<uint8_t>(t * maxVal / half);
    return static_cast<uint8_t>((period - t) * maxVal / half);
}

void Presenter::updateAnimation()
{
    ++tick;
    pulseSlow = getWave(200, 255);
    pulseFast = getWave(64,  255);
}

void Presenter::writePixel(int i, PixelColor c)
{
    highlight->setFieldHighlight(i, c.r, c.g, c.b);
}

void Presenter::renderNoPosition()
{
    activeFigureIdx    = -1;
    helpFromIdx        = helpToIdx        = -1;
    lastFromIdx        = lastToIdx        = -1;
    lastInvalidFromIdx = lastInvalidToIdx = -1;

    for (int i = 0; i < 64; i++) {
        PixelColor c;

        if (i < 16) {
            c = (fields->getField(i) == Fields::white)
                ? theme.initPosCorrect
                : theme.initPosFalse.dimmed(pulseSlow);
        }
        else if (i < 48) {
            if (fields->getField(i) != Fields::none)
                c = theme.initPosFalse.dimmed(pulseSlow);
        }
        else {
            c = (fields->getField(i) == Fields::black)
                ? theme.initPosCorrect
                : theme.initPosFalse.dimmed(pulseSlow);
        }

        writePixel(i, c);
    }
}

PixelColor Presenter::computeGamePixel(int i) const
{
    PixelColor c;

    if (i == lastFromIdx || i == lastToIdx)	    			// Last move
        c = theme.lastMove;

    if (i == helpFromIdx || i == helpToIdx)					// Help
        c = theme.help.dimmed(pulseSlow);

    if (i == lastInvalidFromIdx || i == lastInvalidToIdx)	// False move
        c = theme.error.dimmed(pulseFast);

    if (availableNormalMoves[i])							// Allowed moves
        c = theme.availableMove;

    if (i == activeFigureIdx)								// Active figure
        c = theme.activePiece.dimmed(pulseSlow);

    if (checkKingIdx != -1)									// Check
    {
        if (i == checkKingIdx)     c = theme.checkKing.dimmed(pulseFast);
        if (i == checkAttackerIdx) c = theme.checkAttack.dimmed(pulseFast);
    }

    if (availableAttackMoves[i])							//  Attacked move
        c = theme.moveAttack.dimmed(pulseSlow);

    return c;
}

void Presenter::renderGame()
{
    for (int i = 0; i < 64; i++)
        writePixel(i, computeGamePixel(i));
}

void Presenter::renderGameOver()
{
    for (int i = 0; i < 64; i++) {
        PixelColor c;

        if (winningSide == 8 || winningSide == 16) {
            const bool whiteWon = (winningSide == 8);
            if (i == posWhiteKing)
                c = (whiteWon ? theme.mateKingWin : theme.mateKingLose).dimmed(pulseSlow);
            if (i == posBlackKing)
                c = (whiteWon ? theme.mateKingLose : theme.mateKingWin).dimmed(pulseSlow);
        } else {
            if (i == posWhiteKing || i == posBlackKing)
                c = theme.stalemate.dimmed(pulseSlow);
        }

        writePixel(i, c);
    }
}

void Presenter::run()
{
    if (fields == nullptr || highlight == nullptr) return;

    updateAnimation();

    switch (gameState) {
        case NO_POSITION: renderNoPosition(); break;
        case GAME:        renderGame();       break;
        case GAME_OVER:   renderGameOver();   break;
        default:          break;
    }
}

void Presenter::message(const std::string& message)
{
    if (message.find("THEME:") == std::string::npos) return;

    int param = 0;
    if (sscanf(message.c_str(), "THEME:%d", &param) == 1
        && param >= 0 && param <= 11)
    {
        curThemeId = param;
        loadTheme(curThemeId);
        if (flash->isInit())
            flash->write(300, reinterpret_cast<uint8_t*>(&curThemeId), sizeof(curThemeId));
        printf("Theme set:%d\n", curThemeId);
    }
}

void Presenter::setFields(Fields& f)      { fields    = &f; }
void Presenter::setMode(GameState s)      { gameState = s;  }
void Presenter::setActiveSide(MoveTurn s) { move      = s;  }
void Presenter::setGameMode(GameMode m)   { mode      = m;  }

void Presenter::clearAllHighlights()
{
    activeFigureIdx    = -1;
    helpFromIdx        = helpToIdx        = -1;
    checkAttackerIdx   = checkKingIdx     = -1;
    lastInvalidFromIdx = lastInvalidToIdx = -1;
    memset(availableNormalMoves, 0, sizeof(availableNormalMoves));
    memset(availableAttackMoves, 0, sizeof(availableAttackMoves));
}

void Presenter::setLastMove(int from, int to)
{
    lastFromIdx = from;
    lastToIdx   = to;
    lastInvalidFromIdx = lastInvalidToIdx = -1;
    clearAllHighlights();
}

void Presenter::setInvalidLastMove(int from, int to)
{
    clearAllHighlights();
    if (lastInvalidFromIdx == to && lastInvalidToIdx == from) {
        lastInvalidFromIdx = lastInvalidToIdx = -1;
        activeFigureIdx = -1;
        return;
    }
    lastInvalidFromIdx = from;
    lastInvalidToIdx   = to;
    activeFigureIdx    = -1;
}

void Presenter::setHelpMove(int from, int to)
{
    helpFromIdx = from;
    helpToIdx   = to;
}

void Presenter::setActiveFigure(int index)
{
    activeFigureIdx = index;
    if (index == lastInvalidFromIdx || index == -1)
        lastInvalidFromIdx = lastInvalidToIdx = -1;
}

void Presenter::addAvailableMove(int index, bool isAttack)
{
    if (index >= 0 && index < 64) {
        if (isAttack) availableAttackMoves[index] = true;
        else          availableNormalMoves[index]  = true;
    }
}

void Presenter::setCheck(int posAttack, int posKing)
{
    checkAttackerIdx = posAttack;
    checkKingIdx     = posKing;
}

void Presenter::setCheckmate(int ws, int whiteKingPos, int blackKingPos)
{
    winningSide  = ws;
    posWhiteKing = whiteKingPos;
    posBlackKing = blackKingPos;
    setMode(GAME_OVER);
}

void Presenter::setStalemate(int whiteKingPos, int blackKingPos)
{
    winningSide  = 0;
    posWhiteKing = whiteKingPos;
    posBlackKing = blackKingPos;
    setMode(GAME_OVER);
}
