#ifndef NewAnchoredExperimentHCI_H_
#define NewAnchoredExperimentHCI_H_

#include "GL/glew.h"
#include <GLFW/glfw3.h>
#include "AbstractHCI.h"
#include "MVRCore/AbstractCamera.H"
#include <MVRCore/CameraOffAxis.H>
#include "CFrameMgr.H"
#include "app/include/GPUMesh.h"
#include "app/include/GLSLProgram.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <MVRCore/DataFileUtils.H>
#include "app/include/TextureMgr.h"
#include "app/include/TouchData.h"
#include <glm/gtx/string_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <MVRCore/Time.h>
#include <iterator>


class NewAnchoredExperimentHCI : public AbstractHCI {

public:
	NewAnchoredExperimentHCI(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr, TextureMgrRef textMan, FeedbackRef feedback);
	virtual ~NewAnchoredExperimentHCI();
	void update(const std::vector<MinVR::EventRef> &events);
 
	glm::dvec3 convertScreenToRoomCoordinates(glm::dvec2 screenCoords);
	void initializeContextSpecificVars(int threadId,MinVR::WindowRef window);
	


	void closestTouchPair(std::map<int, TouchDataRef> thisRegisteredTouchData, glm::dvec3 &pos1, glm::dvec3 &pos2, double &minDistance);
	void updateHandPos(const std::vector<MinVR::EventRef>& events);

	// matrix transforms
	void translate(glm::dmat4 transMat);
	void yRotationAndScale(TouchDataRef centOfRotData, TouchDataRef otherTouchData);

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

#endif /* NewAnchoredExperimentHCI_H_ */