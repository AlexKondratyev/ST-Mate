/*
 * LoadTheme.cpp
 *
 *  Created on: Mar 18, 2026
 *      Author: alex
 */

#include "Presenter.h"

//Code	Theme Name
//0		Default - standard theme (default)
//1		Neon Night - bright neon colours
//2		Forest Glade - natural green and brown shades
//3		Ocean Breeze - marine theme with blue tones
//4		Royal Purple - noble purple shades
//5		Autumn Sunset - warm autumn colours
//6		Arctic Ice - cool pastel tones
//7		Desert Sand - sandy and earthy shades
//8		Candy Land - bright candy colours
//9		Midnight Oil - dark saturated tones
//10	Spring Meadow - fresh spring colours
//11	Classic Elegance - restrained classic palette
void Presenter::loadTheme(int themeId)
{
	switch (themeId) {
		default:
		case 0:
			theme = ChessTheme({});
			break;
		case 1: //Neon Night
			theme = ChessTheme{
				{0, 255, 255},     // init correct
			    {255, 0, 255},     // init false
			    {0, 255, 255},     // avalibleMove
			    {255, 0, 255},     // moveAttack
			    {255, 255, 0},     // lastMove
			    {0, 255, 0},       // help
			    {0, 255, 255},   // activePiece
			    {255, 0, 0},       // error
			    {255, 0, 0},       // checkKing
			    {0, 0, 255},       // checkAttack
				{0, 255, 255},     // mateKingWin
				{255, 0, 255},     // mateKingLose
			    {0, 128, 128}      // stalemate
			};
			break;

		case 2: //Forest Glade
			theme = ChessTheme{
				{34, 139, 34},      // init correct
			    {178, 34, 34},      // init false
			    {144, 238, 144},    // avalibleMove
			    {255, 99, 71},      // moveAttack
			    {255, 215, 0},      // lastMove
			    {70, 130, 180},     // help
			    {255, 250, 240},    // activePiece
			    {220, 20, 60},      // error
			    {178, 34, 34},      // checkKing
			    {25, 25, 112},      // checkAttack
			    {50, 205, 50},      // mateKingWin
			    {139, 69, 19},      // mateKingLose
			    {64, 224, 208}      // stalemate
			};
			break;

		case 3: //Ocean Breeze
			theme = ChessTheme{
				{0, 191, 255},       // init correct
			    {255, 69, 0},        // init false
			    {135, 206, 235},     // avalibleMove
			    {255, 140, 0},       // moveAttack
			    {255, 255, 224},     // lastMove
			    {30, 144, 255},      // help
			    {240, 248, 255},     // activePiece
			    {255, 0, 0},         // error
			    {255, 69, 0},        // checkKing
			    {0, 0, 128},         // checkAttack
			    {0, 250, 154},       // mateKingWin
			    {70, 130, 180},      // mateKingLose
			    {175, 238, 238}      // stalemate
			};
			break;

		case 4: //Royal Purple
			theme = ChessTheme{
				{147, 112, 219},     // init correct
			    {255, 0, 0},         // init false
			    {216, 191, 216},     // avalibleMove
			    {255, 105, 180},     // moveAttack
			    {255, 215, 0},       // lastMove
			    {123, 104, 238},     // help
			    {255, 240, 245},     // activePiece
			    {178, 34, 34},       // error
			    {220, 20, 60},       // checkKing
			    {75, 0, 130},        // checkAttack
			    {218, 112, 214},     // mateKingWin
			    {106, 90, 205},      // mateKingLose
			    {221, 160, 221}      // stalemate
			};
			break;

		case 5: //Autumn Sunset
			theme = ChessTheme{
				{255, 140, 0},       // init correct
			    {178, 34, 34},       // init false
			    {255, 218, 185},     // avalibleMove
			    {255, 99, 71},       // moveAttack
			    {255, 215, 0},       // lastMove
			    {210, 105, 30},      // help
			    {255, 228, 196},     // activePiece
			    {139, 0, 0},         // error
			    {255, 69, 0},        // checkKing
			    {139, 69, 19},       // checkAttack
			    {154, 205, 50},      // mateKingWin
			    {160, 82, 45},       // mateKingLose
			    {205, 133, 63}       // stalemate
			};
			break;

		case 6: //Arctic Ice
			theme = ChessTheme{
				{173, 216, 230},     // init correct
			    {255, 182, 193},     // init false
			    {240, 248, 255},     // avalibleMove
			    {255, 228, 225},     // moveAttack
			    {224, 255, 255},     // lastMove
			    {70, 130, 180},      // help
			    {255, 250, 250},     // activePiece
			    {255, 160, 122},     // error
			    {255, 182, 193},     // checkKing
			    {25, 25, 112},       // checkAttack
			    {152, 251, 152},     // mateKingWin 
			    {176, 224, 230},     // mateKingLose
			    {175, 238, 238}      // stalemate
			};
			break;

		case 7: //Desert Sand
			theme = ChessTheme{
				{210, 180, 140},     // init correct
			    {205, 92, 92},       // init false
			    {245, 222, 179},     // avalibleMove
			    {222, 184, 135},     // moveAttack
			    {255, 235, 205},     // lastMove
			    {139, 90, 43},       // help
			    {255, 239, 213},     // activePiece
			    {165, 42, 42},       // error
			    {205, 92, 92},       // checkKing
			    {101, 67, 33},       // checkAttack
			    {107, 142, 35},      // mateKingWin
			    {160, 82, 45},       // mateKingLose
			    {188, 143, 143}      // stalemate
			};
			break;

		case 8: //Candy Land
			theme = ChessTheme{
				{255, 182, 193},     // init correct
			    {255, 105, 180},     // init false
			    {255, 218, 185},     // avalibleMove
			    {255, 182, 193},     // moveAttack
			    {255, 255, 224},     // lastMove
			    {176, 224, 230},     // help
			    {255, 250, 205},     // activePiece
			    {255, 99, 71},       // error
			    {255, 20, 147},      // checkKing
			    {135, 206, 250},     // checkAttack
			    {152, 251, 152},     // mateKingWin
			    {221, 160, 221},     // mateKingLose
			    {255, 228, 225}      // stalemate
			};
			break;

		case 9: //Midnight Oil
			theme = ChessTheme{
				{100, 149, 237},     // init correct
			    {220, 20, 60},       // init false
			    {25, 25, 112},       // avalibleMove
			    {139, 0, 0},         // moveAttack
			    {255, 215, 0},       // lastMove
			    {70, 130, 180},      // help
			    {255, 255, 255},     // activePiece
			    {255, 0, 0},         // error
			    {178, 34, 34},       // checkKing
			    {0, 0, 139},         // checkAttack
			    {50, 205, 50},       // mateKingWin
			    {85, 107, 47},       // mateKingLose
			    {72, 61, 139}        // stalemate
			};
			break;

		case 10: //Spring Meadow
			theme = ChessTheme{
				{124, 252, 0},       // init correct
			    {255, 69, 0},        // init false
			    {144, 238, 144},     // avalibleMove
			    {255, 160, 122},     // moveAttack
			    {255, 255, 0},       // lastMove
			    {0, 191, 255},       // help
			    {245, 245, 220},     // activePiece
			    {255, 0, 0},         // error
			    {255, 140, 0},       // checkKing
			    {30, 144, 255},      // checkAttack
			    {50, 205, 50},       // mateKingWin
			    {107, 142, 35},      // mateKingLose
			    {64, 224, 208}       // stalemate
			};
			break;

		case 11: //Classic Elegance
			theme = ChessTheme{
				{0, 100, 0},          // init correct
			    {139, 0, 0},          // init false
			    {211, 211, 211},      // avalibleMove
			    {255, 160, 122},      // moveAttack
			    {255, 255, 224},      // lastMove
			    {70, 130, 180},       // help
			    {255, 250, 240},      // activePiece
			    {178, 34, 34},        // error
			    {220, 20, 60},        // checkKing
			    {25, 25, 112},        // checkAttack
			    {46, 139, 87},        // mateKingWin
			    {139, 69, 19},        // mateKingLose
			    {112, 128, 144}       // stalemate
			};
			break;
	}
}
