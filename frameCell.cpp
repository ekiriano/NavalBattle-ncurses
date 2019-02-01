#include "frameCell.h"

int FrameCell::getCh()
{
	return ch;
}
Color FrameCell::getColor()
{
	return color;
}
void FrameCell::setColor(Color color)
{
	this->color = color;
}
void FrameCell::setCh(char ch){
	this->ch = ch;
}
void FrameCell::set(char ch, Color color)
{
	setColor(color);
	setCh(ch);
}