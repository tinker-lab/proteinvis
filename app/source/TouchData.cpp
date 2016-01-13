#include "app/include/TouchData.h"

TouchData::TouchData(MinVR::EventRef event, glm::dvec3 currRoomPos) {
	prevEvent = event;
	currEvent = event;
	handBelongTo = -1;
	double freq = 60.0;
	double mincutoff = 50.0;
	double beta = 3.0;
	double dcutoff = 1.2;

	xFilter.reset(new OneEuroFilter(freq, mincutoff, beta, dcutoff));
	zFilter.reset(new OneEuroFilter(freq, mincutoff, beta, dcutoff));

	double timeForFilter = glfwGetTime();

	double xClean = xFilter->filter(currRoomPos.x, timeForFilter);
	double zClean = zFilter->filter(currRoomPos.z, timeForFilter);

	currRoomPos = glm::dvec3(xClean, currRoomPos.y, zClean);

	this->currRoomPos = currRoomPos;
	prevRoomPos = currRoomPos;
	
}

TouchData::~TouchData() {

}

std::string TouchData::toString() {
	return currEvent->toString();
}

void TouchData::setCurrentEvent(MinVR::EventRef event) {
	prevEvent = currEvent;
	currEvent = event;
}

MinVR::EventRef TouchData::getCurrentEvent() {
	return currEvent;
}

MinVR::EventRef TouchData::getPreviousEvent() {
	return prevEvent;
}

void TouchData::setCurrRoomPos(glm::dvec3 pos) {
	prevRoomPos = currRoomPos;

	double timeForFilter = glfwGetTime();

	//std::cout << glm::to_string(pos) << std::endl;
	double xClean = xFilter->filter(pos.x, timeForFilter);
	double zClean = zFilter->filter(pos.z, timeForFilter);

	pos = glm::dvec3(xClean, currRoomPos.y, zClean);
	currRoomPos = pos;
	//std::cout << glm::to_string(pos) << std::endl;
}

glm::dvec3 TouchData::getCurrRoomPos() {
	return currRoomPos;
}


glm::dvec3 TouchData::getPrevRoomPos() {
	return prevRoomPos;
}

//  gives you the distance in screen coordinates
glm::dvec3 TouchData::roomPositionDifference() {
	return currRoomPos - prevRoomPos;
}

void TouchData::setBelongTo(int hand){
	handBelongTo = hand;
}

int TouchData::getBelongTo(){
	return handBelongTo;
}