#include "app/include/NewAnchoredExperimentHCI.h"

const double THRESH = 0.00134;


NewAnchoredExperimentHCI::NewAnchoredExperimentHCI(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr, TextureMgrRef texMan, FeedbackRef feedback) : AbstractHCI(cFrameMgr, feedback) {
	offAxisCamera = std::dynamic_pointer_cast<MinVR::CameraOffAxis>(camera);
	this->texMan = texMan;
	startTime = getCurrentTime();
	
}

NewAnchoredExperimentHCI::~NewAnchoredExperimentHCI(){

}

void NewAnchoredExperimentHCI::initializeContextSpecificVars(int threadId,MinVR::WindowRef window) {
	
	std::cout<<"TuioHCIinit is been called"<<std::endl;
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


void NewAnchoredExperimentHCI::reset()
{
	registeredTouchData.clear();
	prevHandPos1 = glm::dvec3(DBL_MAX, -1.0, DBL_MAX);
	prevHandPos2 = glm::dvec3(DBL_MAX, -1.0, DBL_MAX);
	currHandPos1 = glm::dvec3(DBL_MAX, -1.0, DBL_MAX);
	currHandPos2 = glm::dvec3(DBL_MAX, -1.0, DBL_MAX);
	xzRotFlag = false;
	centerRotMode = false;
	liftedFingers = true;
}


//void NewAnchoredExperimentHCI::testForCrazyInput

// this function produces a map, which we can later query to draw things.
void NewAnchoredExperimentHCI::update(const std::vector<MinVR::EventRef> &events){

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
			//std::cout << "Move " << events[i]->getId() <<std::endl;

			if (it != registeredTouchData.end()) { // if id is found
				glm::dvec2 screenCoord (events[p]->get4DData());
				glm::dvec3 roomCoord = convertScreenToRoomCoordinates(glm::dvec2(events[p]->get4DData()));

				// update map
				it->second->setCurrentEvent(events[p]);
				it->second->setCurrRoomPos(roomCoord);
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
	

	// from TUIO move
	// translate
	//std::cout<<"xzRotFlag"<< xzRotFlag <<std::endl;
	if (registeredTouchData.size() == 1 && !xzRotFlag) {  //only one finger on screen
		xzTrans = true;
		// negate the translation so this is actually virtual to room space
		xzTransMat = glm::translate(glm::dmat4(1.0f), -1.0*registeredTouchData.begin()->second->roomPositionDifference());
		feedback->displayText = "translating"; 
	}

	// from TUIO move
	// yRotScale

	if (registeredTouchData.size() == 2 && !xzRotFlag && numTouchForHand1 == 1 && numTouchForHand2 == 1) {
		//std::cout << "Inside y rot scale " << std::endl;
		// have to do calculations, switching between both touch points 

		std::map<int, TouchDataRef>::iterator iter = registeredTouchData.begin(); // current touch point
		TouchDataRef otherTouch = iter->second;
		TouchDataRef centOfRot = std::next(iter, 1)->second; // find other touch point

		 /*swap between which one is center of rotation between the two during rotation-scale*/
		yRotationAndScale(centOfRot, otherTouch);
		yRotationAndScale(otherTouch, centOfRot);

		feedback->displayText = "rotating-scaling"; 
	} // END OF yRotScale

	//std::cout << "Touch size: " << registeredTouchData.size() << std::endl;
	// find closest pair of TouchPoints
	if (registeredTouchData.size() > 1) {
		closestTouchPair(registeredTouchData , pos1, pos2, minDistance);

		//std::cout << "Min distance: " << minDistance << std::endl;
		
	}

	
	

	if (minDistance < 0.06 && currHandPos1 != prevHandPos1) {
		
		xzRotFlag = true;
		//std::cout << "Inside XZRot Mode" << std::endl;

	}

	if (xzRotFlag && liftedFingers) { // might have to be xzRotFlag and not any other flag 

		feedback->displayText = "rotating";

		if(initRoomPos){
			// pos1 and pos2 are where you put your fingers down. We're finding the centroid
			// to calculate the box boundary for xzRotMode
			initRoomTouchCentre = 0.5*(pos1 + pos2); 
			initRoomPos = false;

		}

		// try to change around center of origin
		glm::dvec3 centOfRot (glm::dvec3((glm::column(cFrameMgr->getVirtualToRoomSpaceFrame(), 3))));
		//calculate the current handToTouch vector
		
		if(registeredTouchData.size() > 1 && !centerRotMode) {
			roomTouchCentre = 0.5*(pos1 + pos2);
		}
		
		//std::cout << "Touch Center: " << glm::to_string(roomTouchCentre) << std::endl; 

		//Should not be equal in XZRotMode but just in case.
		// for choosing the hand that rotates
		/*std::cout<<"currHandPos1: "<<glm::to_string(currHandPos1)<<std::endl;*/


		if (numTouchForHand1 >= numTouchForHand2) { 
			//std::cout << "Using right hand: " << std::endl; 
			currHandToTouch = roomTouchCentre - currHandPos1;
			prevHandToTouch = roomTouchCentre - prevHandPos1;
		} else {
			//std::cout << "Using Left hand: " << std::endl; 
			currHandToTouch = roomTouchCentre - currHandPos2;
			prevHandToTouch = roomTouchCentre - prevHandPos2;
		}
		

		//set up the 2 vertices for a square boundry for the gesture
		glm::dvec3 upRight = glm::dvec3(initRoomTouchCentre.x+0.2, 0.0, initRoomTouchCentre.z+0.2);
		glm::dvec3 lowLeft = glm::dvec3(initRoomTouchCentre.x-0.2, 0.0, initRoomTouchCentre.z-0.2);


		// this if-else block for setting xzRotFlag,
		// also for grabbing xzCentOfRot
		if(registeredTouchData.size() == 0) { //if no touch on screen then automatically exit the xzrot mode
			xzRotFlag = false;
			initRoomPos = true;
			liftedFingers = true;
			feedback->displayText = "";
			feedback->centOfRot.x = DBL_MAX;
			//std::cout<<"no touchyyy so I quit"<<std::endl;
		}
		else { //if there are touch(s) then check if the touch is in bound of the rectangle

			bool setxzRotFlag = true;
			std::map<int, TouchDataRef>::iterator iter;
			int countFingers = 0;
			for (iter = registeredTouchData.begin(); iter != registeredTouchData.end(); iter++) {
				

				// not exactly sure why roomPos. > upRight.z , I think it should be <. but that doesn't work
				if(!(iter->second->getCurrRoomPos().x > upRight.x || iter->second->getCurrRoomPos().z > upRight.z ||iter->second->getCurrRoomPos().x < lowLeft.x ||iter->second->getCurrRoomPos().z < lowLeft.z)){ //you are in the box

					//std::cout << "fingers in bound so STILL IN XZRot Mode" << std::endl;
					setxzRotFlag = false; 
					countFingers += 1;

				} //else{ // touch point not in box, assume as center of rotation
				//	centOfRot = iter->second->getCurrRoomPos();
				//	centerRotMode = true;
				//	//std::cout << "Cent of Rot set" << std::endl;
				//	feedback->centOfRot = centOfRot;
				//}

				// only tries to change the xzRotFlag at the end of the data in the map
				if(iter == std::prev(registeredTouchData.end(),1) && setxzRotFlag) {
					xzRotFlag = false;
					initRoomPos = true;
					feedback->displayText = ""; 
					feedback->centOfRot.x = DBL_MAX;
					//std::cout << "all fingers went out of bound so Out of XZRot Mode" << std::endl;

					// found bug where person just drags their fingers across the table, and it reinitiates xzRotMode
					liftedFingers = false;
				}
			} // end for loop over registeredTouchData

			if(countFingers == registeredTouchData.size()){//all fingers in bound 
				centerRotMode = false;
				feedback->centOfRot.x = DBL_MAX;
			}

		} // end if/else block

		
		//std::cout<<"currHandToTouch: "<<glm::to_string(currHandToTouch)<<std::endl;
		//std::cout<<"prevHandToTouch: "<<glm::to_string(prevHandToTouch)<<std::endl;
		//std::cout<<"dot product of them: "<< glm::to_string(glm::dot(glm::normalize(currHandToTouch), glm::normalize(prevHandToTouch))) << std::endl;
		
			
			//std::cout<<"what we clamping: "<<glm::clamp(glm::dot(currHandToTouch, prevHandToTouch),-1.0,1.0)<<std::endl;

			double alpha = glm::acos(glm::clamp(glm::dot(glm::normalize(currHandToTouch), glm::normalize(prevHandToTouch)),-1.0,1.0)); // from 0 to pi
			//std::cout<<"alpha: " << alpha << std::endl;

			// get cross prod
			
			glm::dvec3 cross = glm::normalize(glm::cross(currHandToTouch, prevHandToTouch)); 
			//std::cout<<"cross: "<<glm::to_string(cross)<<std::endl;
			glm::dvec3 projCross = glm::normalize(glm::dvec3(cross.x, 0.0, cross.z)); // projection
			//std::cout<<"projcross: "<<glm::to_string(projCross)<<std::endl;

			// project cross prod onto the screen, get a length
			
			double lengthOfProjection = glm::dot(cross, projCross); 
			//std::cout<<"lengthOfProjection: "<<glm::to_string(lengthOfProjection)<<std::endl;
			// projected cross prod 
			//glm::dvec3 projectedCrossProd = lengthOfProjection * normProjCross; 

			//std::cout<<"lengthOfProjection: "<<lengthOfProjection<<std::endl;
			//std::cout<<"alpha in degree before times lengthofprojection: "<<glm::degrees(alpha)<<std::endl;

			alpha = alpha * lengthOfProjection;

			//std::cout<<"alpha in degree after: "<<glm::degrees(alpha)<<std::endl;
			//std::cout<<"normProjCross: "<<glm::to_string(normProjCross)<<std::endl;

			glm::dmat4 XZRotMat = glm::rotate(glm::dmat4(1.0), glm::degrees(alpha) /* * 2.0 */, glm::normalize(projCross));

			// have translations when we have a touch point not in the bounding box

			// translate to origin
			glm::dmat4 transMat(glm::translate(glm::dmat4(1.0), -1.0*centOfRot));
			// translate back
			glm::dmat4 transBack(glm::translate(glm::dmat4(1.0), centOfRot));	

			// put it into the matrix stack	
			//std::cout<<"XZRotMat: "<<glm::to_string(XZRotMat) <<std::endl;
			
			cFrameMgr->setRoomToVirtualSpaceFrame(cFrameMgr->getRoomToVirtualSpaceFrame() * transBack * XZRotMat * transMat);
		

	} // end xzRot Gesture

	// Y Translation Gesture
	double prevHandsDist = glm::length(prevHandPos1 - prevHandPos2);
	double currHandsDist = glm::length(currHandPos1 - currHandPos2);
	//std::cout << "curr - prev hand dist: " << glm::abs(currHandsDist - prevHandsDist) << std::endl;
	
	// weighted balloon gesture for Y translation
	if (registeredTouchData.size() > 3 && glm::abs(currHandsDist - prevHandsDist) < 0.045 && glm::abs(currHandsDist - prevHandsDist) > 0.0005) {

		// check if we have two points for each hand
		if (numTouchForHand1 == 2 && numTouchForHand2 == 2) {
			//feedback->displayText = "translating"; 
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
			if (glm::abs(angle) > (M_PI/360.0)) { 
				double scale = 1.0;
				double transBy = scale * angle / (M_PI/2.0);

				//double transBy = currHandsDist - prevHandsDist;
				glm::dvec3 yTransBy (0.0, transBy, 0.0);
				glm::dmat4 yTransMat (glm::translate(glm::dmat4(1.0), -yTransBy));

				cFrameMgr->setRoomToVirtualSpaceFrame(cFrameMgr->getRoomToVirtualSpaceFrame() * yTransMat);
			}
		}
	}

	

	///// Apply the correct matrix transforms based on updated state (booleans, registeredTouchData, instance variables)
	if (xzTrans) {
		translate(xzTransMat);
	} else if (yTrans) {
		translate(yTransMat);
	}

	if(registeredTouchData.size() == 0) {
		feedback->displayText = "";
		liftedFingers = true;
	}

	// this is bret's commented out line
	//updateHandPos(events);
}

void NewAnchoredExperimentHCI::closestTouchPair(std::map<int, TouchDataRef> thisRegisteredTouchData, glm::dvec3 &pos1, glm::dvec3 &pos2, double &minDistance) {
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



glm::dvec3 NewAnchoredExperimentHCI::convertScreenToRoomCoordinates(glm::dvec2 screenCoords) {
	glm::dvec3 xVec = offAxisCamera->getTopRight() - offAxisCamera->getTopLeft();
	glm::dvec3 yVec = offAxisCamera->getBottomRight() - offAxisCamera->getTopRight();
	return offAxisCamera->getTopLeft() + (screenCoords.x * xVec) + (screenCoords.y * yVec);
}


void NewAnchoredExperimentHCI::translate(glm::dmat4 transMat){
	glm::dmat4 newTransform = cFrameMgr->getRoomToVirtualSpaceFrame()*transMat;
	cFrameMgr->setRoomToVirtualSpaceFrame(newTransform);
}

void NewAnchoredExperimentHCI::yRotationAndScale(TouchDataRef centOfRotData, TouchDataRef roomCoordData) {
	// have to do calculations, switching between both touch points 

	// translate to origin
	glm::dmat4 transMat(glm::translate(glm::dmat4(1.0), -1.0*centOfRotData->getCurrRoomPos()));
	glm::dmat4 rotMat = glm::dmat4(1.0);
	glm::dmat4 scaleMat = glm::dmat4(1.0);
	// movement of touch point is above threshold
	if(glm::abs(glm::length(roomCoordData->getPrevRoomPos()) - glm::length(roomCoordData->getCurrRoomPos())) > THRESH) {
		//std::cout<<"using the filtered pos in rotate and scale"<<std::endl;
		// rotate

		//// 0 vector guard
		glm::dvec3 prevDiffBetweenTwoPoints;
		if (glm::length(roomCoordData->getPrevRoomPos() - centOfRotData->getCurrRoomPos()) > 0.0) {
			prevDiffBetweenTwoPoints = glm::normalize(roomCoordData->getPrevRoomPos() - centOfRotData->getCurrRoomPos()); // "it" is the current thing through the  for loop below
		} 

		//// 0 vector guard
		glm::dvec3 currDiffBetweenTwoPoints;
		if (glm::length(roomCoordData->getCurrRoomPos() - centOfRotData->getCurrRoomPos()) > 0.0) {
			currDiffBetweenTwoPoints = glm::normalize(roomCoordData->getCurrRoomPos() - centOfRotData->getCurrRoomPos());
		} 



		// both distances are normalized
		glm::dvec3 crossProd = glm::cross(prevDiffBetweenTwoPoints,currDiffBetweenTwoPoints);
		double theta = glm::acos(glm::dot(prevDiffBetweenTwoPoints,currDiffBetweenTwoPoints));
		if(crossProd.y < 0){
			theta = -theta;
		}

		//std::cout << "Rotation Angle Theta: " << theta << std::endl;
		// glm::rotate takes degrees! Madness.
		rotMat = glm::rotate(glm::dmat4(1.0) , glm::degrees(-theta), glm::dvec3(0.0, 1.0, 0.0));

		// scale
		double prevDistanceDiff = glm::length(roomCoordData->getPrevRoomPos() - centOfRotData->getCurrRoomPos());
		double currDistanceDiff = glm::length(roomCoordData->getCurrRoomPos() - centOfRotData->getCurrRoomPos());

		//std::cout << prevDistanceDiff/currDistanceDiff << std::endl;

		// might move this into a more general function
		// to test for crazy input
		/*if (glm::dvec3(prevDistanceDiff/currDistanceDiff)) {

		}*/

		glm::dvec3 scaleBy = glm::dvec3(prevDistanceDiff/currDistanceDiff);
		scaleMat = glm::scale(
			glm::dmat4(1.0),
			scaleBy); 

	}


	// translate back
	glm::dmat4 transBack(glm::translate(glm::dmat4(1.0), centOfRotData->getCurrRoomPos()));

	// combine transforms
	glm::dmat4 yRotScaleMat = cFrameMgr->getRoomToVirtualSpaceFrame() * transBack * scaleMat *rotMat * transMat;
	cFrameMgr->setRoomToVirtualSpaceFrame(yRotScaleMat);

}