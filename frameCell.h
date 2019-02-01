#ifndef __FRAMECELL_H
#define __FRANECELL_H

#include "window.h"

/* 
 * This is the most fundamental class of the game. A cell is represented by
 * a character (which will be '32' for ship body part and -1 for empty (it's not unsigned so it works)
 * and a color. The color will be specific to each ship 
*/

class FrameCell
{
	char ch;
	Color color;

	public:
		int getCh();
		Color getColor();
		void setColor(Color color);
		void setCh(char ch);
		void set(char ch, Color color);
};

#endif