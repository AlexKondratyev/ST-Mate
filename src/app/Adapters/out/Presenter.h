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
    Color initPosCorrect{0,255,0};	// Цвет начальной позиции правильно (Зелйный)
    Color initPosFalse{255,0,0};	// Цвет начальной позиции не верно (красный)
    Color avalibleMove{128,128,128};   // Доступный ход (Тускло белый)
    Color moveAttack{255,0,0};    	// Клетка для взятия (Красный)
    Color lastMove{128,128,0};      	// Последний ход (Желтый)
    Color help{0,128,255};          	// Подсказка (Голубой)
    Color activePiece{255,255,255};   	// Фигура в руке (Белый)
    Color error{255,0,0};         	// Ошибка (Красный)
    Color checkKing{255,0,0};       // Шах короля (Красный)
    Color checkAttack{0,0,255};     // Шах Атакующий (Синий)
    Color mateKingWin{0,255,0};     // Мат Короля победившего (Красный)
    Color mateKingLose{255,0,0};    // Мат Короля пороигравшего (Зелёный)
    Color stalemate{0,255,255};		// Пат (Голубой)
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

    // Внутренние состояния для анимаций
    uint32_t tick = 0; // Счетчик кадров для анимации

    int activeFigureIdx = -1;
    int helpFromIdx = -1, helpToIdx = -1;
    int lastFromIdx = -1, lastToIdx = -1;
    int lastInvalidFromIdx = -1, lastInvalidToIdx = -1;

    int checkAttackerIdx = -1, checkKingIdx = -1;

    int winningSide = 0;
    int posWhiteKing = -1, posBlackKing = -1;

    // Массивы для доступных ходов (64 клетки)
    bool availableNormalMoves[64] = {false};
    bool availableAttackMoves[64] = {false};

    void loadTheme(int themeId); // 0 - Classic
    // Вспомогательная функция генерации волны для пульсации
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
