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

struct PixelColor {
    uint8_t r = 0, g = 0, b = 0;

    constexpr PixelColor() = default;
    constexpr PixelColor(uint8_t r_, uint8_t g_, uint8_t b_)
        : r(r_), g(g_), b(b_) {}

    // color * (alpha/255) — заменяет повторяющееся "r * pulse / 255"
    PixelColor dimmed(uint8_t alpha) const {
        return { static_cast<uint8_t>(r * alpha / 255u),
                 static_cast<uint8_t>(g * alpha / 255u),
                 static_cast<uint8_t>(b * alpha / 255u) };
    }
};

struct ChessTheme {
    PixelColor initPosCorrect  {0,   255, 0  }; // Initial position correct (Green)
    PixelColor initPosFalse    {255, 0,   0  }; // Initial position incorrect (Red)
    PixelColor availableMove   {128, 128, 128}; // Available move (Dim white)
    PixelColor moveAttack      {255, 0,   0  }; // Square for capture (Red)
    PixelColor lastMove        {128, 128, 0  }; // Last move (Yellow)
    PixelColor help            {0,   128, 255}; // Hint (Light Blue)
    PixelColor activePiece     {255, 255, 255}; // Piece in hand (White)
    PixelColor error           {255, 0,   0  }; // Error (Red)
    PixelColor checkKing       {255, 0,   0  }; // King in check (Red)
    PixelColor checkAttack     {0,   0,   255}; // Check attacker (Blue)
    PixelColor mateKingWin     {0,   255, 0  }; // Winning king in checkmate (Green)
    PixelColor mateKingLose    {255, 0,   0  }; // King in checkmate (losing side) (Red)
    PixelColor stalemate       {0,   255, 255}; // Stalemate (Cyan) / Draw (Light Blue)
};

// ----------------------------------------------------------------
class Presenter : public OutputBoundary, public Listener
{
public:
    Presenter(FieldHighlightInterface* highlight, FlashDriverInterface* flash);
    virtual ~Presenter() = default;

    void run();

    virtual void message(const std::string& message) override;

    virtual void setFields(Fields& fields) override;
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

private:
    Fields*                  fields    = nullptr;
    FieldHighlightInterface* highlight = nullptr;
    FlashDriverInterface*    flash     = nullptr;

    int        curThemeId = 0;
    ChessTheme theme;
    void loadTheme(int themeId);

    GameState gameState = NO_POSITION;
    MoveTurn  move      = WHITE_TURN;
    GameMode  mode      = PLAYER_VS_PLAYER;

    int activeFigureIdx    = -1;
    int helpFromIdx        = -1, helpToIdx        = -1;
    int lastFromIdx        = -1, lastToIdx        = -1;
    int lastInvalidFromIdx = -1, lastInvalidToIdx = -1;
    int checkAttackerIdx   = -1, checkKingIdx     = -1;
    int winningSide        =  0;
    int posWhiteKing       = -1, posBlackKing     = -1;

    bool availableNormalMoves[64] = {};
    bool availableAttackMoves[64] = {};

    uint32_t tick      = 0;
    uint8_t  pulseSlow = 0;
    uint8_t  pulseFast = 0;

    uint8_t    getWave(uint32_t period, uint8_t maxVal) const;
    void       updateAnimation();

    void renderNoPosition();
    void renderGame();
    void renderGameOver();

    PixelColor computeGamePixel(int i) const;

    void writePixel(int i, PixelColor c);
};

#endif /* ADAPTERS_OUT_PRESENTER_H_ */
