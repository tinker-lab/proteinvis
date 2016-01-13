#ifndef ABSTRACTHCI_H_
#define ABSTRACTHCI_H_

#include <memory>
#include <glm/glm.hpp>
#include "CFrameMgr.H"
#include <vector>
#include "MVRCore/Event.H"
#include "MVRCore/AbstractCamera.h"
#include "app/include/Feedback.h"


typedef std::shared_ptr<class AbstractHCI> AbstractHCIRef;

class AbstractHCI
{
public:
	AbstractHCI(CFrameMgrRef cFrameMgr, FeedbackRef feedback);
	virtual ~AbstractHCI();
	virtual void update(const std::vector<MinVR::EventRef> &events) = 0;
	virtual void draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window);
	virtual void initializeContextSpecificVars(int threadId,MinVR::WindowRef window) = 0;
	virtual void initVBO(int threadId);
	virtual void initGL();
	virtual int getNumberTouches();
	FeedbackRef feedback;
	virtual void reset();


protected:
	CFrameMgrRef cFrameMgr;
	
};


#endif /* ABSTRACTHCI_H_ */