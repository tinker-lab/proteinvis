/* 

Need to know how to switch the currentHCI with another HCI
1. Transition betw trials
2. Transition betw HCIs
3. Record data
4. 

*/

#include "app/include/ExperimentMgr.h"

const double ExperimentMgr::NEARENOUGH = 0.0656168;

ExperimentMgr::ExperimentMgr(CurrentHCIMgrRef currentHCIMgr, CFrameMgrRef mgr, MinVR::AbstractCameraRef camera, TextureMgrRef texMan, FeedbackRef feedback) { //might need ampersand
	
	this->cFrameMgr = mgr;
	this->currentHCIMgr = currentHCIMgr; // some new HCIs
	this->texMan = texMan;
	this->feedback = feedback;
	offAxisCamera = std::dynamic_pointer_cast<MinVR::CameraOffAxis>(camera);
	startTime = getCurrentTime();
	trialTimeLimit = MinVR::ConfigVal("TrialTimeLimit", 300000, false); 

	std::vector<std::string> expOrderString = MinVR::splitStringIntoArray(MinVR::ConfigVal("ExperimentOrder", "", false));
	for (int i = 0; i < expOrderString.size(); i++) {
		experimentOrder.push_back(MinVR::stringToInt(expOrderString[i]));
		//std::cout << experimentOrder[i] << std::endl;
	}

	boost::posix_time::time_facet* facet = new boost::posix_time::time_facet();
	facet->format("%Y-%m-%d.%H.%M.%S");
	std::stringstream stream;
	stream.imbue(std::locale(stream.getloc(), facet));
	stream << boost::posix_time::second_clock::local_time();
	std::string eventStreamFile = "Results-" + stream.str() + ".txt";
	_answerRecorder.open(eventStreamFile);
	if (experimentOrder[1] == HCI::NEWYTRANS) { // second element in ExperimentOrder in cfg
		_answerRecorder << "Experiment NEW" << std::endl;
	} else {
		_answerRecorder << "Experiment ORIG" << std::endl;
	}
	_answerRecorder <<"Time (ms), Error (ft), Failed Trial"<<std::endl;

	inZoneTime = MinVR::ConfigVal("InZoneTime", 1500, false);

	// increments upward along the experimentOrder vector
	experimentProgress = 0;
	trialCount = 0;
}	

ExperimentMgr::~ExperimentMgr() {
	if (_answerRecorder.is_open()) {
		std::flush(_answerRecorder);
		_answerRecorder.close();
	}
}

void ExperimentMgr::initializeContextSpecificVars(int threadId, MinVR::WindowRef window){

	
	
	//numberQuestions = MinVR::ConfigVal("NumberQuestions", 1, false); 
	showCompleteTrial = false;
	secondTimer = false;
	inPosition = false;
	// ExperimentMgr doesn't need texture manager, but tetrahedron does.
	tetra.reset(new Tetrahedron(window->getCamera(0), cFrameMgr, texMan, NEARENOUGH));
	tetra->initializeContextSpecificVars(threadId);

	//////////////////////////
	// Experiment Variables //
	//////////////////////////
	HCIExperiment = experimentOrder[experimentProgress]; // 0 - 6
	transformIndex = 1; // 1 - 5 but randomized
	numTrials = MinVR::ConfigVal("NumTrials", 5, false);
	numPracticeTrials = MinVR::ConfigVal("NumPracticeTrials", 3, false);


	// get translation transforms
	for(int i=0; i < numTrials; i++) {
		std::string key = "TransMat"+MinVR::intToString(i);
		glm::dmat4 transMat = MinVR::ConfigVal(key, glm::dmat4(0.0), false);
		transMats.push_back(transMat);
	}

	//// get rotation transforms
	for(int i=0; i < numTrials; i++) {
		std::string key = "RotMat"+MinVR::intToString(i);
		glm::dmat4 rotMat = MinVR::ConfigVal(key, glm::dmat4(0.0), false);
		rotMats.push_back(rotMat);
	}
	
	//// get combined transforms
	for(int i=0; i < numTrials; i++) {
		std::string key = "CombinedMat"+MinVR::intToString(i);
		glm::dmat4 combinedMat = MinVR::ConfigVal(key, glm::dmat4(0.0), false); 
		combinedMats.push_back(combinedMat);
	}
	
 // initialization depends on config file.
	double d = MinVR::ConfigVal("Test", 0.5, false);
	std::vector<std::string> strings = MinVR::splitStringIntoArray(MinVR::ConfigVal("TestMulti", "", false));
}

bool ExperimentMgr::finishedEverything() {
	if (experimentProgress == experimentOrder.size()) {
		return true;
	}
	return false;
}

// each trial is a tetra's orientation specified by a dmat4, in a config file
// and a restart in time
// and a new file to log output


// Does the following:
// switch HCI !!! App actually changes the HCI
// update trial number
// update experiment number
// point to next matrices we need for experiment
void ExperimentMgr::advance() {

	if (HCIExperiment != HCI::LIKERT && HCIExperiment != HCI::PROMPT) {
		trialCount++;
		Beep(235, 500);
		//std::cout << "\a"<<std::endl;
	}

	bool firstTrialAfterPractice = trialCount == numPracticeTrials;
	bool atEndOfTrials = trialCount >= numTrials;
	if (HCIExperiment == HCI::LIKERT || HCIExperiment == HCI::PROMPT || firstTrialAfterPractice || atEndOfTrials) {
		experimentProgress++;

		if (HCIExperiment == HCI::LIKERT) {
			_answerRecorder << std::endl;
		}

		// finished the study
		if (experimentProgress == experimentOrder.size()) {
			std::cout << "Finished :D" << std::endl;
			if (_answerRecorder.is_open()) {
				std::flush(_answerRecorder);
				_answerRecorder.close();
			}
			return;
		}

		currentHCIMgr->currentHCI->reset();
		feedback->displayText = "";
		HCIExperiment = experimentOrder[experimentProgress];
	}

	bool doingPractice = trialCount < numPracticeTrials && HCIExperiment != HCI::LIKERT && HCIExperiment != HCI::PROMPT;
	if (doingPractice){
		feedback->displayPractice = true;
	}
	else {
		feedback->displayPractice = false;
	}

	if (atEndOfTrials) {
		trialCount = 0;
	}

	std::cout << "trial count: " << trialCount << std::endl;
	std::cout << "experiment number: " << HCIExperiment << std::endl;

	trialStart = getCurrentTime();
}


void ExperimentMgr::userGivedUp(){
	std::cout<<"User just can't just Can't "<<std::endl;
	currentLengthOfTrial = (getDuration(getCurrentTime(), trialStart)).total_milliseconds();
	std::cout<<"Trial Time: "<<currentLengthOfTrial<<std::endl;
	_answerRecorder << currentLengthOfTrial << ", " << -1 << ", " << 1 << std::endl;
}


// assume Cframe manager has updated matrices
// since App calls currentHCI->update before this call
bool ExperimentMgr::checkFinish() {

	//Automatically advance to the next trial if they took too long
	currentLengthOfTrial = (getDuration(getCurrentTime(), trialStart)).total_milliseconds();
	//std::cout<<"Trial Time: "<<currentLengthOfTrial<<std::endl;
	if (HCIExperiment != HCI::PROMPT && HCIExperiment != HCI::LIKERT && currentLengthOfTrial > trialTimeLimit && trialCount >= numPracticeTrials) {
		_answerRecorder << currentLengthOfTrial << ", " << -1 << ", " << 1 << std::endl;
		return true;
	}

	glm::dmat4 currHCItransform = cFrameMgr->getVirtualToRoomSpaceFrame();

	if(HCIExperiment == HCI::PROMPT){
	
		PromptHCI* promp = dynamic_cast<PromptHCI*>((currentHCIMgr->currentHCI).get());
		if(promp->done){
			promp->done = false;
			return true;
		}
		else {
			return false;
		}
	
	}
	if (HCIExperiment == HCI::LIKERT) {
 
		LikertHCI* likert = dynamic_cast<LikertHCI*>((currentHCIMgr->currentHCI).get());
		if (likert->done) {
			likert->done = false;
			return true;
		}
		else {
			return false;
		}
	}
	else if (HCIExperiment == HCI::NEWYTRANS) {
		transform = transMats[trialCount];
	} else if (HCIExperiment == HCI::NEWXZROT) {
		transform = rotMats[trialCount];
	} else if (HCIExperiment == HCI::NEWANCHORED) {
		transform = combinedMats[trialCount];
	}
	else if (HCIExperiment == HCI::ORIGYTRANS) {
		transform = transMats[trialCount];
	} else if (HCIExperiment == HCI::ORIGXZROT) {
		transform = rotMats[trialCount];
	} else if (HCIExperiment == HCI::ORIGANCHORED) {
		transform = combinedMats[trialCount];
	}
	
	
	//std::cout<<"staticTransform: "<<glm::to_string(staticTransform)<<std::endl;
	
	/*std::cout << "Muh xforms: " << glm::to_string(transMats[0]) << std::endl;
	std::cout << "Muh xforms2: " << glm::to_string(transMats[1]) << std::endl;
	std::cout << "Muh xforms3: " << glm::to_string(transMats[2]) << std::endl;
	std::cout << "Muh xforms4: " << glm::to_string(transMats[3]) << std::endl;
	std::cout << "Muh xforms5: " << glm::to_string(transMats[4]) << std::endl;*/

	// points in Model space are identity transformed to World to Room
	glm::dvec3 transformableTetraPointA = glm::dvec3(currHCItransform * transform * glm::dvec4(tetra->pointA, 1.0));
	glm::dvec3 staticTetraPointA = tetra->pointA;

	glm::dvec3 transformableTetraPointB = glm::dvec3(currHCItransform * transform * glm::dvec4(tetra->pointB, 1.0));
	glm::dvec3 staticTetraPointB = tetra->pointB;

	glm::dvec3 transformableTetraPointC = glm::dvec3(currHCItransform * transform * glm::dvec4(tetra->pointC, 1.0));
	glm::dvec3 staticTetraPointC = tetra->pointC;

	glm::dvec3 transformableTetraPointD = glm::dvec3(currHCItransform * transform * glm::dvec4(tetra->pointD, 1.0));
	glm::dvec3 staticTetraPointD = tetra->pointD;

	//std::cout << "Static point: " << glm::to_string(staticTetraPointA) << std::endl;
	//std::cout << "Transformable point: " << glm::to_string(transformableTetraPointA) << std::endl;
	//std::cout << "Distance: " << glm::distance(transformableTetraPointA, staticTetraPointA) << std::endl;

	double aDist = glm::distance(transformableTetraPointA, staticTetraPointA);
	double bDist = glm::distance(transformableTetraPointB, staticTetraPointB);
	double cDist = glm::distance(transformableTetraPointC, staticTetraPointC);
	double dDist = glm::distance(transformableTetraPointD, staticTetraPointD);
	bool nearA = aDist < NEARENOUGH;
	bool nearB = bDist < NEARENOUGH;
	bool nearC = cDist < NEARENOUGH;
	bool nearD = dDist < NEARENOUGH;
	bool prevInPosition = inPosition;

	if (nearA && nearB && nearC && nearD){ //if in the correct posisition
		
		inPosition = true;
		showCompleteTrial = true;		
	}
	else{
	
		inPosition = false;
		secondTimer = false;
		showCompleteTrial = false;	
	}

	//std::cout<<"inPosition : "<<inPosition<<std::endl;
	//std::cout<<"prevInPosition : "<<prevInPosition<<std::endl;

	if(inPosition == true /*to guard the situation when go out of time stamp */ && inPosition != prevInPosition /*make sure only start the time stamp initially*/){
		startInZone = getCurrentTime();
		secondTimer = true;

		
	}

	

	if(secondTimer){
		
		t2 = getCurrentTime();
		totalTimeInZone = (getDuration(t2, startInZone)).total_milliseconds();
	
	}

	
	
	//std::cout<<"total time in zone :  " <<totalTimeInZone<<std::endl;

	if (nearA && nearB && nearC && nearD &&  totalTimeInZone > inZoneTime ) {
		
		// set to false for next trial
		showCompleteTrial = false;
		inPosition = false;
		secondTimer = false;
		trialEnd = getCurrentTime();

		double diff = (getDuration(trialEnd, trialStart)).total_milliseconds();
		double error = getError(aDist, bDist, cDist, dDist);
		_answerRecorder << diff << ", " << error << ", " << 0 << std::endl;

		return true;
	}
	
	return false;
}


double ExperimentMgr::getError(double A, double B, double C, double D){

	return (A+B+C+D)/4.0;
}


void ExperimentMgr::draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window) {
	// use transforms stored in the std::vector
	// apply them to whatever objects we're rendering, and draw them.

	if (HCIExperiment != HCI::LIKERT && HCIExperiment != HCI::PROMPT  && showCompleteTrial == false) {
	
		// draws both the static and the transformable tetrahedron
		tetra->draw(threadId, camera, window, "Koala2", transform, "red",  "green", "blue", "Koala", "forestGreen");
	} 
	else if(HCIExperiment != HCI::LIKERT && HCIExperiment != HCI::PROMPT && showCompleteTrial == true){
	
		tetra->draw(threadId, camera, window, "forestGreen", transform, "forestGreen",  "forestGreen", "forestGreen", "forestGreen", "forestGreen");
	} 
	else { // this is the likertHCI
		currentHCIMgr->currentHCI->draw(threadId, camera, window);
	}
}




