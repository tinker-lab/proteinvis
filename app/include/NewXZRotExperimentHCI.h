#ifndef NEWXZROTEXPERIMENTHCI_h_
#define NEWXZROTEXPERIMENTHCI_h_

#include "gl/glew.h"
#include <glfw/glfw3.h>
#include "abstracthci.h"
#include "mvrcore/abstractcamera.h"
#include <mvrcore/cameraoffaxis.h>
#include "cframemgr.h"
#include "app/include/gpumesh.h"
#include "app/include/glslprogram.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mvrcore/datafileutils.h>
#include "app/include/texturemgr.h"
#include "app/include/touchdata.h"
#include <glm/gtx/string_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <mvrcore/time.h>
#include <iterator>


class NewXZRotExperimentHCI : public AbstractHCI {

public:
	NewXZRotExperimentHCI(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr, TextureMgrRef textMan, FeedbackRef feedback);
	virtual ~NewXZRotExperimentHCI();
	void update(const std::vector<MinVR::EventRef> &events);
	glm::dvec3 convertScreenToRoomCoordinates(glm::dvec2 screenCoords);
	void initializeContextSpecificVars(int threadId,MinVR::WindowRef window);

	void closestTouchPair(std::map<int, TouchDataRef> thisRegisteredTouchData, glm::dvec3 &pos1, glm::dvec3 &pos2, double &minDistance);
	void updateHandPos(const std::vector<MinVR::EventRef>& events);
	
	void reset();
	
	
	
private:
	std::shared_ptr<MinVR::CameraOffAxis> offAxisCamera;
	std::map<int, TouchDataRef> registeredTouchData; 
	std::shared_ptr<GPUMesh> cubeMesh;
	std::shared_ptr<GLSLProgram> shader;
	std::map<int, GLuint> _vboId;
	TextureMgrRef texMan; 
	MinVR::TimeStamp startTime;
	double prevScaleBy;
	MinVR::EventRef hand1;
	MinVR::EventRef hand2;
	glm::dvec3 currHandPos1;
	glm::dvec3 prevHandPos1;
	glm::dvec3 currHandPos2;
	glm::dvec3 prevHandPos2;
	int numTouchForHand1;
	int numTouchForHand2;
	bool xzRotFlag;
	bool initRoomPos;
	bool liftedFingers;
	glm::dvec3 initRoomTouchCentre;
	glm::dvec3 roomTouchCentre;
	bool centerRotMode;
	glm::dvec3 currHandToTouch;
	glm::dvec3 prevHandToTouch;
};

#endif /* NEWXZROTEXPERIMENTHCI_h_ */