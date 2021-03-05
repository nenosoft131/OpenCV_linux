#include "shapefinder.h"

shapeFinder::shapeFinder()
{
	this->callCounter = 0;
	this->seenBeforeCheck = false;
}

shapeFinder::shapeFinder(Mat image)
{
	this->image = image;
	this->callCounter = 0;
	this->seenBeforeCheck = false;
}

void shapeFinder::setImage(Mat image)
{
	this->image = image;
}

std::vector<foundShape> shapeFinder::findRegPolygons(int numSides, int minApothem, int maxApothem, int maxShapes, int voteThreshold, int apothemStep, int baseRotation, int angleTolerance)
{
	std::vector<foundShape> bestShapes;
	std::vector<Mat> voteImages;
	std::vector<int> apothems;

	// Vote for the desired radii of the shape:
	for(int apothem = minApothem; apothem <= maxApothem; apothem += apothemStep) {
		voteImages.push_back(voteRegPoly(numSides, apothem, angleTolerance, baseRotation));
		apothems.push_back(apothem);
	}

	// Overlay showing votes on the input image:
	/*Mat temp(image.size(), CV_8UC1);
	votes.convertTo(temp, CV_8U, 5);
	cv::imwrite("votes.png", temp);
	cvUtils::saveOverlay("overlay.png", image, temp, 0.7);*/

	findShapes(&bestShapes, voteImages, apothems, maxShapes, voteThreshold);
	callCounter++;

	return bestShapes;
}

std::vector<foundShape> shapeFinder::findRectangles(cv::Size aspectRatio, int minWidth, int maxWidth, int maxShapes, int voteThreshold, int widthStep, int angleTolerance)
{
	std::vector<foundShape> bestShapes;
	std::vector<Mat> voteImages;
	std::vector<int> sizes;

	// Vote for the desired radii of the shape:
	for(int width = minWidth; width <= maxWidth; width += widthStep) {
		voteImages.push_back(voteRectangle(aspectRatio, width, angleTolerance));
		sizes.push_back(width);
	}

	// Overlay showing votes on the input image:
	/*Mat temp(image.size(), CV_8UC1);
	Mat votes(image.size(), CV_32S, cv::Scalar(0));
	for(unsigned int i = 0; i < voteImages.size(); i++) {
		votes += voteImages[i];
	}
	votes.convertTo(temp, CV_8U, 5);
	cv::imwrite("votes.png", temp);
	cvUtils::saveOverlay("overlay.png", image, temp, 0.7);*/

	findShapes(&bestShapes, voteImages, sizes, maxShapes, voteThreshold);
	callCounter++;

	return bestShapes;
}

void shapeFinder::findGradients(Mat magnitudes, Mat xSobel, Mat ySobel, float* thresholdValue)
{
	Mat xSquared(image.rows, image.cols, CV_32F);
	Mat ySquared(image.rows, image.cols, CV_32F);
	Mat sorted(image.rows, image.cols, CV_32F);

	// Compute gradient magnitudes
	cv::Sobel(image, xSobel, CV_32F, 1, 0);
	cv::Sobel(image, ySobel, CV_32F, 0, 1);
	cv::multiply(xSobel, xSobel, xSquared);
	cv::multiply(ySobel, ySobel, ySquared);
	magnitudes = xSquared+ySquared;
	cv::sqrt(magnitudes, magnitudes);

	// Find the threshold for the top 20%. Only done for each 10 frames, since it is slow:
	if(callCounter++ % 10 == 0) {
		sorted = magnitudes.clone();
		sorted = sorted.reshape(0, 1);
		cv::sort(sorted, sorted, CV_SORT_EVERY_ROW | CV_SORT_ASCENDING);
		currentThreshold = sorted.at<float>(0, floor(0.9f*sorted.cols));
	}

	*thresholdValue = currentThreshold;

	/*printf("Threshold value: %f\n", *thresholdValue);
	*thresholdValue = 70;*/
}

Mat shapeFinder::voteRegPoly(int numSides, int apothem, int rotation, int angleTolerance)
{
	Mat xSobel(image.size(), CV_32F);
	Mat ySobel(image.size(), CV_32F);
	Mat magnitudes(image.size(), CV_32F);
	Mat votes(image.size(), CV_32S, cv::Scalar(0));
	float thresholdValue;

	findGradients(magnitudes, xSobel, ySobel, &thresholdValue);

	int w = cvUtils::round((float)apothem*tan(M_PI/numSides));

	// Find the acceptable grandient ranges:
	std::vector<cv::Range> ranges;
	angleRanges(&ranges, numSides, angleTolerance, rotation);

	cv::Vec2f unitGradient;
	cv::Point currentPoint;
	float angle;

	// Loop through gradients:
	for(currentPoint.y = 0; currentPoint.y < magnitudes.rows; currentPoint.y++) {
		for(currentPoint.x = 0; currentPoint.x < magnitudes.cols; currentPoint.x++) {
			// Ignore gradients below the threshold:
			if(magnitudes.at<float>(currentPoint.y, currentPoint.x) < thresholdValue) {
				continue;
			}

			// Ignore gradients not in accepted angular ranges:
			angle = cv::fastAtan2(ySobel.at<float>(currentPoint.y, currentPoint.x), xSobel.at<float>(currentPoint.y, currentPoint.x));
			if(!cvUtils::inRanges(angle, ranges)) {
				continue;
			}

			float length = std::sqrt(std::pow(xSobel.at<float>(currentPoint.y, currentPoint.x), 2) + std::pow(ySobel.at<float>(currentPoint.y, currentPoint.x), 2));
			unitGradient[0] = xSobel.at<float>(currentPoint.y, currentPoint.x)/length;
			unitGradient[1] = ySobel.at<float>(currentPoint.y, currentPoint.x)/length;

			vote(votes, unitGradient, currentPoint, w, apothem);
		}
	}

	return votes;
}

Mat shapeFinder::voteRectangle(cv::Size aspectRatio, int width, int angleTolerance)
{
	Mat xSobel(image.size(), CV_32F);
	Mat ySobel(image.size(), CV_32F);
	Mat magnitudes(image.size(), CV_32F);
	Mat votes(image.size(), CV_32S, cv::Scalar(0));
	float thresholdValue;

	Mat temp1(image.size(), CV_32F);
	Mat temp2(image.size(), CV_32F);

	findGradients(magnitudes, xSobel, ySobel, &thresholdValue);

	//imwrite("magnitudes.png", magnitudes);

	int height = cvUtils::round(((float)aspectRatio.height/(float)aspectRatio.width)*width);

	// Find the acceptable grandient ranges:
	std::vector<cv::Range> ranges;
	ranges.push_back(cv::Range(-angleTolerance, angleTolerance)); // Vertical lines (horizontal gradient)
	ranges.push_back(cv::Range(180-angleTolerance, 180+angleTolerance)); // Vertical lines
	ranges.push_back(cv::Range(90-angleTolerance, 90+angleTolerance)); // Horizontal lines (vertical gradient)
	ranges.push_back(cv::Range(270-angleTolerance, 270+angleTolerance)); // Horizontal lines

	cv::Vec2f unitGradient;
	cv::Point currentPoint;
	float angle;

	// Loop through gradients:
	for(currentPoint.y = 0; currentPoint.y < magnitudes.rows; currentPoint.y++) {
		for(currentPoint.x = 0; currentPoint.x < magnitudes.cols; currentPoint.x++) {
			// Ignore gradients below the threshold:
			if(magnitudes.at<float>(currentPoint.y, currentPoint.x) < thresholdValue) {
				continue;
			}

			//temp1.at<float>(currentPoint.y, currentPoint.x) = magnitudes.at<float>(currentPoint.y, currentPoint.x);

			// Ignore gradients not in accepted angular ranges:
			angle = cv::fastAtan2(ySobel.at<float>(currentPoint.y, currentPoint.x), xSobel.at<float>(currentPoint.y, currentPoint.x));
			if(!cvUtils::inRanges(angle, ranges)) {
				continue;
			}

			//temp2.at<float>(currentPoint.y, currentPoint.x) = magnitudes.at<float>(currentPoint.y, currentPoint.x);

			float length = std::sqrt(std::pow(xSobel.at<float>(currentPoint.y, currentPoint.x), 2) + std::pow(ySobel.at<float>(currentPoint.y, currentPoint.x), 2));
			unitGradient[0] = xSobel.at<float>(currentPoint.y, currentPoint.x)/length;
			unitGradient[1] = ySobel.at<float>(currentPoint.y, currentPoint.x)/length;

			if(angle < 45) {
				vote(votes, unitGradient, currentPoint, width/2, height/2); // Vote horizontal
			} else {
				vote(votes, unitGradient, currentPoint, height/2, width/2); // Vote vertical
			}
		}
	}

	//imwrite("magFiltered1.png", temp1);
	//imwrite("magFiltered2.png", temp2);

	return votes;
}

void shapeFinder::findShapes(std::vector<foundShape>* bestShapes, std::vector<Mat> voteImages, std::vector<int> sizes, int maxShapes, int voteThreshold)
{
	cv::Point point;
	Mat totalVotes(voteImages[0].size(), CV_32S, cv::Scalar(0));
	Mat mask(voteImages[0].size(), CV_8U, cv::Scalar(255));
	std::vector<foundShape> checkedShapes;
	int maxVotes = 0, bestSize = 0;
	double accVotes;

	for(unsigned int i = 0; i < voteImages.size(); i++) {
		totalVotes += voteImages[i];
	}

	for(int i = 0; i < maxShapes; i++) {
		cv::minMaxLoc(totalVotes, NULL, &accVotes, NULL, &point, mask);

		maxVotes = 0;
		for(unsigned int j = 0; j < voteImages.size(); j++) { // Loop through votes for each image and find the best.
			if(voteImages[j].at<int>(point) > maxVotes) {
				maxVotes = voteImages[j].at<int>(point);
				bestSize = sizes[j];
			}
		}

		if(maxVotes < voteThreshold) { // There are no points left above the threshold.
			break;
		}

		circle(mask, point, bestSize, cv::Scalar(0), -1); // Mask out this position so it won't be found later.

		/*if(maxVotes < 0.3*accVotes) { // If a single vote image does not hold enough votes to make this a plausible shape, we continue to the next.
			continue;
		}*/

		if(seenBeforeCheck) { // Prepare for the seen-before-check in the next round by adding all potential shapes to the list.
			checkedShapes.push_back(foundShape(point, bestSize, maxVotes));
		}

		if(seenBeforeCheck && !seenBefore(foundShape(point, bestSize, maxVotes))) { // If this shape was not in the previous frame, we don't believe it's a legitimate shape (provided we actually care about that).
			continue;
		}

		// All tests are passed, so this is one of the best shapes. Let's go add it.
		bestShapes->push_back(foundShape(point, bestSize, maxVotes));

		/*char string[255];
		sprintf(string, "mask%d.png", i);
		cv::imwrite(string, mask);*/
	}

	previousFindings = checkedShapes;
}

bool shapeFinder::seenBefore(foundShape shape, int allowedShift)
{
	for(unsigned int i = 0; i < previousFindings.size(); i++) {
		if(shape.getPosition().x > previousFindings[i].getPosition().x-allowedShift
				&& shape.getPosition().x < previousFindings[i].getPosition().x+allowedShift
				&& shape.getPosition().y > previousFindings[i].getPosition().y-allowedShift
				&& shape.getPosition().y < previousFindings[i].getPosition().y+allowedShift) {
			return true;
		}
	}

	return false;
}

void shapeFinder::angleRanges(std::vector<cv::Range>* ranges, int numSides, int rotation, int angleTolerance)
{
	cv::Range range;
	int baseAngle;
	for(int i = 0; i < numSides; i++) {
		baseAngle = cvUtils::round((float)i*(360.0f/(float)numSides)+90+rotation); // The base angle is rotated by 90 degrees, since the gradients are orthogonal to the sides of the shape.
		range.start = (baseAngle-angleTolerance)%360;
		range.end = (baseAngle+angleTolerance)%360;
		ranges->push_back(range);
		/*range.start = (baseAngle+180-angleTolerance)%360; // Since the shape edges can be both dark on light and light on dark, gradients are also allowed in the opposite direction.
		range.end = (baseAngle+180+angleTolerance)%360;
		ranges.push_back(range);*/
	}
}

void shapeFinder::vote(Mat votes, cv::Vec2f unitGradient, cv::Point basePoint, int w, int distance)
{
	cv::Point votePoint;
	int vote;

	// Voting mechanism:
	for(int m = -2*w; m <= 2*w; m++) {
		vote = (m >= -w && m <= w) ? 1 : -1; // Vote positive when within -w and w. Otherwise, vote negative.

		// First we vote in the gradient direction (as per the + sign in the round()-call):
		votePoint.x = basePoint.x + cvUtils::round(m*-unitGradient[1] + distance*unitGradient[0]);
		votePoint.y = basePoint.y + cvUtils::round(m*unitGradient[0] + distance*unitGradient[1]);
		if(cvUtils::inImage(votePoint, votes.size())) { // Make sure the vote is inside the image.
			votes.at<int>(votePoint.y,votePoint.x) += vote;
		}

		// Then we vote in the opposite direction:
		votePoint.x = basePoint.x + cvUtils::round(m*-unitGradient[1] - distance*unitGradient[0]);
		votePoint.y = basePoint.y + cvUtils::round(m*unitGradient[0] - distance*unitGradient[1]);
		if(cvUtils::inImage(votePoint, votes.size())) { // Make sure the vote is inside the image.
			votes.at<int>(votePoint.y,votePoint.x) += vote;
		}
	}
}

bool shapeFinder::getSeenBefore()
{
	return seenBeforeCheck;
}

void shapeFinder::setSeenBefore(bool seenBefore)
{
	this->seenBeforeCheck = seenBefore;
}
