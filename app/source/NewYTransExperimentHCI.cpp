#include "app/include/NewYTransExperimentHCI.h"

const double THRESH = 0.00134;
#define ANGLE_AXIS_CHANGE_THRESHOLD 0.0034906585

NewYTransExperimentHCI::NewYTransExperimentHCI(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr, TextureMgrRef texMan, FeedbackRef feedback) : AbstractHCI(cFrameMgr, feedback) {
	offAxisCamera = std::dynamic_pointer_cast<MinVR::CameraOffAxis>(camera);
	this->texMan = texMan;
	startTime = getCurrentTime();
	//std::cout<<"NewYTansExperimentHCI has been created"<<std::endl;
	
}

NewYTransExperimentHCI::~NewYTransExperimentHCI(){

}

void NewYTransExperimentHCI::initializeContextSpecificVars(int threadId,MinVR::WindowRef window) {

	initVBO(threadId);
	//std::cout<<"NewYTansExperimentHCI  is been called"<<std::endl;
	prevHandPos1 = glm::dvec3(DBL_MAX, -1.0, DBL_MAX);
	prevHandPos2 = glm::dvec3(DBL_MAX, -1.0, DBL_MAX);
	currHandPos1 = glm::dvec3(DBL_MAX, -1.0, DBL_MAX);
	currHandPos2 = glm::dvec3(DBL_MAX, -1.0, DBL_MAX);
	xzRotFlag = false;
	centerRotMode = false;
	liftedFingers = true;
	//freopen("output2.txt","w",stdout);

	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "openGL ERROR in initializeContextSpecificVars: "<< err << std::endl;
	}
}

void NewYTransExperimentHCI::reset() 
{
	prevHandPos1 = glm::dvec3(DBL_MAX, -1.0, DBL_MAX);
	prevHandPos2 = glm::dvec3(DBL_MAX, -1.0, DBL_MAX);
	currHandPos1 = glm::dvec3(DBL_MAX, -1.0, DBL_MAX);
	currHandPos2 = glm::dvec3(DBL_MAX, -1.0, DBL_MAX);
	xzRotFlag = false;
	centerRotMode = false;
	liftedFingers = true;
	registeredTouchData.clear();
}



//void NewYTransExperimentHCI::testForCrazyInput

// this function produces a map, which we can later query to draw things.
void NewYTransExperimentHCI::update(const std::vector<MinVR::EventRef> &events){

	//later to be pass into closestTouchPair function
	glm::dvec3 pos1;
	glm::dvec3 pos2;
	double minDistance = DBL_MAX; 
	MinVR::TimeStamp timestamp;

	//boolean flags
	bool xzTrans = false;
	glm::dmat4 xzTransMat = dmat4(0.0);
	bool yTrans = false;
	glm::dmat4 yTransMat = dmat4(0.0);
	bool yRotScale = false;

	numTouchForHand1 = 0;
	numTouchForHand2 = 0;

	// only update the map and other variables first
	for(int p = 0; p < events.size(); p++) {

		
		timestamp = events[p]->getTimestamp();
		std::string name = events[p]->getName();
		int id = events[p]->getId();


		if (boost::algorithm::starts_with(name, "TUIO_Cursor_up")) {
			// delete the cursor down associated with this up event
			std::map<int, TouchDataRef>::iterator it = registeredTouchData.find(id); 

			if (it != registeredTouchData.end()) { // if id is found
				registeredTouchData.erase(it);	   //erase value associate with that it
				//std::cout << "UP" <<std::endl;
			}
			else {
				std::cout<<"ERROR: Received touch up for a cursor not in the registered touch data!"<<std::endl;
				std::cout << events[p]->toString() << std::endl;
				for (auto iter = registeredTouchData.begin(); iter != registeredTouchData.end(); iter++) {
					std::cout << "\t" << iter->second->toString() << std::endl;
				}
			}

		} else if (boost::algorithm::starts_with(name, "TUIO_Cursor_down")) {
			// always add a new one on DOWN
			glm::dvec3 roomCoord = convertScreenToRoomCoordinates(events[p]->get2DData());
			TouchDataRef datum(new TouchData(events[p], roomCoord));
			registeredTouchData.insert(std::pair<int, TouchDataRef>(id, datum));
			//std::cout << "DOWN " << glm::to_string(events[p]->get2DData()) <<std::endl;


		} else if (boost::algorithm::starts_with(name, "TUIO_CursorMove")) {
			// update the map with the move event
			// if the corresponding id was down, make it a move event
			std::map<int, TouchDataRef>::iterator it = registeredTouchData.find(id); 
			//std::cout << "Move " << events[p]->getId() <<std::endl;

			if (it != registeredTouchData.end()) { // if id is found
				glm::dvec2 screenCoord (events[p]->get4DData());
				glm::dvec3 roomCoord = convertScreenToRoomCoordinates(glm::dvec2(events[p]->get4DData()));

				// update map
				it->second->setCurrentEvent(events[p]);
				it->second->setCurrRoomPos(roomCoord);
			}
			else {
				std::cout<<"ERROR: Received touch move for a cursor not in the registered touch data!"<<std::endl;
			}
		}
		// end of TUIO events

		// Update hand positions
		if (name == "Hand_Tracker1") {
			//std::cout << "Inside hand tracking 1 event " << std::endl;
			//only enter one time to init prevHandPos1
			if (prevHandPos1.y == -1.0) {
				glm::dvec3 currHandPos1 (events[p]->getCoordinateFrameData()[3]);
				prevHandPos1 = currHandPos1;
				initRoomPos = true;
			} else {
				//std::cout << "updating hand 1 curr and prev position  " << std::endl;
				prevHandPos1 = currHandPos1;
				currHandPos1 = glm::dvec3(events[p]->getCoordinateFrameData()[3]);
			} 
		} 
		

		//std::cout<<"currHandPos1: "<<glm::to_string(currHandPos1)<<std::endl;
		//std::cout<<"prevHandPos1: "<<glm::to_string(prevHandPos1)<<std::endl;

		if(name == "Hand_Tracker2") {
			//std::cout << "Inside hand tracking 2 event " << std::endl;
			if (prevHandPos2.y == -1.0) {
				glm::dvec3 currHandPos2 (events[p]->getCoordinateFrameData()[3]);
				prevHandPos2 = currHandPos2;
			} else {
				//std::cout << "updating hand 2 curr and prev position  " << std::endl;
				prevHandPos2 = currHandPos2;
				currHandPos2 = glm::dvec3(events[p]->getCoordinateFrameData()[3]);
			} 
		}
	} // end of data-updating for loop
	
	// give feedback object the touch data so
	// it can draw touch positions
	feedback->registeredTouchData = registeredTouchData;

	//// At this point, the touch data should be updated, and hand positions
	std::map<int, TouchDataRef>::iterator iter;
	for (iter = registeredTouchData.begin(); iter != registeredTouchData.end(); iter++) {

		glm::dvec3 currRoomPos (iter->second->getCurrRoomPos());
		bool belongsToHand1 = (glm::length(currHandPos1 - currRoomPos) <  glm::length(currHandPos2 - currRoomPos));
		
		if (belongsToHand1) {
			numTouchForHand1++;
			if(iter->second->getBelongTo() == -1){
				iter->second->setBelongTo(1);
			}
		} 
		else { // belongs to hand 2
			numTouchForHand2++;
			if(iter->second->getBelongTo() == -1){
				iter->second->setBelongTo(2);
			}
		}
	} // end touch enumeration

	// Y Translation Gesture
	double prevHandsDist = glm::length(prevHandPos1 - prevHandPos2);
	double currHandsDist = glm::length(currHandPos1 - currHandPos2);
	//std::cout << "curr - prev hand dist: " << glm::abs(currHandsDist - prevHandsDist) << std::endl;
	
	// weighted balloon gesture for Y translation
	if (registeredTouchData.size() > 3 && glm::abs(currHandsDist - prevHandsDist) < 0.045 && glm::abs(currHandsDist - prevHandsDist) > 0.0005) {

		// check if we have two points for each hand
		if (numTouchForHand1 == 2 && numTouchForHand2 == 2) {
			feedback->displayText = "translating"; 
			//std::cout << "In Y Trans Mode" << std::endl;
			//calculate translate distance
			glm::dvec3 rightTouch1 = glm::dvec3(0.0,-1.0,0.0);
			glm::dvec3 rightTouch2;
			glm::dvec3 leftTouch1 = glm::dvec3(0.0,-1.0,0.0);
			glm::dvec3 leftTouch2;
			
			
			std::map<int, TouchDataRef>::iterator iter;
			for (iter = registeredTouchData.begin(); iter != registeredTouchData.end(); iter++){
				if(iter->second->getBelongTo() == 1){
					if(rightTouch1 == glm::dvec3(0.0,-1.0,0.0)){
						rightTouch2 = iter->second->getCurrRoomPos();
					}
					rightTouch1 = iter->second->getCurrRoomPos();
				}
				else{
					if(leftTouch1 ==  glm::dvec3(0.0,-1.0,0.0)){
						leftTouch2 = iter->second->getCurrRoomPos();
					}
					leftTouch1 = iter->second->getCurrRoomPos();
				}
			}

			/*std::cout<<"RightTouch1: "<<glm::to_string(RightTouch1)<<std::endl;
			std::cout<<"RightTouch2: "<<glm::to_string(RightTouch2)<<std::endl;
			std::cout<<"LeftTouch1: "<<glm::to_string(LeftTouch1)<<std::endl;
			std::cout<<"LeftTouch2: "<<glm::to_string(LeftTouch2)<<std::endl;
*/
			double prevHandsDist = glm::length(prevHandPos1 - prevHandPos2);
			double currHandsDist = glm::length(currHandPos1 - currHandPos2);
			glm::dvec3 rightTouchCentre = 0.5 * (rightTouch1 + rightTouch2);
			glm::dvec3 leftTouchCentre = 0.5 * (leftTouch1 + leftTouch2 );
			//both hand go outward neg on right pos on left
			double angle = 0.0;
			glm::dvec3 rightBefore = prevHandPos1 - rightTouchCentre;
			glm::dvec3 rightAfter = currHandPos1 - rightTouchCentre;
			glm::dvec3 rightZVector = glm::cross(glm::normalize(rightBefore),glm::normalize(rightAfter));
			glm::dvec3 leftBefore = prevHandPos2 - leftTouchCentre;
			glm::dvec3 leftAfter= currHandPos2 - leftTouchCentre;
			glm::dvec3 leftZVector= glm::cross(glm::normalize(leftBefore),glm::normalize(leftAfter));
			
			//std::cout<<"outwardrightZVector: "<<glm::to_string(rightZVector)<<std::endl;
			//std::cout<<"outwardleftZVector: "<<glm::to_string(leftZVector)<<std::endl;

			double rightAngle = glm::acos(glm::clamp(glm::dot(glm::normalize(rightBefore), glm::normalize(rightAfter)), -1.0, 1.0));
			double leftAngle = glm::acos(glm::clamp(glm::dot(glm::normalize(leftBefore), glm::normalize(leftAfter)), -1.0, 1.0));
			angle = (rightAngle+leftAngle)*0.5;

			if (leftZVector.z < 0) {
				angle = -angle;
			}
			//std::cout<<"angle: "<<angle<<std::endl;
			if (glm::abs(angle) > ANGLE_AXIS_CHANGE_THRESHOLD) { 
				double scale = 1.0;
				double transBy = scale * angle / (M_PI/2.0);

				//double transBy = currHandsDist - prevHandsDist;
				glm::dvec3 yTransBy (0.0, transBy, 0.0);
				glm::dmat4 yTransMat (glm::translate(glm::dmat4(1.0), -yTransBy));

				cFrameMgr->setRoomToVirtualSpaceFrame(cFrameMgr->getRoomToVirtualSpaceFrame() * yTransMat);

				//std::cout<<"Translating by: "<<transBy<<std::endl;
			}
			else {
				//std::cout<<"Not translating because angle is too little"<<std::endl;
			}
		}
		else {
			//std::cout<<"Not translating because there are more than 4 touches, but not 2 per hand. Left: "<<numTouchForHand1<<" Right: "<<numTouchForHand2<<std::endl;
		}
	}


	

	///// Apply the correct matrix transforms based on updated state (booleans, registeredTouchData, instance variables)
	if (yTrans) {
		translate(yTransMat);
	}

	if(registeredTouchData.size() == 0) {
		feedback->displayText = "";
		liftedFingers = true;
	}

	// this is bret's commented out line
	//updateHandPos(events);
}

void NewYTransExperimentHCI::closestTouchPair(std::map<int, TouchDataRef> thisRegisteredTouchData, glm::dvec3 &pos1, glm::dvec3 &pos2, double &minDistance) {
	std::map<int, TouchDataRef>::iterator it1;
	std::map<int, TouchDataRef>::iterator it2;

	for(it1 = registeredTouchData.begin(); it1 != std::prev(registeredTouchData.end(), 1); it1++) {
		pos1 = it1->second->getCurrRoomPos();
		//std::cout << "pos1 : " << glm::to_string(pos1) << std::endl;

		for(it2 = std::next(it1, 1) ; it2 != registeredTouchData.end(); it2++){
			pos2 = it2->second->getCurrRoomPos();
			//std::cout << "pos2: " << glm::to_string(pos2) << std::endl;

			if(minDistance > glm::abs(glm::length(pos1-pos2))){
				minDistance = glm::abs(glm::length(pos1-pos2));
			}
		}
	}
	//std::cout << "pos1 : " << glm::to_string(pos1) << std::endl;
	//std::cout << "pos2: " << glm::to_string(pos2) << std::endl;
	//std::cout << "Calculating minDist: " << minDistance << std::endl;
}



glm::dvec3 NewYTransExperimentHCI::convertScreenToRoomCoordinates(glm::dvec2 screenCoords) {
	glm::dvec3 xVec = offAxisCamera->getTopRight() - offAxisCamera->getTopLeft();
	glm::dvec3 yVec = offAxisCamera->getBottomRight() - offAxisCamera->getTopRight();
	return offAxisCamera->getTopLeft() + (screenCoords.x * xVec) + (screenCoords.y * yVec);
}


int NewYTransExperimentHCI::getNumberTouches(){

	return registeredTouchData.size();
}

void NewYTransExperimentHCI::translate(glm::dmat4 transMat){
	glm::dmat4 newTransform = cFrameMgr->getRoomToVirtualSpaceFrame()*transMat;
	cFrameMgr->setRoomToVirtualSpaceFrame(newTransform);
}


