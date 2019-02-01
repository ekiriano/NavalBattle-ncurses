#include "map.h"


Map::Map() {
	init(-1);
}


void Map::init(int value) {
	for(int i=0; i<50; i++)
        for(int j=0; j<50; j++){
            cell[i][j] = value;
        }
}
void Map::setCell(int x, int y, int value) {cell[x][y] = value;}
int Map::getCell(int x, int y) {return cell[x][y];}