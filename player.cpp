#include "player.h"

Player::Player() {
	init(0);
};

//this init function sets to default all the parameters used ingame for the player
void Player::init(int windowDim) {
	selectionPosition = 0;	
	for(int i=0; i<5; i++)
		selectionState[i] = false;
	map.init(-1);
	tempMap.init(-1);
    mask.init(-1);
    score.setValue(windowDim * windowDim);
}


bool Player::validateTempMap(int windowDim) {
    /*
     * We first check to see if the ship on the tempMap does not overlap a
     * ship on the map.
     */    
	for(int i=0; i<windowDim; i++) {
        for(int j=0; j<windowDim; j++) {
            if((tempMap.getCell(i,j)!=-1) && (map.getCell(i, j) != -1)) {                               
                return false;
            }
        }
    }

    //if it is ok, then we copy the tampMap to the map.
    for(int i=0; i<windowDim; i++) {
        for(int j=0; j<windowDim; j++) {
            if(tempMap.getCell(i,j)!=-1) {                
                map.setCell(i,j,tempMap.getCell(i, j));                
            }
        }
    }

    return true;
}
//if there is no ship to be selected we proceed to the next stage of the game.
bool Player::isSelectionReady() {
    for(int i=0; i<5; i++)
        if(selectionState[i] == false) { return false; }
    return true;
}
/* 
 * the map used in the actual game, to hide the parts of the ship.
 * 0 means that it hides. -1 means it does not hide
 */
void Player::generateMask() {
    mask.init(0);
}

void Player::setPlayerName(std::string playerName) {this->playerName = playerName;}
void Player::setSelectionState(int pos, bool value) {selectionState[pos] = value;}
void Player::setSelectionPosition(int selectionPosition) { this->selectionPosition = selectionPosition;}
void Player::setMap(Map &map) {this->map = map;}
void Player::setTempMap(Map &tempMap) {this->tempMap = tempMap;}
void Player::setMask(Map &mask) {this->mask = mask;}
void Player::setComputerControlled(bool computerControlled) {this->computerControlled = computerControlled;}
void Player::setScore(int value) {score.setValue(value);}

std::string Player::getPlayerName() {return playerName;}
int Player::getSelectionState(int pos) {return selectionState[pos];}
int Player::getSelectionPosition() { return selectionPosition;}
Map& Player::getMap() {return map;}
Map& Player::getTempMap() {return tempMap;}
Map& Player::getMask() { return mask;}
bool Player::getComputerControlled() {return computerControlled;}
int Player::getScore() {return score.getValue();}



