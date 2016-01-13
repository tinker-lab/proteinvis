#ifndef EXPERIMENTMGR_H_
#define EXPERIMENTMGR_H_

// what things do we need to include anyway?
#include "app/include/Tetrahedron.h"
#include "app/include/AbstractHCI.h"
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/glm.hpp>
#include "CFrameMgr.H"
#include "app/include/GLSLProgram.h"
#include <MVRCore/CameraOffAxis.H>
#include <MVRCore/DataFileUtils.H>
#include <glm/gtc/matrix_transform.hpp>
#include "app/include/TextureMgr.h"
#include "MVRCore/AbstractCamera.H"
#include "CFrameMgr.H"
#include "MVRCore/AbstractCamera.H"
#include "app/include/TextureMgr.h"
#include "app/include/TestHCI.h"
#include "app/include/Feedback.h"
#include "app/include/NewYTransExperimentHCI.h"
#include "app/include/CurrentHCIMgr.h"
#include "app/include/NewXZRotExperimentHCI.h"
#include "app/include/NewAnchoredExperimentHCI.h"
#include <MVRCore/ConfigVal.H>
#include <MVRCore/Time.h>
#include "app/include/LikertHCI.h"
#include "OrigAnchoredHCI.h"
#include "OrigXZRotExperimentHCI.h"
#include "OrigYTransExperimentHCI.h"
#include "app/include/PromptHCI.h"

typedef std::shared_ptr<class ExperimentMgr> ExperimentMgrRef;

class ExperimentMgr {
public:

	static const double NEARENOUGH;
	ExperimentMgr(CurrentHCIMgrRef currentHCIMgr, CFrameMgrRef cFrameMgr, MinVR::AbstractCameraRef camera, TextureMgrRef texMan, FeedbackRef feedback); //maybe need ampersand
	virtual ~ExperimentMgr();
	void advance (); // change function signature and call
	void initializeContextSpecificVars(int threadId, MinVR::WindowRef window);
	bool checkFinish();
	void draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window);
	double getError(double A, double B, double C, double D);
	MinVR::TimeStamp trialStart;
	MinVR::TimeStamp trialEnd;
	int HCIExperiment; // for old advance()
	void userGivedUp();
	//int getExperimentNumber();
	bool finishedEverything();

	enum HCI {
		PROMPT			=-1,
		LIKERT			= 0,
		NEWYTRANS		= 1,
		NEWXZROT		= 2,
		NEWANCHORED		= 3,
		ORIGYTRANS		= 4,
		ORIGXZROT		= 5,
		ORIGANCHORED	= 6
	};

private:
	void initGL();
	CurrentHCIMgrRef currentHCIMgr;
	TetrahedronRef tetra;
	int transformIndex;
	std::vector<glm::dmat4> transforms;
	TextureMgrRef texMan;
	FeedbackRef feedback;

	int trialCount; // 1 to 8 // NEED THIS
	bool inPosition; // for checkFinish()
	bool secondTimer; // for checkFinish()
	bool showCompleteTrial;  // for checkFinish()
	
	// for advance()
	int trialSet;
	int likertCount;
	int numTrials;
	int numPracticeTrials;

	// for advance()'s revisions
	std::vector<int> experimentOrder;
	int experimentProgress;

	MinVR::TimeStamp startInZone;
	double totalTimeInZone;
	MinVR::TimeStamp startTime;
	MinVR::TimeStamp t2;
	glm::dmat4 transform;
	CFrameMgrRef cFrameMgr;
	std::shared_ptr<MinVR::CameraOffAxis> offAxisCamera;
	std::vector<glm::dmat4> transMats;
	std::vector<glm::dmat4> rotMats;
	std::vector<glm::dmat4> combinedMats;
	
	double Error;
	std::ofstream _answerRecorder;
	int trialTimeLimit;
	double currentLengthOfTrial;
	double inZoneTime;

};

#endif /* EXPERIMENTMGR_H_ */
