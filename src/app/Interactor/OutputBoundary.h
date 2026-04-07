/*
 * OutputBoundary.h
 *
 *  Created on: Feb 2, 2026
 *      Author: alex
 */

#ifndef INTERACTOR_OUTPUTBOUNDARY_H_
#define INTERACTOR_OUTPUTBOUNDARY_H_

#include "../GameSettings.h"
#include "../../app/Entities/Fields.h"

enum GameState
{
	NO_POSITION,
	GAME,
	GAME_OVER,
};
enum MoveTurn
{
	WHITE_TURN,
	BLACK_TURN,
};

class OutputBoundary
{
public:
    virtual ~OutputBoundary() = default;

    // Глобальные настройки
    virtual void setFields(Fields &fields) = 0;
    virtual void setMode(GameState state) = 0;
    virtual void setActiveSide(MoveTurn side) = 0;
    virtual void setGameMode(GameMode mode) = 0;

    // Игровые события (устанавливают флаги для run)
    virtual void clearAllHighlights() = 0; // Сброс всех подсветок хода

    virtual void setHelpMove(int from, int to) = 0;
    virtual void setActiveFigure(int index) = 0;
    virtual void addAvailableMove(int index, bool isAttack = false) = 0;
    virtual void setLastMove(int from, int to) = 0;
    virtual void setInvalidLastMove(int from, int to) = 0;

    // Критические состояния
    virtual void setCheck(int posAttack, int posKing) = 0;
    virtual void setCheckmate(int winningSide, int whiteKingPos, int blackKingPos) = 0;
    virtual void setStalemate(int whiteKingPos, int blackKingPos) = 0;
};

#endif /* INTERACTOR_OUTPUTBOUNDARY_H_ */
