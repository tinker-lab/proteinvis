#include "app/include/Feedback.h"

Feedback::Feedback(MinVR::AbstractCameraRef camera,CFrameMgrRef cFrameMgr,TextureMgrRef texMan) {
	this->cFrameMgr = cFrameMgr;
	this->texMan = texMan;
	/*this->registeredTouchData = touchData;*/
	offAxisCamera = std::dynamic_pointer_cast<MinVR::CameraOffAxis>(camera);
	displayText = "";
	displayPractice = false;
	centOfRot = glm::dvec3(DBL_MAX, 0.0, 0.0); // some crappy initial value
}

Feedback::~Feedback() {
}


void Feedback::initializeContextSpecificVars(int threadId,MinVR::WindowRef window) {
	
	initVBO(threadId, window);
	initGL();

	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "openGL ERROR in initializeContextSpecificVars: "<< err << std::endl;
	}
}



void Feedback::initVBOB (int threadId, MinVR::WindowRef window, std::string textureStr, GPUMeshRef &mesh) {

	std::tr1::shared_ptr<Texture> texture = texMan->getTexture(threadId, textureStr);
	float texHeight = texture->getHeight();
	float texWidth = texture->getWidth();
	float windowHeight = window->getHeight();
	float windowWidth = window->getWidth();

	float texHeightScreen =  texHeight/windowHeight;
	float texWidthScreen = texWidth/windowWidth;

	glm::dvec3 quad = glm::abs(convertScreenToRoomCoordinates(glm::dvec2(texWidthScreen+0.5, texHeightScreen+0.5)));

	std::vector<int> quadIndices;
	std::vector<GPUMesh::Vertex> quadData;
	GPUMesh::Vertex quadVert;

	quadVert.position = glm::dvec3(-quad.x, 0.0, -quad.z);
	quadVert.normal = glm::dvec3(0.0, 1.0, 0.0);
	quadVert.texCoord0 = glm::dvec2(0.0, 1.0);
	quadData.push_back(quadVert);
	quadIndices.push_back(quadData.size()-1);

	quadVert.position = glm::dvec3(-quad.x, 0.0, quad.z);
	quadVert.texCoord0 = glm::dvec2(0.0, 0.0);
	quadData.push_back(quadVert);
	quadIndices.push_back(quadData.size()-1);

	quadVert.position = glm::dvec3(quad.x, 0.0, -quad.z);
	quadVert.texCoord0 = glm::dvec2(1.0, 1.0);
	quadData.push_back(quadVert);
	quadIndices.push_back(quadData.size()-1);


	quadVert.position = glm::dvec3(quad.x, 0.0, quad.z);
	quadVert.texCoord0 = glm::dvec2(1.0, 0.0);
	quadData.push_back(quadVert);
	quadIndices.push_back(quadData.size()-1);


	mesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*quadData.size(), sizeof(int)*quadIndices.size(),0,quadData,sizeof(int)*quadIndices.size(), &quadIndices[0]));

	

}


void Feedback::initVBO(int threadId, MinVR::WindowRef window) {

	//initVBOB(threadId, window, "rotating", quadMesh); //
	initVBOB(threadId, window, "touch-smaller", touchMesh);
	initVBOB(threadId, window, "between", betweenMesh);
	//initVBOB(threadId, window, "centOfRot"); //
	//initVBOB(threadId, window, "vector");


	/////////////////////////////
	// VBO for displaying Text //
	/////////////////////////////
	std::vector<int> quadIndices;
	std::vector<GPUMesh::Vertex> quadData;
	GPUMesh::Vertex quadVert;

	float windowHeight = window->getHeight();
	float windowWidth = window->getWidth();
	float texHeight = texMan->getTexture(threadId, "rotating")->getHeight();
	float texWidth = texMan->getTexture(threadId, "rotating")->getWidth();
	float quadHeightScreen =  texHeight/windowHeight;
	float quadWidthScreen = texWidth/windowWidth;

	glm::dvec3 quad = glm::abs(convertScreenToRoomCoordinates(glm::dvec2(quadWidthScreen+0.5, quadHeightScreen+0.5)));

	//std::cout<<"quad: "<<glm::to_string(quad)<<std::endl;
	//std::cout<<"wind H: "<<windowHeight<<std::endl;
	//std::cout<<"wind W: "<<windowWidth<<std::endl;
	//std::cout<<"tex H: "<<texHeight<<std::endl;
	//std::cout<<"tex W: "<<texWidth<<std::endl;

	//std::cout << "quad Height Screen: " <<quadHeightScreen << std::endl;
	//std::cout << "quad W Screen: " << quadWidthScreen << std::endl;

	glm::dvec3 topleft = convertScreenToRoomCoordinates(glm::dvec2(0.1,0.9));
	//std::cout<<"topleft: "<<glm::to_string(topleft)<<std::endl;

	quadVert.position = glm::dvec3(topleft.x, 0.0, topleft.z);
	quadVert.normal = glm::dvec3(0.0, 1.0, 0.0);
	quadVert.texCoord0 = glm::dvec2(0.0, 1.0);
	quadData.push_back(quadVert);
	quadIndices.push_back(quadData.size()-1);

	quadVert.position = glm::dvec3(topleft.x, 0.0, topleft.z+quad.z);
	quadVert.texCoord0 = glm::dvec2(0.0, 0.0);
	quadData.push_back(quadVert);
	quadIndices.push_back(quadData.size()-1);

	quadVert.position = glm::dvec3(topleft.x+quad.x, 0.0, topleft.z);
	quadVert.texCoord0 = glm::dvec2(1.0, 1.0);
	quadData.push_back(quadVert);
	quadIndices.push_back(quadData.size()-1);


	quadVert.position = glm::dvec3(topleft.x+quad.x, 0.0, topleft.z+quad.z);
	quadVert.texCoord0 = glm::dvec2(1.0, 0.0);
	quadData.push_back(quadVert);
	quadIndices.push_back(quadData.size()-1);


	quadMesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*quadData.size(), sizeof(int)*quadIndices.size(),0,quadData,sizeof(int)*quadIndices.size(), &quadIndices[0]));

	

	//////////////////////////////
	//// VBO for Touch Feedback //
	//////////////////////////////
	//texHeight = texMan->getTexture(threadId, "touch")->getHeight();
	//texWidth = texMan->getTexture(threadId, "touch")->getWidth();
	//float touchHeightScreen =  texHeight/windowHeight;
	//float touchWidthScreen = texWidth/windowWidth;

	//glm::dvec3 touchQuad = glm::abs(convertScreenToRoomCoordinates(glm::dvec2(touchWidthScreen+0.5, touchHeightScreen+0.5)));

	//std::vector<int> touchIndices;
	//std::vector<GPUMesh::Vertex> touchData;
	//GPUMesh::Vertex touchVert;

	//touchVert.position = glm::dvec3(-touchQuad.x, 0.0, -touchQuad.z);
	//touchVert.normal = glm::dvec3(0.0, 1.0, 0.0);
	//touchVert.texCoord0 = glm::dvec2(0.0, 1.0);
	//touchData.push_back(touchVert);
	//touchIndices.push_back(touchData.size()-1);

	//touchVert.position = glm::dvec3(-touchQuad.x, 0.0, touchQuad.z);
	//touchVert.texCoord0 = glm::dvec2(0.0, 0.0);
	//touchData.push_back(touchVert);
	//touchIndices.push_back(touchData.size()-1);

	//touchVert.position = glm::dvec3(touchQuad.x, 0.0, -touchQuad.z);
	//touchVert.texCoord0 = glm::dvec2(1.0, 1.0);
	//touchData.push_back(touchVert);
	//touchIndices.push_back(touchData.size()-1);


	//touchVert.position = glm::dvec3(touchQuad.x, 0.0, touchQuad.z);
	//touchVert.texCoord0 = glm::dvec2(1.0, 0.0);
	//touchData.push_back(touchVert);
	//touchIndices.push_back(touchData.size()-1);


	//touchMesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*touchData.size(), sizeof(int)*touchIndices.size(),0,touchData,sizeof(int)*touchIndices.size(), &touchIndices[0]));

	//// vbo for centroid

	//// vbo for drawing vectors
}

void Feedback::initGL() {

	glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

	// Wait, do we even need to load shaders?
	//load in shaders
	std::map<std::string, std::string> args, dummyArgs;
	shader.reset(new GLSLProgram());
	shader->compileShader(MinVR::DataFileUtils::findDataFile("tex.vert").c_str(), GLSLShader::VERTEX, dummyArgs);
	shader->compileShader(MinVR::DataFileUtils::findDataFile("tex.frag").c_str(), GLSLShader::FRAGMENT, args);
	shader->link();

}

glm::dvec3 Feedback::convertScreenToRoomCoordinates(glm::dvec2 screenCoords) {
	glm::dvec3 xVec = offAxisCamera->getTopRight() - offAxisCamera->getTopLeft();
	glm::dvec3 yVec = offAxisCamera->getBottomRight() - offAxisCamera->getTopRight();
	return offAxisCamera->getTopLeft() + (screenCoords.x * xVec) + (screenCoords.y * yVec);
}

void Feedback::draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window) {

	//turn on blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shader->use();
	shader->setUniform("projection_mat", offAxisCamera->getLastAppliedProjectionMatrix());
	shader->setUniform("view_mat", offAxisCamera->getLastAppliedViewMatrix());

	const int numQuadIndices = (int)(quadMesh->getFilledIndexByteSize()/sizeof(int));
	const int numTouchIndices = (int)(touchMesh->getFilledIndexByteSize()/sizeof(int));
	const int numBetweenIndices = (int)(touchMesh->getFilledIndexByteSize()/sizeof(int));

	// if we are in some mode, displayText should be fine
	// displayText is modified through HCIs
	if (displayText != "") {
		

		//std::cout << "Display Text: " << displayText << std::endl;
		texMan->getTexture(threadId, displayText)->bind(1);
		shader->setUniform("textureSampler", 1);

		glm::dvec4 quadTranslate(0.0, 0.0, 0.0, 1.0);

		// draw text here, remember to mess with the shader with the alpha value
		glm::dmat4 quadAtCorner = glm::dmat4(1.0);
		quadAtCorner[3] = quadTranslate;
		camera->setObjectToWorldMatrix(quadAtCorner);
		shader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix());
		glBindVertexArray(quadMesh->getVAOID());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, numQuadIndices);

		
	}

	if (displayText == "between"){
	
		texMan->getTexture(threadId, displayText)->bind(1);
		shader->setUniform("textureSampler", 1);

		glm::dvec4 quadTranslate(0.0, 1.0, 0.0, 1.0);

		// draw text here, remember to mess with the shader with the alpha value
		glm::dmat4 quadAtCorner = glm::dmat4(1.0);
		quadAtCorner[3] = quadTranslate;
		camera->setObjectToWorldMatrix(quadAtCorner);
		shader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix());
		glBindVertexArray(betweenMesh->getVAOID());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, numBetweenIndices);
	
	} 

	if (displayPractice) {
		texMan->getTexture(threadId, "practice")->bind(1);
		shader->setUniform("textureSampler", 1);

		glm::dvec4 quadTranslate(2.5, 0.0, 0.0, 1.0);

		// draw text here, remember to mess with the shader with the alpha value
		glm::dmat4 quadAtCorner = glm::dmat4(1.0);
		quadAtCorner[3] = quadTranslate;
		camera->setObjectToWorldMatrix(quadAtCorner);
		shader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix());
		glBindVertexArray(quadMesh->getVAOID());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, numQuadIndices);
	}


	// draw the touch texture
	// somehow need the room position of the touch
	glBindVertexArray(touchMesh->getVAOID());
	texMan->getTexture(threadId, "touch2")->bind(2);
	shader->setUniform("textureSampler", 2);

	std::map<int, TouchDataRef>::iterator it;
	glm::dvec3 roomCoord;

	for(it = registeredTouchData.begin(); it != registeredTouchData.end(); ++it) {
		
		TouchDataRef event = it->second;
		roomCoord = event->getCurrRoomPos();
		
		// new matrix for each triangle
		camera->setObjectToWorldMatrix(glm::translate(glm::dmat4(1.0f), roomCoord));
		shader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix());

		// draw quad
		glDrawArrays(GL_TRIANGLE_STRIP, 0, numTouchIndices);
	}

	// draw center of rotation point for XZRotMode, if there is one.
	// logic controlled by the HCI

	// uses same VBO as touch
	if (centOfRot.x != DBL_MAX) {
		texMan->getTexture(threadId, "centOfRot")->bind(3);
		shader->setUniform("textureSampler", 3);
		camera->setObjectToWorldMatrix(glm::translate(glm::dmat4(1.0f), centOfRot));
		shader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix());

		// draw quad
		glDrawArrays(GL_TRIANGLE_STRIP, 0, numTouchIndices);
	}
	
	// turn off blending
	glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_BLEND);

}

