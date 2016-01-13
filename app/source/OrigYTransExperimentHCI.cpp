
#include "OrigYTransExperimentHCI.h"

#define FINGER_MOVEMENT_THRESHOLD 0.005
#define AXIS_DIFFERENCE_THRESHOLD  1.22
#define ANGLE_DIFFERENCE_THRESHOLD 0.349
#define YTRANS_ANGLE_DIFFERENCE_THRESHOLD 2.0943951
#define MOVEMENT_TIME_WINDOW 0.06
#define HISTORY_SIZE 50
#define ALPHA_SMOOTHING 0.1
#define FINGER_AXIS_ANGLE_LOW_THRESH 1.04719755
#define FINGER_AXIS_ANGLE_HIGH_THRESH 2.0943951
#define AXIS_CHANGE_THRESHOLD 0.34906585 //20 deg
#define ANGLE_AXIS_CHANGE_THRESHOLD 0.0034906585

OrigYTransExperimentHCI::OrigYTransExperimentHCI(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr, TextureMgrRef texMan, FeedbackRef feedback) : AbstractHCI(cFrameMgr, feedback)
{
	offAxisCamera = std::dynamic_pointer_cast<MinVR::CameraOffAxis>(camera);
	this->texMan = texMan;

	_touch1IsValid = false;
	_touch2IsValid = false;
	_rotating = false;
	_lastRotationAxis = glm::dvec3(1,0,0);
	_lastRightAxis = glm::dvec3(1.0, 0, 0);
	_centerAxis = glm::dvec3(0.0);
		
}

OrigYTransExperimentHCI::~OrigYTransExperimentHCI()
{
}

void OrigYTransExperimentHCI::initializeContextSpecificVars(int threadId,MinVR::WindowRef window)
{
}

void OrigYTransExperimentHCI::reset()
{
	_touch1IsValid = false;
	_touch2IsValid = false;
	_rotating = false;
	_lastRotationAxis = glm::dvec3(1,0,0);
	_lastRightAxis = glm::dvec3(1.0, 0, 0);
	_centerAxis = glm::dvec3(0.0);
}

void OrigYTransExperimentHCI::update(const std::vector<MinVR::EventRef> &events)
{
	glm::dmat4 rightTracker, leftTracker;
    bool updateRightTracker = false;
    bool updateLeftTracker = false;

    for (int i=0; i < events.size(); i++) {
		std::string name = events[i]->getName();
		if (boost::algorithm::starts_with(name, "TUIO_Cursor_up")) {
			int id = events[i]->getId();
	    	offerTouchUp(id);
		}
		else if (boost::algorithm::starts_with(name, "TUIO_Cursor_down")) {
			// Cursor DOWN event
			offerTouchDown(events[i]);
		}
		else if (boost::algorithm::starts_with(name, "TUIO_CursorMove")) {
			// Cursor MOVE event
			offerTouchMove(events[i]);
			
		}
		else if (name == "Hand_Tracker1") {	
			//Right hand
			rightTracker = events[i]->getCoordinateFrameData();
			updateRightTracker = true;
		}
		else if (name == "Hand_Tracker2") {
			// left hand
			leftTracker = events[i]->getCoordinateFrameData();
			updateLeftTracker = true;
		}
    }
    if (updateRightTracker && updateLeftTracker) {
		updateTrackers(rightTracker, leftTracker);
    }
	// give feedback object the touch data so
	// it can draw touch positions
	feedback->registeredTouchData = registeredTouchData;
}

void OrigYTransExperimentHCI::draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window)
{
}

bool OrigYTransExperimentHCI::offerTouchDown(MinVR::EventRef event)
{
	glm::dvec3 roomCoord = convertScreenToRoomCoordinates(event->get2DData());
	TouchDataRef info(new TouchData(event, roomCoord));
	registeredTouchData.insert(std::pair<int, TouchDataRef>(event->getId(), info));
	//info.id = id;
	//info.screenPos = pos;
	//info.lastScreenPos = pos;
	//info.pos = convertScreenToRoomCoordinates(pos);
	//info.lastPos = convertScreenToRoomCoordinates(pos);
	//info.isValid = true;
	
	// Try to remove touches from leaning on the table
	//TODO update these numbers with the new screen size
	if (abs(info->getCurrRoomPos().x) > 1.75 || abs(info->getCurrRoomPos().z) > 1.25) {
		return false;
	}
	
	if (!_touch1IsValid) {
		_touch1IsValid = true;
		_touch1 = info;
		determineTouchToHandCoorespondence(_touch1);
		if (_touch2IsValid) {
			if (MinVR::ConfigVal("CenterAlwaysAtOrigin", false, false)){
				_centerAxis = glm::dvec3((glm::column(cFrameMgr->getVirtualToRoomSpaceFrame(), 3)));
			}
			else {
				_centerAxis = _touch1->getCurrRoomPos() + (0.5*(_touch2->getCurrRoomPos()-_touch1->getCurrRoomPos()));
			}
			//feedback->centOfRot = _centerAxis;
			//_feedbackWidget->setFingerIndicatorPositions(_touch1.pos, _touch2.pos);
		}
		//cout << "Touch1 down"<<endl;
		return true;
	}
	else if (!_touch2IsValid) {
		_touch2IsValid = true;
		_touch2 = info;
		determineTouchToHandCoorespondence(_touch2);
		if(_touch1IsValid) {
			if (MinVR::ConfigVal("CenterAlwaysAtOrigin", false, false)){
				_centerAxis = glm::dvec3((glm::column(cFrameMgr->getVirtualToRoomSpaceFrame(), 3)));
			}
			else {
				_centerAxis = _touch1->getCurrRoomPos() + (0.5*(_touch2->getCurrRoomPos()-_touch1->getCurrRoomPos()));
			}
			//feedback->centOfRot = _centerAxis;
			//_feedbackWidget->setFingerIndicatorPositions(_touch1.pos, _touch2.pos);
		}
		//cout << "Touch2 down"<<endl;
		return true;
	}

	return false;
}

bool OrigYTransExperimentHCI::offerTouchUp(int id)
{
	if (_touch1IsValid && _touch1->getCurrentEvent()->getId() == id) {
		_touch1IsValid = false;
		//_roomToWidgetHemisphere = CoordinateFrame();
		//_feedbackWidget->resetGlobe();
		//_touch1Moves.clear();
		_leftAxisHistory.clear();
		_rightAxisHistory.clear();
		//cout << "Touch1 up"<<endl;
	}
	else if (_touch2IsValid && _touch2->getCurrentEvent()->getId() == id) {
		_touch2IsValid = false;
		//_roomToWidgetHemisphere = CoordinateFrame();
		//_feedbackWidget->resetGlobe();
		//_touch2Moves.clear();
		_leftAxisHistory.clear();
		_rightAxisHistory.clear();
		//cout << "Touch2 up"<<endl;
	}

	std::map<int, TouchDataRef>::iterator it = registeredTouchData.find(id); 

	if (it != registeredTouchData.end()) { // if id is found
		registeredTouchData.erase(it);	   //erase value associate with that it
		//std::cout << "UP" <<std::endl;
		return true;
	}
	else {
		std::cout<<"Could not find id="<<id<<" in registeredTouchData"<<std::endl;
	}

	return false;	
}

void OrigYTransExperimentHCI::offerTouchMove(MinVR::EventRef event)
{
	bool axisChanged = false;
	if (_touch1IsValid && event->getId() == _touch1->getCurrentEvent()->getId()) {
		_touch1->setCurrRoomPos(convertScreenToRoomCoordinates(glm::dvec2(event->get4DData())));
		_touch1->setCurrentEvent(event);
		movement move;
		move.touchName = "touch1";
		move.distance = glm::length(_touch1->getCurrRoomPos() - _touch1->getPrevRoomPos());
		move.timeStamp = boost::posix_time::microsec_clock::local_time();
		_touch1Moves.push_back(move);
		
	}
	else if (_touch2IsValid && event->getId() == _touch2->getCurrentEvent()->getId()) {
		_touch2->setCurrRoomPos(convertScreenToRoomCoordinates(glm::dvec2(event->get4DData())));
		_touch2->setCurrentEvent(event);
		movement move;
		move.touchName = "touch2";
		move.distance = glm::length(_touch2->getCurrRoomPos() - _touch2->getPrevRoomPos());
		move.timeStamp = boost::posix_time::microsec_clock::local_time();
		_touch2Moves.push_back(move);
	}

	std::map<int, TouchDataRef>::iterator it = registeredTouchData.find(event->getId()); 
	//std::cout << "Move " << events[i]->getId() <<std::endl;

	if (it != registeredTouchData.end()) { // if id is found
		glm::dvec2 screenCoord (event->get4DData());
		glm::dvec3 roomCoord = convertScreenToRoomCoordinates(glm::dvec2(event->get4DData()));

		// update map
		it->second->setCurrentEvent(event);
		it->second->setCurrRoomPos(roomCoord);
	}
}


double OrigYTransExperimentHCI::getTotalMovement(std::vector<movement> moves)
{
	// First we remove all the movements that happened later than the movement window
	boost::posix_time::ptime curTime, timeDiffTime;
	curTime = boost::posix_time::microsec_clock::local_time();
	int lastElementToRemove = -1;
	for(int i=0; i<moves.size(); i++) {
		double timeDiff = (curTime - moves[i].timeStamp).total_milliseconds();
		if (timeDiff > MOVEMENT_TIME_WINDOW) {
			lastElementToRemove = i;
		}
		else {
			break;
		}
	}
	if (lastElementToRemove != -1) {
		std::vector<movement>::const_iterator last = moves.begin() + lastElementToRemove + 1;
		moves.erase(moves.begin(), last);
	}
	
	// Total the distance for the movements within the movement time window
	float totalDistance = 0;
	for(int i=0; i<moves.size();i++) {
		totalDistance += moves[i].distance;
	}

	return totalDistance;
}

bool OrigYTransExperimentHCI::testForCrazyManipulation(const glm::dmat4& xFrame)
{
	float angle;
	glm::dvec3 axis;
	glm::dquat quat = glm::toQuat(glm::dmat3(xFrame));
	angle = glm::angle(quat);
	axis = glm::axis(quat);
	if (glm::length(glm::dvec3(glm::column(xFrame, 3))) > 0.75 || angle > 90) { //TODO: should the angle be radians pi/2?
		return true;
	}
	return false;
}

glm::dvec3 OrigYTransExperimentHCI::convertScreenToRoomCoordinates(glm::dvec2 screenCoords) {
	glm::dvec3 xVec = offAxisCamera->getTopRight() - offAxisCamera->getTopLeft();
	glm::dvec3 yVec = offAxisCamera->getBottomRight() - offAxisCamera->getTopRight();
	return offAxisCamera->getTopLeft() + (screenCoords.x * xVec) + (screenCoords.y * yVec);
}

void OrigYTransExperimentHCI::determineTouchToHandCoorespondence(TouchDataRef touch) {
	// Use the distances between touch points and the tracker to identify which hand they belong to.
	float distToLeft = glm::length(glm::dvec3(glm::column(_currentLeftTrackerFrame, 3)) - touch->getCurrRoomPos());
	float distToRight = glm::length(glm::dvec3(glm::column(_currentRightTrackerFrame, 3)) - touch->getCurrRoomPos());
	if(distToLeft < distToRight) {
		touch->setBelongTo(TouchData::LEFT_HAND);
	}
	else {
		touch->setBelongTo(TouchData::RIGHT_HAND);
	}
}

void OrigYTransExperimentHCI::updateTrackers(const glm::dmat4 &rightTrackerFrame, const glm::dmat4 &leftTrackerFrame)
{
	_previousRightTrackerFrame = _currentRightTrackerFrame;
	_previousLeftTrackerFrame = _currentLeftTrackerFrame;
	_currentRightTrackerFrame = rightTrackerFrame;
	_currentLeftTrackerFrame = leftTrackerFrame;
	
	// We only do a rotation around the x and z axis if the touch points haven't moved much recently,
	// and if the rotations are moving in the same direction
	
	if (_touch1IsValid && _touch2IsValid && getTotalMovement(_touch1Moves) < FINGER_MOVEMENT_THRESHOLD && 
		getTotalMovement(_touch2Moves) < FINGER_MOVEMENT_THRESHOLD) {
		
		_translating = false;

		//feedback->displayText = "rotating";
		
		// If the two touch points are on the same hand we just assume that it is rotating if the fingers are still
		// Use the distances between touch points and the tracker to identify which hand they belong to.
		if(_touch1->getBelongTo() == TouchData::LEFT_HAND && _touch2->getBelongTo() == TouchData::LEFT_HAND) {
			// both point are on the left hand
			
		}
		else if(_touch1->getBelongTo() == TouchData::RIGHT_HAND && _touch2->getBelongTo() == TouchData::RIGHT_HAND) {
			// both points are on right hand
			
		}
		else {
			glm::dvec3 oldLeftVec, oldRightVec;
			glm::dvec3 curLeftVec, curRightVec;
			if (_touch1->getBelongTo() == TouchData::LEFT_HAND) {
				oldLeftVec = glm::normalize(glm::dvec3(glm::column(_previousLeftTrackerFrame, 3)) - _touch1->getCurrRoomPos());
				curLeftVec = glm::normalize(glm::dvec3(glm::column(_currentLeftTrackerFrame, 3)) - _touch1->getCurrRoomPos());
				oldRightVec = glm::normalize(glm::dvec3(glm::column(_previousRightTrackerFrame, 3)) - _touch2->getCurrRoomPos());
				curRightVec = glm::normalize(glm::dvec3(glm::column(_currentRightTrackerFrame, 3)) - _touch2->getCurrRoomPos());
			}
			else {
				oldRightVec = glm::normalize(glm::dvec3(glm::column(_previousRightTrackerFrame, 3)) - _touch1->getCurrRoomPos());
				curRightVec = glm::normalize(glm::dvec3(glm::column(_currentRightTrackerFrame, 3)) - _touch1->getCurrRoomPos());
				oldLeftVec = glm::normalize(glm::dvec3(glm::column(_previousLeftTrackerFrame, 3)) - _touch2->getCurrRoomPos());
				curLeftVec = glm::normalize(glm::dvec3(glm::column(_currentLeftTrackerFrame, 3)) - _touch2->getCurrRoomPos());
			}
			
			glm::dvec3 rightAxis = glm::cross(curRightVec, oldRightVec);
			glm::dvec3 leftAxis = glm::cross(curLeftVec, oldLeftVec);
			double rightAngle = glm::acos(glm::clamp(glm::dot(curRightVec, oldRightVec), -1.0, 1.0));
			double leftAngle = glm::acos(glm::clamp(glm::dot(curLeftVec, oldLeftVec), -1.0, 1.0));
			double rotationAngle = (rightAngle + leftAngle)/2.0;
		
			//TEST to get rid of a normalize zero vector
			if (glm::epsilonEqual(rightAngle, 0.0, 0.00001) || glm::epsilonEqual(leftAngle, 0.0, 0.000001)) {
				return;
			}

			glm::dvec3 rightAxisOnTable;
			glm::dvec3 leftAxisOnTable;
			
			/*
			Vector3 avgLeft = leftAxis;
			Vector3 avgRight = rightAxis;
			for(int i=_leftAxisHistory.size()-1; i >=0; i--) {
				avgLeft = ALPHA_SMOOTHING*_leftAxisHistory[i] + (1.0-ALPHA_SMOOTHING)*avgLeft;
				avgRight = ALPHA_SMOOTHING*_rightAxisHistory[i] + (1.0-ALPHA_SMOOTHING)*avgRight;
			}
			
			rightAxisOnTable = Vector3(avgRight.x, 0.0, avgRight.z);
			leftAxisOnTable = Vector3(avgLeft.x, 0.0, avgLeft.z);
			*/
			rightAxisOnTable = glm::dvec3(rightAxis.x, 0.0, rightAxis.z);
			leftAxisOnTable = glm::dvec3(leftAxis.x, 0.0, leftAxis.z);
			rightAxisOnTable = glm::normalize(rightAxisOnTable);
			leftAxisOnTable = glm::normalize(leftAxisOnTable);
		
			glm::dvec3 rotationAxis = glm::normalize((rightAxisOnTable + leftAxisOnTable)/2.0);
			double angleWithLastRotAxis = glm::acos(glm::clamp(glm::dot(rotationAxis, _lastRotationAxis), -1.0, 1.0));
			double angleBtwAxes = glm::acos(glm::clamp(glm::dot(glm::normalize(rightAxisOnTable), glm::normalize(leftAxisOnTable)), -1.0, 1.0));
			double angleWithFingerAxis = glm::acos(glm::clamp(glm::dot(rightAxisOnTable, glm::normalize(_touch1->getCurrRoomPos()-_touch2->getCurrRoomPos())), -1.0, 1.0));

			
			_lastLeftAxis = leftAxisOnTable;
			_lastRightAxis = rightAxisOnTable;
			
			double rightProjection = glm::dot(glm::normalize(rightAxis), rightAxisOnTable);
			double leftProjection = glm::dot(glm::normalize(leftAxis), leftAxisOnTable);
			rightAngle = rightProjection*rightAngle;
			leftAngle = leftProjection*leftAngle;
			rotationAngle = (rightAngle + leftAngle)/2.0;
			
			double angleDifference = glm::abs(rightAngle - leftAngle);
			
			glm::dvec3 rotationDir = glm::cross(rotationAxis, glm::dvec3(0,1,0));
			// Keep the rotationdir pointing the same direction regardless of the direction of rotation along it
			if (glm::dot(rotationDir, glm::cross(glm::normalize(_touch1->getCurrRoomPos()-_touch2->getCurrRoomPos()), glm::dvec3(0.0, 1.0, 0.0))) < 0) {
				rotationDir = -rotationDir;
			}
			//_feedbackWidget->setRotationIndicatorDirection(rotationDir);
			_lastRotationAxis = rotationAxis;
			
			
			//cout << "\t Difference btw axes = "<<angleBtwAxes*180.0/M_PI<<" diff btw angles = "<<angleDifference*180.0/M_PI<< " angle with finger axis: "<<angleWithFingerAxis*180.0/M_PI<< " RotationAngle: "<<rotationAngle*180.0/M_PI<<" Axis change: "<<angleWithLastRotAxis*180.0/M_PI<<endl;
			if (angleBtwAxes > YTRANS_ANGLE_DIFFERENCE_THRESHOLD && angleWithFingerAxis > FINGER_AXIS_ANGLE_LOW_THRESH && angleWithFingerAxis < FINGER_AXIS_ANGLE_HIGH_THRESH) {
				// Translate y
				double gain = 2.0;
				double distance = rotationAngle/ (M_PI/2.0);
				// Determine whether we are translating up or down based on the distance change between the hands
				if (glm::length(glm::dvec3(glm::column(_currentLeftTrackerFrame, 3)) - glm::dvec3(glm::column(_currentRightTrackerFrame, 3))) >= glm::length(glm::dvec3(glm::column(_previousLeftTrackerFrame, 3)) - glm::dvec3(glm::column(_previousRightTrackerFrame, 3)))) {
					distance = -distance;
				}
				//cout << "Translating y: "<<distance<<endl;
				feedback->displayText = "translating"; 
				glm::dvec3 translationVec(0.0, distance, 0.0);
				glm::dmat4 translation = glm::translate(glm::dmat4(1.0), translationVec);
				glm::dmat4 newFrame = cFrameMgr->getRoomToVirtualSpaceFrame() * translation;
				if (!testForCrazyManipulation(translation)) {//!testForManipulationOutsideOfBounds(newFrame)&&
					//_feedbackWidget->setYTranslation(translation);
					//_totalTranslationLength+=abs(distance);
					cFrameMgr->setRoomToVirtualSpaceFrame(newFrame);
				}
			} 
			else {
				_currRotationFrame = glm::dmat4(1.0);
				_rotating = false;
				feedback->displayText = "";

			}			
		}
	}
	else {
		_currRotationFrame = glm::dmat4(1.0);
		//_roomToWidgetHemisphere = CoordinateFrame();
		feedback->displayText = "";
		feedback->centOfRot.x = DBL_MAX;
		_translating = true;
		_rotating = false;
	}
}
