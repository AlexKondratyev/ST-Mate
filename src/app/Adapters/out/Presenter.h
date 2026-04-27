/*
 * Presenter.h
 *
 *  Created on: Feb 2, 2026
 *      Author: alex
 */

#ifndef ADAPTERS_OUT_PRESENTER_H_
#define ADAPTERS_OUT_PRESENTER_H_

#include "../../app/Interactor/OutputBoundary.h"
#include "../../Drivers/BluetoothDriver/Listener.h"
#include "../../Drivers/FlashDriver/FlashDriverInterface.h"
#include "FieldHighlightInterface.h"
#include <stdint.h>

struct ChessTheme {
    struct Color { uint8_t r, g, b; };
    Color initPosCorrect{0,255,0};	// Initial position correct (Green)
    Color initPosFalse{255,0,0};	// Initial position incorrect (Red)
    Color availableMove{128,128,128};   // Available move (Dim white)
    Color moveAttack{255,0,0};    	// Square for capture (Red)
    Color lastMove{128,128,0};      	// Last move (Yellow)
    Color help{0,128,255};          	// Hint (Light Blue)
    Color activePiece{255,255,255};   	// Piece in hand (White)
    Color error{255,0,0};         	// Error (Red)
    Color checkKing{255,0,0};       // King in check (Red)
    Color checkAttack{0,0,255};     // Check attacker (Blue)
    Color mateKingWin{255,0,0};     // Winning king in checkmate (Red)
    Color mateKingLose{0,255,0};    // King in checkmate (losing side) (Green)
    Color stalemate{0,255,255};		// Stalemate (Cyan) / Draw (Light Blue)
};

class Presenter : public OutputBoundary , public Listener
{
private:
    Fields *fields = nullptr;
    FieldHighlightInterface* highlight = nullptr;
    FlashDriverInterface *flash = nullptr;

    int curThemeId = 0;
    ChessTheme theme;


    GameState gameState = NO_POSITION;
    MoveTurn move = WHITE_TURN;
    GameMode mode;

    // Internal states for animations
    uint32_t tick = 0; // Frame counter for animation

    int activeFigureIdx = -1;
    int helpFromIdx = -1, helpToIdx = -1;
    int lastFromIdx = -1, lastToIdx = -1;
    int lastInvalidFromIdx = -1, lastInvalidToIdx = -1;

    int checkAttackerIdx = -1, checkKingIdx = -1;

    int winningSide = 0;
    int posWhiteKing = -1, posBlackKing = -1;

    // Arrays for available moves (64 squares)
    bool availableNormalMoves[64] = {false};
    bool availableAttackMoves[64] = {false};

    void loadTheme(int themeId); // 0 - Classic
    // Auxiliary function for wave generation for pulsation
    uint8_t getWave(uint32_t period, uint8_t max_val);

public:
    Presenter(FieldHighlightInterface *highlight,
    			FlashDriverInterface *flash);
    virtual ~Presenter();

    void run();

    virtual void messege(const std::string &message) override;
    virtual void setFields(Fields &fields) override;
    virtual void setMode(GameState state) override;
    virtual void setActiveSide(MoveTurn side) override;
    virtual void setGameMode(GameMode mode) override;

    virtual void clearAllHighlights() override;
    virtual void setHelpMove(int from, int to) override;
    virtual void setActiveFigure(int index) override;
    virtual void addAvailableMove(int index, bool isAttack = false) override;
    virtual void setLastMove(int from, int to) override;
    virtual void setInvalidLastMove(int from, int to) override;

    virtual void setCheck(int posAttack, int posKing) override;
    virtual void setCheckmate(int winningSide, int whiteKingPos, int blackKingPos) override;
    virtual void setStalemate(int whiteKingPos, int blackKingPos) override;
};

#endif /* ADAPTERS_OUT_PRESENTER_H_ */
