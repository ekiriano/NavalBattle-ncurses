#ifndef __SHIP_H
#define __SHIP_H

#include "window.h"
#include "map.h"

class Ship 
{
	int size;
	Color color;
	Map modelMatrix; 
	//the codification is the following 
	//modelMatrix[1][2] == 0 means that cell[1][2] is empty;
	//modelMatrix[1][2] == 1 means that cell is full;
	
	int dimX, dimY;

	public:
		Ship();
		void init();

		void setSize(int size);
		void setColor(Color color);
		void setDim(int dimX, int dimY);
		void setModelMatrix(Map &modelMatrix);

		Map& getModelMatrix();
		Color getColor();
		int getSize();		
		int getDimX();
		int getDimY();

		//Mathematical-based function for rotating/translating the modelMatrix.
		void reverseModelRows();
		void transposeModel();
		void rotateLeft();
		void rotateRight();
};
#endif
