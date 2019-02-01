#include "menu.h"

/*
*The names are suggestive for their purpose. On the first line, in most of
*the cases, the name of the (sub)menu is written
*/

void Menu::CMDMainMenu() {
    cmdMessage(0, "Main Menu", true);
    cmdMessage(1, "Press SPACE to start a new game!           Press L to load a game from File!", false);    
    cmdMessage(2, "Press S to see highscores!                 Press Q to quit!", false);
}

void Menu::CMDShipSelectMenu(std::string playerName) {
    cmdMessage(0, "Ship Select Menu", true);
    cmdMessage(1, playerName + ", use arrows to move and SPACE to select a ship from the selector.", false);    
    cmdMessage(2, "Then place it on the map, using arrows and SPACE. Rotate with r/l!", false);
}
void Menu::CMDInfo() {
    cmdMessage(0, "Help: Place ships on your map, then find opponent's ships.", true);
    cmdMessage(1, "Use indications from the command line through the game!", false);    
    cmdMessage(2, "Game Version 1.0 | author : Abed Quail.  press c to continue...", false);
}
void Menu::CMDFoundShip() {
    cmdMessage(1, "YOU FOUND A SHIP!! YAAAAAS! press c to contiue...", true);  
}

void Menu::cmdMessage(int row, std::string msg, bool clear) {
	int space = 1;
    if(clear) cmd->clear();
    if(!row) space = 40 - msg.length() / 2;
    cmd->print(space, row , msg);
}
void Menu::init(int pos) {	
    cmd = new Window(3 ,78 , 0 , pos);
    cmd->setCouleurFenetre(WRED);
    cmd->setCouleurBordure(WRED);
}
//used when requring name info from player
void Menu::CMDAskForName(std::string playerID) {
    cmdMessage(0, "I want a name for " + playerID + "! NOW! (press space when ready)", true);
}
void Menu::advance() {
    char input;
    cmdMessage(2, "Press c to continue!", false);
    while(input = getch() != 'c'){};
}
void Menu::clear() {
    cmd->clear();
}

Menu::Menu() {
	
}
Menu::~Menu() {
	delete cmd;
}

