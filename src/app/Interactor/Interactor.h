/*
 * Interactor.h
 *
 *  Created on: Feb 2, 2026
 *      Author: alex
 */

#ifndef INTERACTOR_INTERACTOR_H_
#define INTERACTOR_INTERACTOR_H_

#include "InputBoundary.h"
#include "OutputBoundary.h"
#include "../../Drivers/FlashDriver/FlashDriverInterface.h"
#include "../../Drivers/BluetoothDriver/Listener.h"

#include "../Entities/ChessEngine.h"

#include <time.h>

static const char* modeStr[]  = {"Engine black","Engine white","PvP"};

class Interactor : public InputBoundary, public Listener
{
private:
	ChessEngineInterface *e = nullptr;
	FlashDriverInterface *flash = nullptr;

	std::string inHand;
	bool castelingInProgress =  false;
	std::string rockMovebyCasteling;
	Fields *fields = nullptr;

	OutputBoundary *output;

	GameMode mode;

	GameState gameState = NO_POSITION;
	MoveTurn sideTurn = WHITE_TURN;
	std::string eMove;

	unsigned int seed = 0;
	bool isNeedHelp = false;
	struct tm datetime;

	void drawValidMoves(const std::string& field);
	void checkActiveGame();
	bool isStartPosition();
	void reset();

	void savePlaySettings();
	void loadPlaySettings();

	void runPvsC();
	void runPvsP();

	void figureMovePvsC(const std::string&, int index, bool isSet);
	void figureMovePvsP(const std::string&, int index, bool isSet);

public:
	Interactor(OutputBoundary *output = nullptr);
	Interactor(GameMode mode, OutputBoundary *output, FlashDriverInterface* flash);
	virtual ~Interactor();

	void run();
	void setSeed(unsigned int seed);
	void setIsNeedHelp(bool isNeedHelp = false);
	void setDateTime(const struct tm &datetime);
	void setMode(GameMode mode);

	virtual void messege(const std::string &message) override;
protected:
	virtual void setFields(Fields &filds) override;
	virtual void figureMove(const std::string&, int index, bool isSet) override;
};

#endif /* INTERACTOR_INTERACTOR_H_ */
