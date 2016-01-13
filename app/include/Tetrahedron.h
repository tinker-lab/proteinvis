#ifndef TETRAHEDRON_H
#define TETRAHEDRON_H

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

typedef std::shared_ptr<class Tetrahedron> TetrahedronRef;
static const int numTetraEdges = 6;
static const double guideCylinderRadius = 0.005;

class Tetrahedron {
public:
	Tetrahedron(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr, TextureMgrRef texMan, double errorMargin);
	~Tetrahedron();
	void initializeContextSpecificVars(int threadId);
	void initVBO(int threadId);
	void initGL() ;

	glm::dvec3 Tetrahedron::getPosition(double latitude, double longitude); 

	void draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window, std::string textureName, glm::dmat4 transMat,std::string color1, std::string color2, std::string color3, std::string color4, std::string color5 );
    void makeCylinder(glm::dvec3 pointA, glm::dvec3 pointB, double radius = 0.02);
	void makeSphere(glm::dvec3 center);
	// tetraVertices
	glm::dvec3 pointA;
	glm::dvec3 pointB;
	glm::dvec3 pointC;
	glm::dvec3 pointD;
	
private:
	std::map<int, GLuint> _vboId;
	std::shared_ptr<MinVR::CameraOffAxis> offAxisCamera;
	
	GPUMeshRef cylinderMesh; // holds six cylinders
	GPUMeshRef sphereMesh; //only one sphere
	int GPUCylinderOffset;
	TextureMgrRef texMan; 
	CFrameMgrRef cFrameMgr;
	std::shared_ptr<GLSLProgram> tetraShader;

	std::vector<GPUMesh::Vertex> cylinderData;
	std::vector<int> cylinderIndices;
	std::vector<GPUMesh::Vertex> sphereData;
	std::vector<int> sphereIndices;
	std::vector<GPUMesh::Vertex> guideCylinderData;
	std::vector<int> guideCylinderIndices;

	double sphereRadius;
    double guideCylinderLength;

	

};

#endif /* TETRAHEDRON_H_ */
