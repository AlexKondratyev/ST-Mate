/*
 * LoadTheme.cpp
 *
 *  Created on: Mar 18, 2026
 *      Author: alex
 */

#include "Presenter.h"

//	0 Default - стандартная тема (по-умолчанию)
//	1 Neon Night - яркие неоновые цвета
//	2 Forest Glade - природные зеленые и коричневые оттенки
//	3 Ocean Breeze - морская тема с синими тонами
//	4 Royal Purple - благородные фиолетовые оттенки
//	5 Autumn Sunset - теплые осенние цвета
//	6 Arctic Ice - холодные пастельные тона
//	7 Desert Sand - песочные и земляные оттенки
//	8 Candy Land - яркие конфетные цвета
//	9 Midnight Oil - темные насыщенные тона
//	10 Spring Meadow - свежие весенние цвета
// 11 Classic Elegance - сдержанная классическая гамма
void Presenter::loadTheme(int themeId)
{
	switch (themeId) {
		default:
		case 0:
			theme = ChessTheme({});
			break;
		case 1: //Neon Night
			theme = ChessTheme{
				{0, 255, 255},     // init correct (циан)
			    {255, 0, 255},     // init false (маджента)
			    {0, 255, 255},     // avalibleMove (циан)
			    {255, 0, 255},     // moveAttack (маджента)
			    {255, 255, 0},     // lastMove (желтый)
			    {0, 255, 0},       // help (зеленый)
			    {0, 255, 255},   // activePiece (белый)
			    {255, 0, 0},       // error (красный)
			    {255, 0, 0},       // checkKing (красный)
			    {0, 0, 255},       // checkAttack (синий)
				{0, 255, 255},     // mateKingWin (зеленый)
				{255, 0, 255},     // mateKingLose (темно-красный)
			    {0, 128, 128}      // stalemate (бирюзовый)
			};
			break;

		case 2: //Forest Glade
			theme = ChessTheme{
				{34, 139, 34},      // init correct (лесной зеленый)
			    {178, 34, 34},      // init false (кирпичный)
			    {144, 238, 144},    // avalibleMove (светло-зеленый)
			    {255, 99, 71},      // moveAttack (томатный)
			    {255, 215, 0},      // lastMove (золотой)
			    {70, 130, 180},     // help (стальной синий)
			    {255, 250, 240},    // activePiece (цвет слоновой кости)
			    {220, 20, 60},      // error (малиновый)
			    {178, 34, 34},      // checkKing (кирпичный)
			    {25, 25, 112},      // checkAttack (полуночный синий)
			    {50, 205, 50},      // mateKingWin (лаймовый)
			    {139, 69, 19},      // mateKingLose (коричневый)
			    {64, 224, 208}      // stalemate (бирюзовый)
			};
			break;

		case 3: //Ocean Breeze
			theme = ChessTheme{
				{0, 191, 255},       // init correct (глубокий небесный)
			    {255, 69, 0},        // init false (оранжево-красный)
			    {135, 206, 235},     // avalibleMove (небесный)
			    {255, 140, 0},       // moveAttack (темно-оранжевый)
			    {255, 255, 224},     // lastMove (светло-желтый)
			    {30, 144, 255},      // help (синий)
			    {240, 248, 255},     // activePiece (алюминиевый)
			    {255, 0, 0},         // error (красный)
			    {255, 69, 0},        // checkKing (оранжево-красный)
			    {0, 0, 128},         // checkAttack (темно-синий)
			    {0, 250, 154},       // mateKingWin (светло-зеленый)
			    {70, 130, 180},      // mateKingLose (стальной синий)
			    {175, 238, 238}      // stalemate (бледно-бирюзовый)
			};
			break;

		case 4: //Royal Purple
			theme = ChessTheme{
				{147, 112, 219},     // init correct (средний пурпурный)
			    {255, 0, 0},         // init false (красный)
			    {216, 191, 216},     // avalibleMove (светло-сливовый)
			    {255, 105, 180},     // moveAttack (горячий розовый)
			    {255, 215, 0},       // lastMove (золотой)
			    {123, 104, 238},     // help (средний синий)
			    {255, 240, 245},     // activePiece (лавандовый румянец)
			    {178, 34, 34},       // error (кирпичный)
			    {220, 20, 60},       // checkKing (малиновый)
			    {75, 0, 130},        // checkAttack (индиго)
			    {218, 112, 214},     // mateKingWin (орхидея)
			    {106, 90, 205},      // mateKingLose (сине-фиолетовый)
			    {221, 160, 221}      // stalemate (сливовый)
			};
			break;

		case 5: //Autumn Sunset
			theme = ChessTheme{
				{255, 140, 0},       // init correct (темно-оранжевый)
			    {178, 34, 34},       // init false (кирпичный)
			    {255, 218, 185},     // avalibleMove (персиковый)
			    {255, 99, 71},       // moveAttack (томатный)
			    {255, 215, 0},       // lastMove (золотой)
			    {210, 105, 30},      // help (шоколадный)
			    {255, 228, 196},     // activePiece (бежевый)
			    {139, 0, 0},         // error (темно-красный)
			    {255, 69, 0},        // checkKing (оранжево-красный)
			    {139, 69, 19},       // checkAttack (коричневый)
			    {154, 205, 50},      // mateKingWin (желто-зеленый)
			    {160, 82, 45},       // mateKingLose (сиена)
			    {205, 133, 63}       // stalemate (перу)
			};
			break;

		case 6: //Arctic Ice
			theme = ChessTheme{
				{173, 216, 230},     // init correct (светло-голубой)
			    {255, 182, 193},     // init false (светло-розовый)
			    {240, 248, 255},     // avalibleMove (алюминиевый)
			    {255, 228, 225},     // moveAttack (розоватый)
			    {224, 255, 255},     // lastMove (светло-циан)
			    {70, 130, 180},      // help (стальной синий)
			    {255, 250, 250},     // activePiece (белый)
			    {255, 160, 122},     // error (светло-лососевый)
			    {255, 182, 193},     // checkKing (светло-розовый)
			    {25, 25, 112},       // checkAttack (полуночный синий)
			    {152, 251, 152},     // mateKingWin (бледно-зеленый)
			    {176, 224, 230},     // mateKingLose (голубой)
			    {175, 238, 238}      // stalemate (бледно-бирюзовый)
			};
			break;

		case 7: //Desert Sand
			theme = ChessTheme{
				{210, 180, 140},     // init correct (песочный)
			    {205, 92, 92},       // init false (индийский красный)
			    {245, 222, 179},     // avalibleMove (пшеничный)
			    {222, 184, 135},     // moveAttack (светлый дуб)
			    {255, 235, 205},     // lastMove (миндальный)
			    {139, 90, 43},       // help (кожаный)
			    {255, 239, 213},     // activePiece (папайя)
			    {165, 42, 42},       // error (коричневый)
			    {205, 92, 92},       // checkKing (индийский красный)
			    {101, 67, 33},       // checkAttack (кофейный)
			    {107, 142, 35},      // mateKingWin (оливковый)
			    {160, 82, 45},       // mateKingLose (сиена)
			    {188, 143, 143}      // stalemate (розово-коричневый)
			};
			break;

		case 8: //Candy Land
			theme = ChessTheme{
				{255, 182, 193},     // init correct (светло-розовый)
			    {255, 105, 180},     // init false (горячий розовый)
			    {255, 218, 185},     // avalibleMove (персиковый)
			    {255, 182, 193},     // moveAttack (светло-розовый)
			    {255, 255, 224},     // lastMove (светло-желтый)
			    {176, 224, 230},     // help (голубой)
			    {255, 250, 205},     // activePiece (лимонный)
			    {255, 99, 71},       // error (томатный)
			    {255, 20, 147},      // checkKing (ярко-розовый)
			    {135, 206, 250},     // checkAttack (светлый небесный)
			    {152, 251, 152},     // mateKingWin (бледно-зеленый)
			    {221, 160, 221},     // mateKingLose (сливовый)
			    {255, 228, 225}      // stalemate (розоватый)
			};
			break;

		case 9: //Midnight Oil
			theme = ChessTheme{
				{100, 149, 237},     // init correct (васильковый)
			    {220, 20, 60},       // init false (малиновый)
			    {25, 25, 112},       // avalibleMove (полуночный синий)
			    {139, 0, 0},         // moveAttack (темно-красный)
			    {255, 215, 0},       // lastMove (золотой)
			    {70, 130, 180},      // help (стальной синий)
			    {255, 255, 255},     // activePiece (белый)
			    {255, 0, 0},         // error (красный)
			    {178, 34, 34},       // checkKing (кирпичный)
			    {0, 0, 139},         // checkAttack (темно-синий)
			    {50, 205, 50},       // mateKingWin (лаймовый)
			    {85, 107, 47},       // mateKingLose (темно-оливковый)
			    {72, 61, 139}        // stalemate (темно-фиолетовый)
			};
			break;

		case 10: //Spring Meadow
			theme = ChessTheme{
				{124, 252, 0},       // init correct (лаймовый)
			    {255, 69, 0},        // init false (оранжево-красный)
			    {144, 238, 144},     // avalibleMove (светло-зеленый)
			    {255, 160, 122},     // moveAttack (светло-лососевый)
			    {255, 255, 0},       // lastMove (желтый)
			    {0, 191, 255},       // help (глубокий небесный)
			    {245, 245, 220},     // activePiece (бежевый)
			    {255, 0, 0},         // error (красный)
			    {255, 140, 0},       // checkKing (темно-оранжевый)
			    {30, 144, 255},      // checkAttack (синий)
			    {50, 205, 50},       // mateKingWin (лаймовый)
			    {107, 142, 35},      // mateKingLose (оливковый)
			    {64, 224, 208}       // stalemate (бирюзовый)
			};
			break;

		case 11: //Classic Elegance
			theme = ChessTheme{
				{0, 100, 0},          // init correct (темно-зеленый)
			    {139, 0, 0},          // init false (темно-красный)
			    {211, 211, 211},      // avalibleMove (светло-серый)
			    {255, 160, 122},      // moveAttack (светло-лососевый)
			    {255, 255, 224},      // lastMove (светло-желтый)
			    {70, 130, 180},       // help (стальной синий)
			    {255, 250, 240},      // activePiece (цвет слоновой кости)
			    {178, 34, 34},        // error (кирпичный)
			    {220, 20, 60},        // checkKing (малиновый)
			    {25, 25, 112},        // checkAttack (полуночный синий)
			    {46, 139, 87},        // mateKingWin (морской зеленый)
			    {139, 69, 19},        // mateKingLose (коричневый)
			    {112, 128, 144}       // stalemate (серо-голубой)
			};
			break;
	}
}
