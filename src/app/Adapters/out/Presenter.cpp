/*
 * Presenter.cpp
 *
 *  Created on: Feb 2, 2026
 *      Author: alex
 */

#include "Presenter.h"

Presenter::Presenter(FieldHighlightInterface *highlight,
						FlashDriverInterface *flash)
{
    this->highlight = highlight;
    this->flash = flash;
    if (flash->isInit()) flash->read(300, (uint8_t*)&curThemeId, sizeof(curThemeId));
    loadTheme(curThemeId);
    clearAllHighlights();
}

Presenter::~Presenter()
{

}

// Быстрая целочисленная генерация пульсаций от 0 до max_val
uint8_t Presenter::getWave(uint32_t period, uint8_t max_val)
{
    uint32_t t = tick % period;
    uint32_t half = period / 2;
    if (t < half) return (t * max_val) / half;
    return ((period - t) * max_val) / half;
}

void Presenter::run()
{
    if (fields == nullptr || highlight == nullptr) return;

    tick++; // Увеличиваем счетчик кадров

    // Генерируем анимационные пульсации
    uint8_t pulseSlow = getWave(200, 255); // Плавное мигание (период 200 тиков)
    uint8_t pulseFast = getWave(64, 255);  // Быстрое мигание (период 64 тика)

    for (int i = 0; i < 64; i++)
    {
        uint8_t r = 0, g = 0, b = 0; // Базовый цвет (черный)

        switch (gameState) {
			// ==========================================
			// РЕЖИМ 1: РАССТАНОВКА ФИГУР (NO_POSITION)
			// ==========================================
			case NO_POSITION:
			{
	            // Плавно переливаем стартовые позиции
	            if (i < 16) {
	                if (fields->getField(i) == Fields::white) {
						r = theme.initPosCorrect.r;
						g = theme.initPosCorrect.g;
						b = theme.initPosCorrect.b;
	                }else {
						r = (theme.initPosFalse.r * pulseSlow) / 255;
						g = (theme.initPosFalse.g * pulseSlow) / 255;
						b = (theme.initPosFalse.b * pulseSlow) / 255;
	                }
	            }
	            // На центре не должно быть фигур
	            else if (i > 15 && i < 48) {
	                if (fields->getField(i) != Fields::none) {
						r = (theme.initPosFalse.r * pulseSlow) / 255;
						g = (theme.initPosFalse.g * pulseSlow) / 255;
						b = (theme.initPosFalse.b * pulseSlow) / 255;
	                }
	            }
	            else if (i >= 48) {
	                if (fields->getField(i) == Fields::black) {
						r = theme.initPosCorrect.r;
						g = theme.initPosCorrect.g;
						b = theme.initPosCorrect.b;

	                } else {
						r = (theme.initPosFalse.r * pulseSlow) / 255;
						g = (theme.initPosFalse.g * pulseSlow) / 255;
						b = (theme.initPosFalse.b * pulseSlow) / 255;
	                }
	            }
	            // Eсли игра сбросилась сбрасываем состояния
	            activeFigureIdx = -1;
	            helpFromIdx = -1; helpToIdx = -1;
	            lastFromIdx = -1; lastToIdx = -1;
	            lastInvalidFromIdx = -1; lastInvalidToIdx = -1;
				break;
			}
			// ==========================================
			// РЕЖИМ 2: ИГРА (GAME)
			// ==========================================
			case GAME:
			{

				// СЛОЙ 1: Последний ход и Ход-подсказка (самый низкий приоритет)
				if (i == lastFromIdx || i == lastToIdx) {// Последний ход
					r = theme.lastMove.r;
					g = theme.lastMove.g;
					b = theme.lastMove.b;
				}

				if (i == helpFromIdx || i == helpToIdx) { // Ход-подсказка
					r = (theme.help.r * pulseSlow) / 255;
					g = (theme.help.g * pulseSlow) / 255;
					b = (theme.help.b * pulseSlow) / 255;
				}

				// СЛОЙ 4: Невадидный ход
				if (i == lastInvalidFromIdx  || i == lastInvalidToIdx) {

					r = (theme.error.r * pulseFast) / 255;
					g = (theme.error.g * pulseFast) / 255;
					b = (theme.error.b * pulseFast) / 255;
				}

				// СЛОЙ 5: ШАХ! (Наивысший приоритет)
				if (checkKingIdx != -1)  {
					if (i == checkKingIdx) {	// Король
						r = (theme.checkKing.r * pulseFast) / 255;
						g = (theme.checkKing.g * pulseFast) / 255;
						b = (theme.checkKing.b * pulseFast) / 255;
					}
					if (i == checkAttackerIdx) { // Атакующий
						r = (theme.checkAttack.r * pulseFast) / 255;
						g = (theme.checkAttack.g * pulseFast) / 255;
						b = (theme.checkAttack.b * pulseFast) / 255;
					}
				}
				// СЛОЙ 2: Доступные ходы
				if (availableNormalMoves[i]) {
					r = theme.avalibleMove.r;
					g = theme.avalibleMove.g;
					b = theme.avalibleMove.b;
				}
				if (availableAttackMoves[i]) { // Клетка для взятия
					r = (theme.moveAttack.r * pulseSlow) / 255;
					g = (theme.moveAttack.g * pulseSlow) / 255;
					b = (theme.moveAttack.b * pulseSlow) / 255;
				}

				// СЛОЙ 3: Активная фигура (которую подняли)
				if (i == activeFigureIdx) {
					r = (theme.activePiece.r * pulseSlow) / 255;
					g = (theme.activePiece.g * pulseSlow) / 255;
					b = (theme.activePiece.b * pulseSlow) / 255;
				}

				break;
			}
			// ==========================================
			// РЕЖИМ 3: КОНЕЦ ИГРЫ (GAME_OVER)
			// ==========================================
			case GAME_OVER:
			{
	            if (winningSide == 8 || winningSide == 16)
	            {
	                // МАТ // Победитель и проигравший пульсируют
	            	if (winningSide == 8)
	            	{
	            		if (i == posWhiteKing) {
							r = (theme.mateKingWin.r * pulseSlow) / 255;
							g = (theme.mateKingWin.g * pulseSlow) / 255;
							b = (theme.mateKingWin.b * pulseSlow) / 255;
	            		}
	            		if (i == posBlackKing) {
							r = (theme.mateKingLose.r * pulseSlow) / 255;
							g = (theme.mateKingLose.g * pulseSlow) / 255;
							b = (theme.mateKingLose.b * pulseSlow) / 255;
	            		}
	            	}
	            	else
	            	{
	            		if (i == posWhiteKing) {
							r = (theme.mateKingLose.r * pulseSlow) / 255;
							g = (theme.mateKingLose.g * pulseSlow) / 255;
							b = (theme.mateKingLose.b * pulseSlow) / 255;
	            		}
	            		if (i == posBlackKing) {
							r = (theme.mateKingWin.r * pulseSlow) / 255;
							g = (theme.mateKingWin.g * pulseSlow) / 255;
							b = (theme.mateKingWin.b * pulseSlow) / 255;
	            		}
	            	}
	            }
	            else
	            {
	                // ПАТ (Stalemate) - Оба короля плавно мигают
	                if (i == posWhiteKing || i == posBlackKing) {
						r = (theme.stalemate.r * pulseSlow) / 255;
						g = (theme.stalemate.g * pulseSlow) / 255;
						b = (theme.stalemate.b * pulseSlow) / 255;
	                }
	            }
				break;
			}
			default:
				break;
		}
        // Отправляем вычисленный цвет на ленту
        highlight->setFieldHighlight(i, r, g, b);
    }
}

void Presenter::messege(const std::string &message)
{
	if (message.find("THEME:") != std::string::npos)
	{
		int param = 0;
	    if (sscanf(message.c_str(), "THEME:%d", &param) == 1)
	    {
	    	if (param >= 0 && param <= 11)
	    	{
	    		curThemeId = param;
	    		loadTheme(curThemeId);
	    		if (flash->isInit()) flash->write(300, (uint8_t*)&curThemeId, sizeof(curThemeId));
	    		printf("Theme set:%d\n",curThemeId);
	    	}
	    }
	}
}

// ==========================================
// СЕТТЕРЫ СОСТОЯНИЙ (Связь с Interactor)
// ==========================================

void Presenter::setFields(Fields &fields) { this->fields = &fields; }
void Presenter::setMode(GameState state) { this->gameState = state; }
void Presenter::setActiveSide(MoveTurn side) { this->move = side; }
void Presenter::setGameMode(GameMode mode) { this->mode = mode; }

void Presenter::clearAllHighlights()
{
    activeFigureIdx = -1;
    helpFromIdx = -1; helpToIdx = -1;
    checkAttackerIdx = -1; checkKingIdx = -1;
	lastInvalidFromIdx = -1;
	lastInvalidToIdx = -1;
    memset(availableNormalMoves, false, sizeof(availableNormalMoves));
    memset(availableAttackMoves, false, sizeof(availableAttackMoves));
}

void Presenter::setLastMove(int from, int to)
{
    lastFromIdx = from;
    lastToIdx = to;
	lastInvalidFromIdx = -1;
	lastInvalidToIdx = -1;

    clearAllHighlights(); // Сбрасываем выделение активной фигуры при новом ходе
}

void Presenter::setInvalidLastMove(int from, int to)
{
    clearAllHighlights(); // Сбрасываем выделение активной фигуры при новом ходе
	if (lastInvalidFromIdx == to && lastInvalidToIdx == from) // поставили на место
	{
		lastInvalidToIdx=-1;
		lastInvalidFromIdx=-1;
		activeFigureIdx = -1;
		return;
	}
    lastInvalidFromIdx = from;
    lastInvalidToIdx = to;
    activeFigureIdx = -1;
}

void Presenter::setHelpMove(int from, int to)
{
    helpFromIdx = from; helpToIdx = to;
}

void Presenter::setActiveFigure(int index)
{
    activeFigureIdx = index;
    if (index == lastInvalidFromIdx || index == -1)
	{
		lastInvalidFromIdx = -1;
		lastInvalidToIdx = -1;
	}
}

void Presenter::addAvailableMove(int index, bool isAttack)
{
    if (index >= 0 && index < 64) {
        if (isAttack) availableAttackMoves[index] = true;
        else          availableNormalMoves[index] = true;
    }
}

void Presenter::setCheck(int posAttack, int posKing)
{
    checkAttackerIdx = posAttack;
    checkKingIdx = posKing;
}

void Presenter::setCheckmate(int winningSide, int whiteKingPos, int blackKingPos)
{
    this->winningSide = winningSide;
    this->posWhiteKing = whiteKingPos;
    this->posBlackKing = blackKingPos;
    setMode(GAME_OVER);
}

void Presenter::setStalemate(int whiteKingPos, int blackKingPos)
{
    this->winningSide = 0; // 0 = Ничья
    this->posWhiteKing = whiteKingPos;
    this->posBlackKing = blackKingPos;
    setMode(GAME_OVER);
}
