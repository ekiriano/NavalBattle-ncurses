#ifndef __MENU_H
#define __MENU_H

#include "window.h"

/* The menu class provides predefined messages for the most important stages of the game.
 * All the messages will be written in the window pointed for *cmd (in our game the 
 * menu down the screen). It provides the method cmdMessage for custom messages.
 * It also have messages for common situations, like "press to continue".
 */

class Menu {
	Window *cmd;
	public:
		void cmdMessage(int row, std::string msg, bool clear);
		void CMDMainMenu();
		void CMDShipSelectMenu(std::string playerName);
		void CMDAskForName(std::string playerID);
		void CMDFoundShip();
		void CMDInfo(); 
		void clear();
		void init(int pos);
		void advance(); //can't use contiue keyword :D
		Menu();
		~Menu();
};
#endif