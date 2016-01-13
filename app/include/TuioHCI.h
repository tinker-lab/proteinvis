#ifndef TUIOHCI_H_
#define TUIOHCI_H_

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
#include "app/include/GLSLProgram.h"
#include "app/include/GPUMesh.h"
#include <MVRCore/Time.h>


class TuioHCI : public AbstractHCI {

public:
	TuioHCI(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr, TextureMgrRef texMan, FeedbackRef feedback);
	virtual ~TuioHCI();
	void update(const std::vector<MinVR::EventRef> &events);
    void draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window);
	glm::dvec3 convertScreenToRoomCoordinates(glm::dvec2 screenCoords);
	void initializeContextSpecificVars(int threadId,MinVR::WindowRef window);
	void TuioHCI::initVBO(int threadId);
	void initGL() ;
	

private:
	std::shared_ptr<MinVR::CameraOffAxis> offAxisCamera;
	std::map<int, TouchDataRef> registeredTouchData; 
	std::shared_ptr<GPUMesh> cubeMesh;
	std::shared_ptr<GLSLProgram> shader;
	std::map<int, GLuint> _vboId;
	TextureMgrRef texMan; 
	MinVR::TimeStamp startTime;


};

#endif /* TUIOHCI_H_ */