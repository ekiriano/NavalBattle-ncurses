#include "ship.h"

Ship::Ship() {
	init();
}
void Ship::init() {
	size = 0;
	color = WBLUE;
}
void Ship::setSize(int size) {this->size = size;}
void Ship::setColor(Color color) {this->color = color;}
void Ship::setModelMatrix(Map &modelMatrix) {this->modelMatrix = modelMatrix;}
void Ship::setDim(int dimX, int dimY){ this->dimX = dimX; this->dimY = dimY;}	

int Ship::getDimX() {return dimX;}
int Ship::getDimY() {return dimY;}
Map& Ship::getModelMatrix() {return modelMatrix;}
int Ship::getSize() {return size;}
Color Ship::getColor() {return color;}

void Ship::rotateLeft() {

	//+90 degrees
	//transpose then reverse rows
	transposeModel();
	reverseModelRows();
}
void Ship::rotateRight() {
	//-90 degrees
	//reverse each row
	//transpose	
	reverseModelRows();
	transposeModel();
}

//reverses the model modelMatrix
void Ship::reverseModelRows() {
	int temp;
	for(int i=0; i<dimX/2; i++)	{
		for(int j=0; j<dimY; j++) {	
			temp = modelMatrix.getCell(dimX-1-i, j);
			modelMatrix.setCell(dimX-1-i, j, modelMatrix.getCell(i, j));
			modelMatrix.setCell(i, j, temp);
		}
	}
}
//does the transpose of the modelMatrix
void Ship::transposeModel() {
	Map tempMap;
	for(int i=0; i<dimX; i++) {
		for(int j=0; j<dimY; j++){
			tempMap.setCell(j, i, modelMatrix.getCell(i, j));
		}
	}

	//variable swap
	int temp = dimY;
	dimY = dimX;
	dimX = temp;

	modelMatrix.init(0);
	for(int i=0; i<dimX; i++) {
		for(int j=0; j<dimY; j++) {
			modelMatrix.setCell(i, j, tempMap.getCell(i, j));
		}
	}
}
