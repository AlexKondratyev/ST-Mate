/*
 * Interactor.cpp
 *
 *  Created on: Feb 2, 2026
 *      Author: alex
 */

#include "Interactor.h"
#include <memory.h>
#include "../Entities/Converter.h"

struct SettingsPGN settingsPGN = {
	"Casual Game",
	"Iserlohn, NRW GER",
	1,
	"Player1",
	"Player2"
};

Interactor::Interactor(OutputBoundary *output)
{
	this->output = output;
	this->mode = PLAYER_VS_PLAYER;
	this->output->setGameMode(mode);
}

Interactor::Interactor(GameMode mode, OutputBoundary *output, FlashDriverInterface* flash)
{
	this->mode = mode;
	this->output = output;
	this->flash = flash;
	if (flash->isInit())
	{
		loadPlaySettings();
	}
	this->output->setGameMode(mode);
}

Interactor::~Interactor()
{
	delete e;
}

void Interactor::setMode(GameMode mode)
{
	if (this->mode != mode)
	{
		this->mode = mode;
		printf("Mode changed: %s\n", modeStr[this->mode]);
		reset();
	}
}

void Interactor::run()
{
	switch (gameState) {
		case NO_POSITION:
		{
//			gameState = GAME;
			if(isStartPosition())
			{
				printf("New game start\n");
				gameState = GAME;
			}
			else
			{
				break;
			}
			break;
		}
		case GAME:
		{
			switch (mode) {
				case PLAYER_WHITE:
				case PLAYER_BLACK:
					runPvsC();
					break;
				case PLAYER_VS_PLAYER:
					runPvsP();
//					e->setFEN("3k4/ppp1n1n1/7p/3p4/P2P4/2PB3P/1P3r1P/2K1R3 w - - 3 27");
					break;
				default:
					break;
			}

			volatile bool newGame = isStartPosition() && !e->getIsLastMoveValid();
			if (newGame)
			{
				gameState = NO_POSITION;
				reset();
				return;
			}

			if (isNeedHelp)
			{
				auto helpMove = e->getHelp();
				if (helpMove.empty()) return;
				output->setHelpMove(Converter::convert(helpMove.substr(0,2)),
										  Converter::convert(helpMove.substr(2,2)));
				isNeedHelp = false;
			}
			break;
		}
		case GAME_OVER:
		{
			break;
		}
		default:
			break;
	}

	output->setMode(gameState);
	output->setActiveSide(sideTurn);
}

void Interactor::figureMove(const std::string& position, int index, bool isSet)
{
	switch (gameState) {
		case NO_POSITION:
		{
			isNeedHelp = false;
			break;
		}
		case GAME:
		{
			switch (mode)
			{
				case PLAYER_WHITE:
				case PLAYER_BLACK:
					figureMovePvsC(position, index, isSet);
					break;
				case PLAYER_VS_PLAYER:
					figureMovePvsP(position, index, isSet);
					break;
				default:
					break;
			}
			break;
		}
		case GAME_OVER:
		{
			reset();
			break;
		}
		default:
			break;
	}
}

void Interactor::reset()
{
	delete e;
	e = nullptr;
	inHand.clear();
	output->clearAllHighlights();
	output->setMode(gameState);
	sideTurn = WHITE_TURN;
	gameState = NO_POSITION;
}

bool Interactor::isStartPosition()
{
	bool isSet = true;
	for (int i = 0; i < 64; i++)
	{
		if (i < 16)
		{
			if (fields->getField(i) != Fields::white)
			{
				isSet = false;
				break;
			}
		}
		if (i > 15 && i < 48)
		{
			if (fields->getField(i) != Fields::none)
			{
				isSet = false;
				break;
			}
		}
		if (i >= 48)
		{
			if (fields->getField(i) != Fields::black)
			{
				isSet = false;
				break;
			}
		}
	}
	return isSet;
}

void Interactor::drawValidMoves(const std::string& field)
{
	for (int i = 0; i < 64; i++)
	{
		std::string moveTo = field+Converter::convert(i);
		bool isValid = e->checkMove(moveTo);
		if (isValid)
			output->addAvailableMove(i, fields->getField(i) != Fields::none);
	}
}


void Interactor::setSeed(unsigned int seed)
{
	if (seed != this->seed)
		reset();
	this->seed = seed;
}

void Interactor::setDateTime(const struct tm &datetime)
{
	this->datetime = datetime;
}

void Interactor::setIsNeedHelp(bool isNeedHelp)
{
    static bool lastButtonState = false;
    // Если кнопка была отпущена, а теперь нажата — это фронт
    if (isNeedHelp && !lastButtonState) {
        this->isNeedHelp = true;
    }
    lastButtonState = isNeedHelp;
}

void Interactor::checkActiveGame()
{
	if (!e->isGameActive())
	{
		switch (e->getGameResult()) {
		case WHITE_WINS:
		case BLACK_WINS:
			output->setCheckmate(e->getWinningSide(),
					Converter::convert(e->getKingPosition(WHITE)),
					Converter::convert(e->getKingPosition(BLACK)));
			break;
		case DRAW:
			output->setStalemate(Converter::convert(e->getKingPosition(WHITE)),
					Converter::convert(e->getKingPosition(BLACK)));
			break;
		}
		gameState = GAME_OVER;
	}
}

void Interactor::messege(const std::string &message)
{
	if (message.find("EVENT?") != std::string::npos)
	{
		printf("Event: %s\n", settingsPGN.event);
	}
	if (message.find("EVENT:") != std::string::npos)
	{
	    const char* value = message.c_str() + 6; // пропускаем "EVENT:"
	    // Копируем в массив с ограничением длины
	    strncpy(settingsPGN.event, value, sizeof(settingsPGN.event) - 1);
	    settingsPGN.event[sizeof(settingsPGN.event) - 1] = '\0'; // гарантия завершения
	    savePlaySettings();
	    printf("Event set: %s\n", settingsPGN.event);
	}

	if (message.find("SITE?") != std::string::npos)
	{
		printf("Site: %s\n", settingsPGN.site);
	}
	if (message.find("SITE:") != std::string::npos)
	{
	    const char* value = message.c_str() + 5; // пропускаем "SITE:"
	    // Копируем в массив с ограничением длины
	    strncpy(settingsPGN.site, value, sizeof(settingsPGN.site) - 1);
	    settingsPGN.site[sizeof(settingsPGN.site) - 1] = '\0'; // гарантия завершения
	    savePlaySettings();
	    printf("Site set: %s\n", settingsPGN.site);
	}

	if (message.find("ROUND?") != std::string::npos)
	{
		printf("Round: %d\n", settingsPGN.round);
	}
	if (message.find("ROUND:") != std::string::npos)
	{
		int param = 0;
	    if (sscanf(message.c_str(), "ROUND:%d", &param) == 1)
	    {
	    	settingsPGN.round = param;
    		printf("Round set %d\n", settingsPGN.round);
    		savePlaySettings();
    	}
	}

	if (message.find("WHITE?") != std::string::npos)
	{
		printf("White name: %s\n", settingsPGN.white);
	}
	if (message.find("WHITE:") != std::string::npos)
	{
	    const char* value = message.c_str() + 6; // пропускаем "WHITE:"
	    // Копируем в массив с ограничением длины
	    strncpy(settingsPGN.white, value, sizeof(settingsPGN.white) - 1);
	    settingsPGN.white[sizeof(settingsPGN.white) - 1] = '\0'; // гарантия завершения
	    savePlaySettings();
	    printf("White name set: %s\n", settingsPGN.white);
	}

	if (message.find("BLACK?") != std::string::npos)
	{
		printf("Black name: %s\n", settingsPGN.black);
	}
	if (message.find("BLACK:") != std::string::npos)
	{
	    const char* value = message.c_str() + 6; // пропускаем "BLACK:"
	    // Копируем в массив с ограничением длины
	    strncpy(settingsPGN.black, value, sizeof(settingsPGN.black) - 1);
	    settingsPGN.black[sizeof(settingsPGN.black) - 1] = '\0'; // гарантия завершения
	    savePlaySettings();
	    printf("Black name set: %s\n", settingsPGN.black);
	}

	if (message.find("FEN?") != std::string::npos)
	{
		if (e == nullptr)
			printf("Game is inactive\n");
		else
			printf("FEN: %s\n", e->getFEN().c_str());
	}
	if (message.find("PGN?") != std::string::npos)
	{
		if (e == nullptr)
			printf("Game is inactive\n");
		else
		{
			printf("PGN:\n%s\n",e->getPGN().c_str());
		}
	}
}

void Interactor::setFields(Fields &fields)
{
	this->fields = &fields;
	output->setFields(*this->fields);
}

void Interactor::savePlaySettings()
{
	if(flash->isInit())
	{
		flash->write(4096, (uint8_t*)&settingsPGN, sizeof(SettingsPGN));
	}
}

void Interactor::loadPlaySettings()
{
	if(flash->isInit())
	{
		flash->read(4096, (uint8_t*)&settingsPGN, sizeof(SettingsPGN));
	}
}
