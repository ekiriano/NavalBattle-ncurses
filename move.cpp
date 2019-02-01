#include "move.h"


int Move::getPlayerTurn() {
	return playerTurn;
}
int Move::getX() {
	return x;
}
int Move::getY() {
	return y;
}
void Move::setMove(int playerTurn, int x, int y) {
	this->playerTurn = playerTurn;
	this->x = x;
	this->y = y;
}

