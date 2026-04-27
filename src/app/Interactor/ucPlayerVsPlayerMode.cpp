/*
 * ucPlayerVsPlayerMode.cpp
 *
 *  Created on: Feb 19, 2026
 *      Author: alex
 */

#include "Interactor.h"
#include "../Entities/Converter.h"

void Interactor::figureMovePvsP(const std::string& position, int index, bool isSet)
{
	if (e == nullptr) return;

	// put the figure back
	if (isSet && inHand == position)
	{
		inHand.clear();
		output->clearAllHighlights();
		// --- ADD CHECK HIGHLIGHT RESTORATION ---
		std::string check = e->isCheck();
		if (!check.empty())
			output->setCheck(Converter::convert(check.substr(0,2)),
							 Converter::convert(check.substr(2,2)));
		return;
	}

	// picked up a figure
	if (!isSet && inHand.empty())
	{
		if (castelingInProgress) {inHand = position; return;}
		inHand = position;
		if (e->getFigureCollor(inHand) == e->getCurrentSide())
		{
			output->setActiveFigure(index);
			drawValidMoves(inHand);
		}
		else
			output->setInvalidLastMove(index, -1);
	}

	// made a move
	if (isSet && !inHand.empty())
	{
		if (castelingInProgress && rockMovebyCasteling.substr(2, 2)==position)
		{
			castelingInProgress = false;
			rockMovebyCasteling.clear();
			inHand.clear();
			output->clearAllHighlights();
			return;
		}
		std::string move = inHand + position;
		e->setMove(move);

		if (e->getIsLastMoveValid())
		{
			output->setLastMove(Converter::convert(inHand), index);
			if (e->isCastlingMove(move, rockMovebyCasteling))
			{
				castelingInProgress = true;
			    std::string from = rockMovebyCasteling.substr(0, 2);
			    std::string to = rockMovebyCasteling.substr(2, 2);
				output->setActiveFigure(Converter::convert(from));
				output->addAvailableMove(Converter::convert(to));
			}
		}
		else
			output->setInvalidLastMove(Converter::convert(inHand), index);

		auto check = e->isCheck();
		if (!check.empty())
			output->setCheck(Converter::convert(check.substr(0,2)),
									Converter::convert(check.substr(2,2)));
		inHand.clear();
		if (sideTurn == WHITE_TURN)
			sideTurn = BLACK_TURN;
		else
			sideTurn = WHITE_TURN;
	}
}

void Interactor::runPvsP()
{
	switch (gameState) {
		case GAME:
		{
			if (e == nullptr)
			{
				e = new ChessEngine(PVP, seed);
				e->setDate(datetime.tm_year, datetime.tm_mon, datetime.tm_mday);
			}

			checkActiveGame();
			break;
		}
		default:
			break;
	}
}


