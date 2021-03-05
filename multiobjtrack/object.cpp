/*
 * object.cpp
 *
 *  Created on: Dec 14, 2015
 *      Author: usman
 */

#include "object.h"

object::object() {
	// TODO Auto-generated constructor stub

}
object::object(string name) {
	// TODO Auto-generated constructor stub
	setType(name);
		if(name=="obj1")
		{
			setMaxHSV(Scalar(256,256,256));
			setMinHSV(Scalar(0,131,0));
			setColor(Scalar(153,0,0));

		}

}
object::~object() {
	// TODO Auto-generated destructor stub
}
int object::getXPos() {

	return object::xPos;
}
int object::getYPos() {

	return object::yPos;
}
void object::setXPos(int x) {
	object::xPos = x;
}
void object::setYPos(int y) {
	object::yPos = y;
}

Scalar object::getMinHSV() {

	return object::HSVmin;
}
void object::setMinHSV(Scalar sc) {
	object::HSVmin = sc;
}
Scalar object::getMaxHSV() {

	return object::HSVmax;
}
void object::setMaxHSV(Scalar sc) {
	object::HSVmax = sc;
}
void object::setType(string type) {
	object::type = type;
}
string object::getType() {
	return object::type;
}
void object::setColor(Scalar color) {
	object::Color = color;
}
Scalar object::getColor() {
	return object::Color;
}
