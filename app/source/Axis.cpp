#include "app/include/Axis.h"

const double piTwelfths = M_PI/12.0;

Axis::Axis(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr, TextureMgrRef texMan) {
	this->cFrameMgr = cFrameMgr;
	this->texMan = texMan;
	offAxisCamera = std::dynamic_pointer_cast<MinVR::CameraOffAxis>(camera);


}

Axis::~Axis() {

}


void Axis::initializeContextSpecificVars(int threadId,MinVR::WindowRef window) {

	initVBO(threadId);
	initGL();

	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "openGL ERROR in initializeContextSpecificVars: "<< err << std::endl;
	}
}

void Axis::initVBO(int threadId){
	//making axis body
	std::vector<int> axisIndices;
	std::vector<GPUMesh::Vertex> axisData;
	GPUMesh::Vertex axisVert;
	for(int i=0; i < 25; i++){
		axisVert.position = glm::dvec3(0.8f,0.1*glm::cos(i*piTwelfths),0.1*glm::sin(i*piTwelfths));
		axisVert.normal =glm::normalize(glm::dvec3(0.0f, glm::cos(i*piTwelfths), glm::sin(i*piTwelfths)));
		axisVert.texCoord0 = glm::dvec2(0.0, 0.0);
		axisData.push_back(axisVert);
		axisIndices.push_back(axisData.size()-1);

		axisVert.position = glm::dvec3(0.0f, 0.1*glm::cos(i*piTwelfths), 0.1*glm::sin(i*piTwelfths));
		axisData.push_back(axisVert);
		axisIndices.push_back(axisData.size()-1);
	}


	//initialize axisMesh Object
	axisMesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*axisData.size(), sizeof(int)*axisIndices.size(), 0, axisData,sizeof(int)*axisIndices.size(), &axisIndices[0]));


	std::vector<int> sphereIndices;
	std::vector<GPUMesh::Vertex> sphereData;
	GPUMesh::Vertex sphereVert;

	// fill up the empty space
	const float STACKS = 40.0f; // longitudes
	const float SLICES = 60.0f; // latitudes
	const float latUnit = 180/STACKS;
	const float lonUnit = 360/SLICES;
	float curr_lat;
	float curr_lon;
	glm::dvec3 newVertex;
	glm::dvec3 nextVertex;
	glm::dvec3 newNormal; // can't create normals until we have 3 points...each vertex must have its own normal

	for (int i = 0; i < STACKS + 1; i++) { // stacks is outer loop
		curr_lat = i*latUnit;

		for (int k = 0; k < SLICES + 1; k++) {            
			curr_lon = k*lonUnit;

			//first vertex
			sphereVert.position = 0.1 * getPosition(curr_lat, curr_lon);
			sphereVert.normal = sphereVert.position;
			sphereVert.texCoord0 = glm::dvec2(0.5,0.5);

			sphereData.push_back(sphereVert);
			sphereIndices.push_back(sphereData.size()-1);

			// second vertex
			sphereVert.position = 0.1 * getPosition(curr_lat + latUnit, curr_lon);
			sphereVert.normal = getPosition(curr_lat, curr_lon);
			sphereVert.texCoord0 = sphereVert.texCoord0;

			sphereData.push_back(sphereVert);
			sphereIndices.push_back(sphereData.size()-1);


			// Texture Coordinates
			/*cpuTexCoords.append(Vector2((1.0/SLICES)*k, (1.0/STACKS)*i));
			cpuTexCoords.append(Vector2((1.0/SLICES)*k, (1.0/STACKS)*(i+1)));*/
		}
	}

	sphereMesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*sphereData.size(), sizeof(int)*sphereIndices.size(), 0, sphereData,sizeof(int)*sphereIndices.size(), &sphereIndices[0]));

	float const pi_OverTwelve = M_PI/12.0;

	GLfloat tetraVertices[] = { 1.2f, 0.0f, 0.0f,  
		0.8, 0.17*glm::cos(3*pi_OverTwelve), 0.17*glm::sin(3*pi_OverTwelve),  
		0.8, -0.17*glm::cos(3*pi_OverTwelve), 0.17*glm::sin(3*pi_OverTwelve),

		0.8,-0.17*glm::cos(3*pi_OverTwelve), 0.17*glm::sin(3*pi_OverTwelve),  
		0.8, 0.0, 0.0,
		0.8, 0.17*glm::cos(3*pi_OverTwelve), 0.17*glm::sin(3*pi_OverTwelve)
		
	}; 

	// normal array
	glm::dvec3 tetraNormals[]   = { 							
		glm::cross(glm::dvec3(0.0f,0.0f,0.0f)-glm::dvec3(-0.4,0.1*glm::cos(24*pi_OverTwelve)+0.07,0.1*glm::sin(24*pi_OverTwelve)+0.1),glm::dvec3(0.0f,0.0f,0.0f)-glm::dvec3(-0.4,-0.1*glm::cos(24*pi_OverTwelve)-0.07,0.1*glm::sin(24*pi_OverTwelve)+0.1))
		
	};



	std::vector<int> tetraIndices;
	std::vector<GPUMesh::Vertex> tetraData;
	GPUMesh::Vertex tetraVert;
	for(int i=0; i<18; i = i+3){
		tetraVert.position = glm::dvec3(tetraVertices[i],tetraVertices[i+1],tetraVertices[i+2]);
		if(i<9){
			tetraVert.normal = tetraNormals[0];
		}
		else{
			tetraVert.normal = glm::dvec3(-1.0, 0.0, 0.0);
		}
		tetraVert.texCoord0 = glm::dvec2(0.3,0.9);
		tetraData.push_back(tetraVert);
		tetraIndices.push_back(tetraData.size()-1);

	}

	tetraMesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*tetraData.size(), sizeof(int)*tetraIndices.size(),0,tetraData,sizeof(int)*tetraIndices.size(), &tetraIndices[0]));


	_vboId[threadId] = GLuint(0);

	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "GLERROR initVBO: "<<err<<std::endl;
	}
}

void Axis::initGL() {

	glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

	//load in shaders
	std::map<std::string, std::string> args, dummyArgs;
	shader.reset(new GLSLProgram());
	shader->compileShader(MinVR::DataFileUtils::findDataFile("phong.vert").c_str(), GLSLShader::VERTEX, dummyArgs);
	shader->compileShader(MinVR::DataFileUtils::findDataFile("phong.frag").c_str(), GLSLShader::FRAGMENT, args);
	shader->link();
	

}

glm::dvec3 Axis::getPosition(double latitude, double longitude) {
  // TOAD: Given a latitude and longitude as input, return the corresponding 3D x,y,z position 
  // on your Earth geometry
  
  // north pole is 0 deg latitude, 0 deg longitude
  // south pole is 180 lat, 0 longitude
  // longitude increase means counter clockwise, as viewed from north pole
  
  double latRad = glm::radians(latitude);
  double lonRad = glm::radians(longitude); 
  
  float z = sin(latRad)*cos(lonRad);
  float x = sin(latRad)*sin(lonRad);
  float y = cos(latRad);
  
  return glm::dvec3(x, y, z);
}

void Axis::draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window, std::string textureName){

	const int numAxisIndices = (int)(axisMesh->getFilledIndexByteSize()/sizeof(int));
	const int numSphereIndices = (int)(sphereMesh->getFilledIndexByteSize()/sizeof(int));
	const int numTetraIndices = (int)(tetraMesh->getFilledIndexByteSize()/sizeof(int));

	shader->use();
	shader->setUniform("projection_mat", offAxisCamera->getLastAppliedProjectionMatrix());
	shader->setUniform("view_mat", offAxisCamera->getLastAppliedViewMatrix());
	
	glm::dvec3 eye_world = glm::dvec3(glm::column(glm::inverse(offAxisCamera->getLastAppliedViewMatrix()), 3));
	shader->setUniform("eye_world", eye_world);
	texMan->getTexture(threadId, textureName)->bind(0);
	shader->setUniform("textureSampler", 0);

	// Begin drawing axes
	glBindVertexArray(axisMesh->getVAOID());

	// x-axis
	//camera->setObjectToWorldMatrix(cFrameMgr->getVirtualToRoomSpaceFrame());
	shader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix());
	glDrawArrays(GL_TRIANGLE_STRIP, 0, numAxisIndices);

	//Draw sphere
	glBindVertexArray(sphereMesh->getVAOID());
	glDrawArrays(GL_TRIANGLE_STRIP, 0, numSphereIndices);

	// draw Arrow head
	glBindVertexArray(tetraMesh->getVAOID());

	// 4 faces
	for (int t = 0; t < 4; t++) {
		glm::dmat4 tetraRotMat1 = glm::rotate(glm::dmat4(1.0), t*90.0, glm::dvec3(1.0, 0.0, 0.0));
		shader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix()*tetraRotMat1);
		glDrawArrays(GL_TRIANGLES, 0, numTetraIndices);
	} 
}