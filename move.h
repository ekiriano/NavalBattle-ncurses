#ifndef __MOVE_H
#define __MOVE_H

#include "window.h"

/*
 * A move is represented by the coordinates of the cell selected by the player, whose
 * ID will be determined by the playerTurn variable.
 */
class Move {
	int playerTurn, x, y;
	public:
		int getPlayerTurn();
		int getX();
		int getY();
		void setMove(int playerTurn, int x, int y);
};


#endif