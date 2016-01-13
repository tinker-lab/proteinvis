#ifndef AXIS_H_
#define AXIS_H_

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



#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

typedef std::shared_ptr<class Axis> AxisRef;

class Axis {

public:
	Axis(MinVR::AbstractCameraRef camera,CFrameMgrRef cFrameMgr,TextureMgrRef texMan);
	virtual ~Axis();
	void draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window, std::string textureName);
	glm::dvec3 getPosition(double latitude, double longitude);
	void initializeContextSpecificVars(int threadId,MinVR::WindowRef window);
	void initVBO(int threadId);
	void initGL() ;

private:
	std::map<int, GLuint> _vboId;
	std::shared_ptr<MinVR::CameraOffAxis> offAxisCamera;
	std::shared_ptr<GLSLProgram> shader;
	std::shared_ptr<GPUMesh> axisMesh;
	std::shared_ptr<GPUMesh> sphereMesh;
	std::shared_ptr<GPUMesh> tetraMesh;
	TextureMgrRef texMan; 
	std::shared_ptr<CFrameMgr> cFrameMgr;
	
};

#endif /* AXIS_H_ */