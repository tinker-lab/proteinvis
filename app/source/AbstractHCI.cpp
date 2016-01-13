#include "app/include/AbstractHCI.h"

AbstractHCI::AbstractHCI(CFrameMgrRef mgr, FeedbackRef feedback) {
	this->cFrameMgr = mgr;
	this->feedback = feedback;
}

AbstractHCI::~AbstractHCI(){

}

void AbstractHCI::draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window){

}


void AbstractHCI::initVBO(int threadId){

}


void AbstractHCI::initGL(){

}

int AbstractHCI::getNumberTouches(){

	return 0;
}

void AbstractHCI::reset()
{
}