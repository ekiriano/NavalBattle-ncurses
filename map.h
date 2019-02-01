#ifndef __MAP_H
#define __MAP_H

#include "window.h"

/*
 * The map is defined by a matrix of cells. The value of each cell is equal to a color ID.
 * We have 5 color IDs, whose range is from 0-4. -1 means that the cell is not part
 * of a ship. 2 means that the cell is part of the 2nd ship.
 */


class Map {

	int cell[50][50];
	public: 
		
		Map();   	
    	void init(int value);
    	void setCell(int x, int y, int value);
    	int getCell(int x, int y);
};

#endif