/*
 * ucPlayerVsComputerMode.cpp
 *
 *  Created on: Feb 19, 2026
 *      Author: alex
 */

#include "Interactor.h"
#include "../Entities/Converter.h"
#include <memory.h>

// вызывается только при перемещении фигуры
void Interactor::figureMovePvsC(const std::string& position, int index, bool isSet)
{
    if (e == nullptr) return;

    int playerSide = mode == PLAYER_WHITE ? WHITE : BLACK;

    // ==========================================================
    // 1. ОБРАБОТКА ХОДА ДВИЖКА (Физический перенос его фигуры)
    // ==========================================================
    if (!eMove.empty())
    {
        if (!isSet && inHand.empty())
        {
            if (position == eMove.substr(0, 2)) {
                inHand = position;
                output->clearAllHighlights();
                output->setActiveFigure(index);
                output->addAvailableMove(Converter::convert(eMove.substr(2, 2)));
                sideTurn = (playerSide == WHITE) ? WHITE_TURN : BLACK_TURN;
            }
        }

        else if (isSet && !inHand.empty())
        {
            if (castelingInProgress && rockMovebyCasteling.substr(2, 2)==position)
            {
    			castelingInProgress = false;
    			rockMovebyCasteling.clear();
    			inHand.clear();
    			output->clearAllHighlights();
    			sideTurn = (playerSide == WHITE) ? WHITE_TURN : BLACK_TURN;
    			eMove.clear();
    			return;
            }

            if (position == eMove.substr(2, 2))
            {
                output->setLastMove(Converter::convert(eMove.substr(0,2)), index);

                // Проверяем рокировку
                if (e->isCastlingMove(eMove, rockMovebyCasteling))
                {
    				castelingInProgress = true;
    			    std::string from = rockMovebyCasteling.substr(0, 2);
    			    std::string to = rockMovebyCasteling.substr(2, 2);
    				output->setActiveFigure(Converter::convert(from));
    				output->addAvailableMove(Converter::convert(to));
    				return;
                }

				auto check = e->isCheck();
				if (!check.empty())
					output->setCheck(Converter::convert(check.substr(0,2)), Converter::convert(check.substr(2,2)));

                inHand.clear();
                eMove.clear();

                // !!! ВАЖНО: Проверяем мат ТОЛЬКО ПОСЛЕ ТОГО, как фигура физически поставлена
                checkActiveGame();

                sideTurn = (playerSide == WHITE) ? WHITE_TURN : BLACK_TURN;
            }
            else if (position == inHand) {
                inHand.clear();
                output->clearAllHighlights();
            }
        }
        return;
    }

    // ==========================================================
    // 2. ОБРАБОТКА ХОДА ИГРОКА
    // ==========================================================

    if (isSet && inHand == position) {
        inHand.clear();
        output->clearAllHighlights();
        // Восстановление шаха
        std::string check = e->isCheck();
        if (!check.empty()) output->setCheck(Converter::convert(check.substr(0,2)), Converter::convert(check.substr(2,2)));
        return;
    }

    // взяли в руку фигуру
    if (!isSet && inHand.empty())
    {
    	if (castelingInProgress) {inHand = position; return;}
        inHand = position;
        if (e->getFigureCollor(inHand) == playerSide) {
            output->setActiveFigure(index);
            drawValidMoves(inHand);
        } else {
            output->setInvalidLastMove(index, -1);
        }
    }

    // сделали ход
    if (isSet && !inHand.empty())
    {
		if (castelingInProgress && rockMovebyCasteling.substr(2, 2)==position)
		{
			castelingInProgress = false;
			rockMovebyCasteling.clear();
			inHand.clear();
			output->clearAllHighlights();
			eMove = e->getMove();
			return;
		}
        std::string move = inHand + position;
        e->setMove(move);

        if (e->getIsLastMoveValid()) {
            output->setLastMove(Converter::convert(inHand), index);
			if (e->isCastlingMove(move, rockMovebyCasteling))
			{
				castelingInProgress = true;
			    std::string from = rockMovebyCasteling.substr(0, 2);
			    std::string to = rockMovebyCasteling.substr(2, 2);
				output->setActiveFigure(Converter::convert(from));
				output->addAvailableMove(Converter::convert(to));
			}

            // Проверка рокировки игрока
            if (e->isCastlingMove(move, rockMovebyCasteling)) {
                castelingInProgress = true;
                output->setHelpMove(Converter::convert(rockMovebyCasteling.substr(0, 2)),
                                  Converter::convert(rockMovebyCasteling.substr(2, 2)));
                inHand.clear();
                return;
            }

            // Проверяем, не поставил ли ИГРОК мат движку
            if (e->getCurrentSide() != playerSide)
            {
            	checkActiveGame();
            }
            // Если игра продолжается, запрашиваем ход у движка
            eMove = e->getMove();
            if (!eMove.empty()) { // Подсвечиваем ход движка, чтобы игрок видел его ДО анимации конца игры
					std::string from = eMove.substr(0, 2);
					std::string to = eMove.substr(2, 2);
					output->setActiveFigure(Converter::convert(from));
					output->addAvailableMove(Converter::convert(to));
            }
            sideTurn = (playerSide == WHITE) ? BLACK_TURN : WHITE_TURN;
        } else {
            output->setInvalidLastMove(Converter::convert(inHand), index);
        }
        inHand.clear();
    }
}

// вызывается всегда переодически
void Interactor::runPvsC()
{
	switch (gameState) {
		case GAME:
		{
			if (e == nullptr)
			{
				eMove.clear();
				sideTurn = WHITE_TURN;
				switch (mode) {
					case PLAYER_WHITE:
						e = new ChessEngine(BLACK, seed);
						e->setDate(datetime.tm_year, datetime.tm_mon, datetime.tm_mday);
						break;
					case PLAYER_BLACK:
						e = new ChessEngine(WHITE, seed);
						e->setDate(datetime.tm_year, datetime.tm_mon, datetime.tm_mday);
						eMove = e->getMove();
						break;
					default:
						break;
				}
			}

			if (!eMove.empty() && !castelingInProgress)
			{
				std::string from = eMove.substr(0, 2);
				std::string to = eMove.substr(2, 2);
				output->setActiveFigure(Converter::convert(from));
				output->addAvailableMove(Converter::convert(to));
			}

			break;
		}
		default:
			break;
	}
}




