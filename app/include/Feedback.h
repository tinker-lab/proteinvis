#ifndef FEEDBACK_H_
#define FEEDBACK_H_

#include "app/include/GPUMesh.h"
#include <GLFW/glfw3.h>
#include "MVRCore/Event.H"
#include <glm/glm.hpp>
#include <memory>
#include <glm/gtx/string_cast.hpp>
#include <glm/glm.hpp>
#include "CFrameMgr.H"
#include "app/include/GLSLProgram.h"
#include <MVRCore/CameraOffAxis.H>
#include <MVRCore/DataFileUtils.H>
#include <glm/gtc/matrix_transform.hpp>
#include "app/include/TextureMgr.h"
#include "MVRCore/AbstractCamera.H"
#include "MVRCore/AbstractWindow.H"
#include "app/include/TouchData.h"

typedef std::shared_ptr<class Feedback> FeedbackRef;

class Feedback {

public:
	Feedback(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr, TextureMgrRef texMan);
	virtual ~Feedback();
	void draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window);
	void initializeContextSpecificVars(int threadId,MinVR::WindowRef window);
	void initVBO(int threadId, MinVR::WindowRef window);
	void initGL() ;
	glm::dvec3 convertScreenToRoomCoordinates(glm::dvec2 screenCoords);
	std::string displayText;
	std::map<int, TouchDataRef> registeredTouchData;
	glm::dvec3 centOfRot;
	void initVBOB(int threadId, MinVR::WindowRef window, std::string textureStr, GPUMeshRef &mesh);
	bool displayPractice;
	

private:
	std::map<int, GLuint> _vboId;
	std::shared_ptr<MinVR::CameraOffAxis> offAxisCamera;
	std::shared_ptr<GLSLProgram> shader;
	std::shared_ptr<GPUMesh> quadMesh;
	GPUMeshRef touchMesh;
	GPUMeshRef betweenMesh;
	GPUMeshRef bgMesh;
	TextureMgrRef texMan; 
	std::shared_ptr<CFrameMgr> cFrameMgr;
	
	
	
};

#endif /* Feedback_H_ */