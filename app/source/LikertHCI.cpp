#include "app/include/LikertHCI.h"

//font stash stuff
#include <stdio.h>
#include <string.h>
#define FONTSTASH_IMPLEMENTATION
#include "fontstash.h"
#define GLFONTSTASH_IMPLEMENTATION
#include "glfontstash.h"

LikertHCI::LikertHCI(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr, TextureMgrRef texMan, FeedbackRef feedback) : AbstractHCI(cFrameMgr, feedback) {

	done = false;
	offAxisCamera = std::dynamic_pointer_cast<MinVR::CameraOffAxis>(camera);
	this->texMan = texMan;

	_cycleCount = 0;

	_questions = MinVR::splitStringIntoArray(MinVR::ConfigVal("LikertQuestions", ""));
	_answers = MinVR::splitStringIntoArray(MinVR::ConfigVal("LikertAnswers", ""));

	_maxNumLinesInQuestions = 0;
	for(int i=0; i < _questions.size(); i++) {
		_questions[i] = boost::replace_all_copy(_questions[i], "*", "\n");
		std::stringstream ss(_questions[i]);
		std::string line;
		int lineCount = 0;
		while(std::getline(ss, line, '\n')){
			lineCount++;
		}
		if (lineCount > _maxNumLinesInQuestions) {
			_maxNumLinesInQuestions = lineCount;
		}
	}

	std::vector<std::string> questionRangeStrings = MinVR::splitStringIntoArray(MinVR::ConfigVal("LikertQuestionRanges", "", false));
	for(int i=0; i< questionRangeStrings.size(); i++){
		glm::dvec2 range;
		std::string text = boost::replace_all_copy(questionRangeStrings[i], "_", ", ");
		MinVR::retypeString(text, range);
		glm::ivec2 irange ((int)range.x, (int)range.y);
		_questionRanges.push_back(irange);
	}

	std::vector<std::string> answerRangeStrings = MinVR::splitStringIntoArray(MinVR::ConfigVal("LikertAnswerRanges", "", false));
	for(int i=0; i < answerRangeStrings.size(); i++) {
		glm::dvec2 range;
		std::string text = boost::replace_all_copy(answerRangeStrings[i], "_", ", ");
		MinVR::retypeString(text, range);
		glm::ivec2 irange ((int)range.x, (int)range.y);
		_answerRanges.push_back(irange);
	}

	//Make all answers the same number of characters
	int maxSize = 0;
	_maxNumLinesInAnswers = 0;
	for(int i=0; i < _answers.size(); i++) {
		_answers[i] = boost::replace_all_copy(_answers[i], "*", "\n");
		std::stringstream ss(_answers[i]);
		std::string line;
		int lineCount = 0;
		while(std::getline(ss, line, '\n')){
			lineCount++;
			if (line.size() > maxSize) {
				maxSize = line.size();
			}
		}
		if (lineCount > _maxNumLinesInAnswers) {
			_maxNumLinesInAnswers = lineCount;
		}
	}

	for(int i=0; i < _answers.size(); i++) {
		std::stringstream ss(_answers[i]);
		std::string line;
		std::string fullLine;
		std::getline(ss, line, '\n'); // We only need to make the first line be the correct width
		int size = line.size();
		if (size < maxSize) {
			int lettersToAdd = maxSize - size;
			int prepend = glm::floor(lettersToAdd/2.0);
			std::string prependString = "";
			for(int j = 0; j < prepend; j++) {
				prependString += " ";
			}
			fullLine=prependString + line;

			std:string postString = "";
			for(int j=0; j < lettersToAdd-prepend; j++) {
				postString += " ";
			}
			fullLine += postString+ _answers[i].substr(line.size());
			_answers[i] = fullLine;
		}
	}

	_currentQuestion = _questionRanges[0].x;

	int numThreads = 1;

	_questionTextures.resize(numThreads);
	_questionSizes.resize(numThreads);
	_answerTextures.resize(numThreads);
	_answerSizes.resize(numThreads);

	for(int i=0; i < numThreads; i++) {
		_questionTextures[i].resize(_questions.size());
		_answerTextures[i].resize(_answers.size());
	}

	boost::posix_time::time_facet* facet = new boost::posix_time::time_facet();
	facet->format("%Y-%m-%d.%H.%M.%S");
	std::stringstream stream;
	stream.imbue(std::locale(stream.getloc(), facet));
	stream << boost::posix_time::second_clock::local_time();
	std::string eventStreamFile = "QuestionAnswers-" + stream.str() + ".txt";
	_answerRecorder.open(eventStreamFile);
}

LikertHCI::~LikertHCI()
{
	std::flush(_answerRecorder);
	_answerRecorder.close();
}

void LikertHCI::initializeContextSpecificVars(int threadId,MinVR::WindowRef window)
{
	
	//load in shaders
	std::map<std::string, std::string> args, dummyArgs;
	_shader.reset(new GLSLProgram());
	_shader->compileShader(MinVR::DataFileUtils::findDataFile("tex.vert").c_str(), GLSLShader::VERTEX, dummyArgs);
	_shader->compileShader(MinVR::DataFileUtils::findDataFile("tex.frag").c_str(), GLSLShader::FRAGMENT, args);
	_shader->link();

	initializeText(threadId, window);

	std::cout << "Likert initialization" << std::endl;
}

void LikertHCI::initializeText(int threadId, MinVR::WindowRef window)
{

	
	int fontNormal = FONS_INVALID;
	struct FONScontext* fs = nullptr;

	fs = glfonsCreate(512, 512, FONS_ZERO_TOPLEFT);
	if (fs == NULL) {
		BOOST_ASSERT_MSG(false, "Could not create stash.");
	}

	fontNormal = fonsAddFont(fs, "sans", MinVR::ConfigVal("RegularFontFile", "app/fonts/DroidSansMono.ttf", false).c_str());
	if (fontNormal == FONS_INVALID) {
		BOOST_ASSERT_MSG(false, "Could not add font normal.\n");
	}

	glUseProgram(0);
	glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT);

	GLuint textFBO;
	glGenFramebuffers(1, &textFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, textFBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	std::map<std::string, std::string> dummyArgs;
	std::shared_ptr<GLSLProgram> shader(new GLSLProgram());
	shader->compileShader(MinVR::DataFileUtils::findDataFile("textRendering.vert").c_str(), GLSLShader::VERTEX, dummyArgs);
	shader->compileShader(MinVR::DataFileUtils::findDataFile("textRendering.frag").c_str(), GLSLShader::FRAGMENT, dummyArgs);
	shader->link();
	shader->use();

	glClearColor(1.0, 1.0, 1.0, 1.0);

	initializeText(threadId, fs, fontNormal, _maxNumLinesInQuestions, 20.0f, shader, 72.0f, _questions, "QuestionText_", _questionTextures, _questionSizes);

	initializeText(threadId, fs, fontNormal, _maxNumLinesInAnswers, 15.0f, shader, 35.0f, _answers, "AnswerText_", _answerTextures, _answerSizes);

	double windowHeight = window->getHeight();
	double windowWidth = window->getWidth();
	_padding = 0.05;
	double numAnswersPerQuestion = 7.0;
	_availableWidth = glm::length(offAxisCamera->getBottomRight() - offAxisCamera->getBottomLeft()) - (2*_padding);
	_individualSize = _availableWidth/ (numAnswersPerQuestion);
	glm::dvec3 start(offAxisCamera->getBottomLeft().x + _padding + _individualSize/2.0, 0.0, 0.5);
	glm::dvec3 spacing(_individualSize, 0.0, 0.0);
	for(int i=0; i < _answers.size(); i++) {
		double texHeight = _answerTextures[0][i]->getHeight(); // all answers are the same dimensions;
		double texWidth = _answerTextures[0][i]->getWidth();
		double quadHeightScreen =  texHeight/windowHeight;
		double quadWidthScreen = texWidth/windowWidth;
		glm::dvec3 quad = glm::abs(convertScreenToRoomCoordinates(glm::dvec2(quadWidthScreen+0.5, quadHeightScreen+0.5)));
		_answerHeights.push_back(quad.z);

		glm::dvec2 size = _answerSizes[threadId][i];
		double halfWidth = _answerHeights[i] * size.x / size.y / 2.0f;
		double halfHeight = _answerHeights[i] / 2.0f;
		glm::dvec3 centerPt = start+ (double)i*spacing;
		glm::dvec3 low(-halfWidth, -0.001, -halfHeight);
		glm::dvec3 high(halfWidth, 0.001, halfHeight);
		AABox bounds(start + (double)i*spacing + low, start + (double)i*spacing +high);
		_answerBounds.push_back(bounds);
		
	}

	for(int i=0; i < _questions.size(); i++) {
		double texHeight = _questionTextures[0][i]->getHeight(); // all answers are the same dimensions;
		double texWidth = _questionTextures[0][i]->getWidth();
		double quadHeightScreen =  texHeight/windowHeight;
		double quadWidthScreen = texWidth/windowWidth;
		glm::dvec3 quad = glm::abs(convertScreenToRoomCoordinates(glm::dvec2(quadWidthScreen+0.5, quadHeightScreen+0.5)));
		_questionHeights.push_back(quad.z);	
	}
	
	glfonsDelete(fs);
	glUseProgram(0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &textFBO);

	glPopAttrib(); // restore viewport and enabled bits

	glEnable(GL_TEXTURE_2D);
	// restore the clear color
	glm::dvec3 clearColor = MinVR::ConfigVal("ClearColor", glm::dvec3(0.1), false);
	glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
}

void LikertHCI::initializeText(int threadId, FONScontext* fs, int fontNormal, int lineCount, float border, std::shared_ptr<GLSLProgram> shader, float textSize, const std::vector<std::string> &texts, std::string texKey, std::vector<std::vector<std::shared_ptr<Texture> > > &textures, std::vector<std::vector<glm::dvec2> > &sizes)
{
	float sx, sy, lh = 0;
	unsigned int white = glfonsRGBA(255,255,255,255);
	unsigned int gray = glfonsRGBA(81, 76, 76, 255);

	fonsClearState(fs);
	fonsSetSize(fs, textSize);
	fonsSetFont(fs, fontNormal);
	fonsSetColor(fs, white);
	fonsSetAlign(fs, FONS_ALIGN_CENTER | FONS_ALIGN_TOP);
	fonsVertMetrics(fs, NULL, NULL, &lh);

	for(int i=0; i < texts.size(); i++) {

		std::stringstream ss(texts[i]);
		std::string line;
		std::vector<std::string> lines;

		float maxWidth = 0;
		while(std::getline(ss, line, '\n')){
			float width = fonsTextBounds(fs, line.c_str(), NULL, NULL);
			if (width > maxWidth) {
				maxWidth = width;
			}
			lines.push_back(line);
		}

		sx = maxWidth + (1.0f*border);
		sy = border;
		if (lineCount > lines.size()) {
			sy += lh * (lineCount - lines.size()) * 0.5;
		}
		float height = lh*lineCount+2.0*border;


		std::shared_ptr<Texture> depthTexture = Texture::createEmpty("depthTex", sx, height, 1, 1, false, GL_TEXTURE_2D, GL_DEPTH_COMPONENT32F);
		depthTexture->setTexParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		depthTexture->setTexParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		depthTexture->setTexParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		depthTexture->setTexParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture->getID(), 0);

		textures[threadId][i] = Texture::createEmpty("colorTex", sx, height, 1, 4, false, GL_TEXTURE_2D, GL_RGBA8);
		textures[threadId][i]->setTexParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		textures[threadId][i]->setTexParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		textures[threadId][i]->setTexParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		textures[threadId][i]->setTexParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		textures[threadId][i]->setTexParameterf(GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0);
		glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures[threadId][i]->getID(), 0);
			
		GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		switch(status) {
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			assert(false);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			assert(false);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			assert(false);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			assert(false);
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			assert(false);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			assert(false);
			break;
		default:
			break;
		}
		assert(status == GL_FRAMEBUFFER_COMPLETE);

		sizes[threadId].push_back(glm::dvec2(sx, height));

		glViewport(0,0, sx, height);
		glClear(GL_COLOR_BUFFER_BIT);

		shader->setUniform("projection_mat", glm::ortho(0., (double)sx, (double)height, 0., -1., 1.));
		shader->setUniform("view_mat", glm::dmat4(1.0));
		shader->setUniform("model_mat", glm::dmat4(1.0));
		shader->setUniform("has_lambertian_texture", false);

		glDisable(GL_DEPTH_TEST);


		// Draw the darker interior quad so we get a white border around the outside from the clear color
		float rim = border/4.0f;

		GLfloat vertices[]  = {rim, height-rim,    sx-rim, height-rim,  rim, rim,  sx-rim, rim};
		GLfloat texCoords[] = { 0, 1,   1, 1,  1, 0,  0, 0 };
		GLfloat colors[]  = { 0.32, 0.3, 0.3, 1.0,   0.32, 0.3, 0.3, 1.0,   0.32, 0.3, 0.3, 1.0,   0.32, 0.3, 0.3, 1.0};

		// create the vao
		GLuint vaoID = 0;
		glGenVertexArrays(1, &vaoID);
		glBindVertexArray(vaoID);

		GLuint quadVBO = 0;
		glGenBuffers(1, &quadVBO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(texCoords)+sizeof(colors), 0, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);                             // copy vertices starting from 0 offest
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(texCoords), texCoords);                // copy texCoords after vertices
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(texCoords), sizeof(colors), colors);  // copy colours after normals

		// set up vertex attributes
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)sizeof(vertices));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(vertices)+sizeof(texCoords)));

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glBindVertexArray(0);
		glDeleteBuffers(1, &quadVBO);
		glDeleteVertexArrays(1, &vaoID);

		shader->setUniform("has_lambertian_texture", true);
		glActiveTexture(GL_TEXTURE0);
		shader->setUniform("lambertian_texture", 0);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for(int j=0; j < lines.size(); j++) { 
			std::string text = boost::replace_all_copy(lines[j], "_", " ");
			fonsDrawText(fs, sx/2.0, sy, text.c_str(), NULL);
			sy += lh;
		}

		glDisable(GL_BLEND);

		textures[threadId][i]->generateMipMaps();
		std::string textureKey = texKey + texts[i];
		texMan->setTextureEntry(threadId, textureKey, textures[threadId][i]);

		depthTexture.reset();
	}
}

void LikertHCI::update(const std::vector<MinVR::EventRef> &events)
{	
	//std::cout << "Begin likert update" << std::endl;
	//std::cout<<"currentQuestions: "<<_currentQuestion<<std::endl;

	for(int i=0; i < events.size(); i++) {

        if (boost::algorithm::starts_with(events[i]->getName(), "TUIO_Cursor_down")) {
			glm::dvec3 roomCoord = convertScreenToRoomCoordinates(events[i]->get2DData());
			//std::cout<< "User Touched at "<<glm::to_string(roomCoord)<<std::endl;
			for(int j=0; j < _answerBounds.size(); j++) {
				//std::cout<<"Checking Box "<<j<<" Low: "<<glm::to_string(_answerBounds[j].low())<<" High: "<<glm::to_string(_answerBounds[j].high())<<std::endl;
				if (_answerBounds[j].contains(roomCoord)) {
					_answerRecorder << _currentQuestion<<", "<<j<<std::endl; 
					
					std::cout<< _answers[j] <<std::endl;
					std::cout<<"Incrementing question"<<std::endl;
					_currentQuestion++;
					if (_currentQuestion > _questionRanges[_cycleCount].y) {
						if (_cycleCount < _questionRanges.size()-1) {
							_currentQuestion = _questionRanges[_cycleCount+1].x;
							_cycleCount++;
						}
                        done = true;
					}
					break;
				}
			}
		}
	}
}




glm::dvec3 LikertHCI::convertScreenToRoomCoordinates(glm::dvec2 screenCoords) {
	glm::dvec3 xVec = offAxisCamera->getTopRight() - offAxisCamera->getTopLeft();
	glm::dvec3 yVec = offAxisCamera->getBottomRight() - offAxisCamera->getTopRight();
	return offAxisCamera->getTopLeft() + (screenCoords.x * xVec) + (screenCoords.y * yVec);
}

void LikertHCI::draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window)
{
	_shader->use();
	MinVR::CameraOffAxis* offAxisCam = dynamic_cast<MinVR::CameraOffAxis*>(camera.get());

	_shader->setUniform("projection_mat", offAxisCam->getLastAppliedProjectionMatrix());
	_shader->setUniform("view_mat", offAxisCam->getLastAppliedViewMatrix());
	_shader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix());
	//_shader->setUniform("normal_matrix", (glm::dmat3(offAxisCamera->getLastAppliedModelMatrix())));
	
	_shader->setUniform("textureSampler",0);

    //question on the upper part of the screen
    glm::dvec3 normal(0.0, 1.0, 0.0);
    glm::dvec3 right(1.0, 0.0, 0.0);



    drawText(threadId, "QuestionText_", _questions, _questionSizes, _currentQuestion, offAxisCam, glm::dvec3(0.0, 0.0, -0.5), normal, right, _questionHeights[_currentQuestion]);

    glm::dvec3 start(offAxisCam->getBottomLeft().x + _padding + _individualSize/2.0, 0.0, 0.5);
    glm::dvec3 spacing(_individualSize, 0.0, 0.0);

	glm::ivec2 range = _answerRanges[_currentQuestion];
	int count = 0;
	for(int i=range.x; i <= range.y; i++) {
        drawText(threadId, "AnswerText_", _answers, _answerSizes, i, offAxisCam, start + (double)count * spacing, normal, right, _answerHeights[i]);
		count++;
    }
}

void LikertHCI::drawText(int threadId, std::string texKey, const std::vector<std::string> &texts, const std::vector<std::vector<glm::dvec2> > &sizes, int indexNum, MinVR::CameraOffAxis* offAxisCamera, glm::dvec3 centerPt, glm::dvec3 normal, glm::dvec3 right, double textHeight)
{
	glm::dvec3 up = glm::cross(normal, right);		

	glm::dvec2 size = sizes[threadId][indexNum];
	
	double halfWidth = textHeight * size.x / size.y / 2.0f;
	double halfHeight = textHeight / 2.0f;

	std::vector<GPUMesh::Vertex> cpuVertexArray;
	std::vector<int> cpuIndexArray;

	GPUMesh::Vertex vert;
	vert.position = centerPt + halfWidth*right + halfHeight*up;
	vert.normal = normal;
	vert.texCoord0 = glm::dvec2(1, 1);
	cpuVertexArray.push_back(vert);
	cpuIndexArray.push_back(cpuVertexArray.size()-1);
	vert.position = centerPt - halfWidth*right + halfHeight*up;
	vert.texCoord0 = glm::dvec2(0,1);
	cpuVertexArray.push_back(vert);
	cpuIndexArray.push_back(cpuVertexArray.size()-1);
	vert.position = centerPt + halfWidth*right - halfHeight*up;
	vert.texCoord0 = glm::dvec2(1,0);
	cpuVertexArray.push_back(vert);
	cpuIndexArray.push_back(cpuVertexArray.size()-1);
	vert.position = centerPt - halfWidth*right - halfHeight*up;
	vert.texCoord0 = glm::dvec2(0,0);
	cpuVertexArray.push_back(vert);
	cpuIndexArray.push_back(cpuVertexArray.size()-1);

	const int numVertices = cpuVertexArray.size();
	const int cpuVertexByteSize = sizeof(GPUMesh::Vertex) * numVertices;
	const int cpuIndexByteSize = sizeof(int) * cpuIndexArray.size();
		
	GPUMeshRef textVAO(new GPUMesh(GL_STREAM_DRAW, cpuVertexByteSize, cpuIndexByteSize, 0, cpuVertexArray, cpuIndexByteSize, &cpuIndexArray[0]));

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	texMan->getTexture(threadId, texKey+texts[indexNum])->bind(0);
	

	glBindVertexArray(textVAO->getVAOID()); // Bind our Vertex Array Object  
	glDrawArrays(GL_TRIANGLE_STRIP, 0, cpuIndexArray.size());
}
