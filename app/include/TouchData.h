#ifndef TOUCHDATA_H_
#define TOUCHDATA_H_

#include "MVRCore/Event.H"
#include <glm/glm.hpp>
#include <memory>
#include "OneEuroFilter.h"
#include <GLFW/glfw3.h>
#include <glm/gtx/string_cast.hpp>

typedef std::shared_ptr<class TouchData> TouchDataRef;

class TouchData {

public:
	TouchData(MinVR::EventRef event, glm::dvec3 currRoomPos);
	virtual ~TouchData();
	void setCurrentEvent(MinVR::EventRef event);
	MinVR::EventRef getCurrentEvent();
	MinVR::EventRef getPreviousEvent();
	

	void setCurrRoomPos(glm::dvec3 pos);
	glm::dvec3 getCurrRoomPos();
	void setPrevRoomPos(glm::dvec3 pos);
	glm::dvec3 getPrevRoomPos();
	void setBelongTo(int hand);
	glm::dvec3 roomPositionDifference();
	int getBelongTo();
	std::string toString();

	enum Hand{
		RIGHT_HAND=1,
		LEFT_HAND
	};

private:
	MinVR::EventRef prevEvent;
	MinVR::EventRef currEvent;
	// these variables are set in the update function
	// so we can use the cframe to transform
	glm::dvec3 currRoomPos;
	glm::dvec3 prevRoomPos;
	std::shared_ptr<OneEuroFilter> xFilter;
	std::shared_ptr<OneEuroFilter> zFilter;
	int handBelongTo; // 1 is right hand 2 is left hand
};

#endif /* TOUCHDATA_H_ */