/*
 * object.h
 *
 *  Created on: Dec 14, 2015
 *      Author: usman
 */
#include <string>
#include "/usr/local/include/opencv/cv.h"

using namespace std;
using namespace cv;

#ifndef OBJECT_H_
#define OBJECT_H_

class object {
public:
	object();
	virtual ~object();
	object(string name);

	int getXPos();
	void setXPos(int x);

	int getYPos();
	void setYPos(int y);

	Scalar getMinHSV();
	void setMinHSV(Scalar sc);

	Scalar getMaxHSV();
	void setMaxHSV(Scalar sc);

	string getType();
	void setType(string type);

	Scalar getColor();
	void setColor(Scalar color);

private:
	int xPos, yPos;
	string type;
	Scalar HSVmin, HSVmax;
	Scalar Color;
};

#endif /* OBJECT_H_ */
