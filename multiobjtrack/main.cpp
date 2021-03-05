// OpenCV Diplay camera. http://talkera.org/opencv
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "/usr/local/include/opencv/highgui.h"

#include <iostream>
#include <stdio.h>
#include <sstream>
#include <string>
#include <cstdio>
#include "object.h"



int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 131;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;
//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS=50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 40*40;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH/1.5;
//names that will appear at the top of each window
const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morphological Operations";
const string trackbarWindowName = "Trackbars";

string intToString(int number){


	std::stringstream ss;
	ss << number;
	return ss.str();
}
void drawObject(vector<object> theFruits,Mat &frame){

	for(int i =0; i<theFruits.size(); i++){

	cv::circle(frame,cv::Point(theFruits.at(i).getXPos(),theFruits.at(i).getYPos()),10,cv::Scalar(0,0,255));
	cv::putText(frame,intToString(theFruits.at(i).getXPos())+ " , " + intToString(theFruits.at(i).getYPos()),cv::Point(theFruits.at(i).getXPos(),theFruits.at(i).getYPos()+20),1,1,Scalar(0,255,0));
	cv::putText(frame,theFruits.at(i).getType(),cv::Point(theFruits.at(i).getXPos(),theFruits.at(i).getYPos()-30),1,2,theFruits.at(i).getColor());
	}
}
void morphOps(Mat &thresh){

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));

	erode(thresh,thresh,erodeElement);
	erode(thresh,thresh,erodeElement);

	dilate(thresh,thresh,dilateElement);
	dilate(thresh,thresh,dilateElement);



}
void trackFilteredObject(object objr,Mat threshold,Mat HSV, Mat &cameraFeed){

	vector<object> vobj;

	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if(numObjects<MAX_NUM_OBJECTS){
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if(area>MIN_OBJECT_AREA){
					object obj;
					obj.setXPos(moment.m10/area);
					obj.setYPos(moment.m01/area);
					obj.setType(objr.getType());
					obj.setColor(objr.getColor());
					vobj.push_back(obj);
					//x = moment.m10/area;
					///y = moment.m01/area;
					objectFound = true;

				}else objectFound = false;


			}
			//let user know you found an object
			if(objectFound ==true){
				//draw object location on screen
				drawObject(vobj,cameraFeed);}

		}else putText(cameraFeed,"TOO MUCH NOISE! ADJUST FILTER",Point(0,50),1,2,Scalar(0,0,255),2);
	}
}

int main(int argc, char* argv[])
{
	//if we would like to calibrate our filter values, set to true.
	bool calibrationMode = false;

	//Matrix to store each frame of the webcam feed
	Mat cameraFeed;
	Mat threshold;
	Mat HSV;
	//video capture object to acquire webcam feed
	VideoCapture capture;
	//open capture object at location zero (default location for webcam)
	capture.open(0);
	//set height and width of capture frame
	capture.set(CV_CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);
	//start an infinite loop where webcam feed is copied to cameraFeed matrix
	//all of our operations will be performed within this loop
	while(1){
		//store image to matrix
		capture.read(cameraFeed);
		object obj1("obj1"),obj2;
		//convert frame from BGR to HSV colorspace
		cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
		inRange(HSV,obj1.getMinHSV(),obj1.getMaxHSV(),threshold);
		morphOps(threshold);
		trackFilteredObject(obj1,threshold,HSV,cameraFeed);


//		cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
//		inRange(HSV,obj2.getMinHSV(),obj2.getMaxHSV(),threshold);
//		morphOps(threshold);
//		trackFilteredObject(threshold,HSV,cameraFeed);

		//show frames
		//imshow(windowName2,threshold);

		imshow(windowName,cameraFeed);
		//imshow(windowName1,HSV);


		//delay 30ms so that screen can refresh.
		//image will not appear without this waitKey() command
		waitKey(30);
	}

	return 0;
}
