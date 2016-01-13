#ifndef LIKERTHCI_H_
#define LIKERTHCI_H_

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
#include "AABox.h"


typedef std::shared_ptr<class LikertHCI> LikertHCIRef;

class LikertHCI : public AbstractHCI {

public:
	LikertHCI(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr, TextureMgrRef textMan, FeedbackRef feedback);
	virtual ~LikertHCI();
	void update(const std::vector<MinVR::EventRef> &events);
    void draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window);
	void initializeContextSpecificVars(int threadId, MinVR::WindowRef window);
	bool done;

private:
	void initializeText(int threadId, MinVR::WindowRef window);
	void initializeText(int threadId, struct FONScontext* fs, int fontNormal,  int lineCount, float border, std::shared_ptr<GLSLProgram> shader, float textSize, const std::vector<std::string> &texts, std::string texKey, std::vector<std::vector<std::shared_ptr<Texture> > > &textures, std::vector<std::vector<glm::dvec2> > &sizes);
	void drawText(int threadId, std::string texKey, const std::vector<std::string> &texts, const std::vector<std::vector<glm::dvec2> > &sizes, int indexNum, MinVR::CameraOffAxis* offAxisCamera, glm::dvec3 centerPt, glm::dvec3 normal, glm::dvec3 right, double textHeight);
	glm::dvec3 convertScreenToRoomCoordinates(glm::dvec2 screenCoords);

	std::shared_ptr<MinVR::CameraOffAxis> offAxisCamera;
	TextureMgrRef texMan;
	std::shared_ptr<GLSLProgram> _shader;
	int _currentQuestion;
	std::vector<std::string> _questions;
	std::vector<std::string> _answers;

	std::vector<std::vector<std::shared_ptr<Texture> > > _questionTextures;
	std::vector<std::vector<std::shared_ptr<Texture> > > _answerTextures;
	std::vector<std::vector<glm::dvec2> > _questionSizes;
	std::vector<std::vector<glm::dvec2> > _answerSizes;
	std::vector<AABox> _answerBounds;
	std::vector<double> _answerHeights;
	std::vector<double> _questionHeights;


	double _padding;
	double _availableWidth;
	double _individualSize;
	std::ofstream _answerRecorder;
	std::vector<glm::ivec2> _answerRanges;
	std::vector<glm::ivec2> _questionRanges;
	int _maxNumLinesInAnswers;
	int _maxNumLinesInQuestions;

	int _cycleCount; //Keeps track of the number of times that likerthci has been current;
};

#endif
