#ifndef CURRENTHCIMGR_H_
#define CURRENTHCIMGR_H_

#include "app/include/TestHCI.h"
#include "app/include/TuioHCI.h"
#include "app/include/NewYTransExperimentHCI.h"

typedef std::shared_ptr<class CurrentHCIMgr> CurrentHCIMgrRef;

class CurrentHCIMgr {
public:
	CurrentHCIMgr();
	virtual ~CurrentHCIMgr();
	std::shared_ptr<AbstractHCI> currentHCI;


private:

	
	
};
#endif /* CURRENTHCIMGR_H_ */
