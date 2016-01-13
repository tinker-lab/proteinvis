#ifndef TOUCH_H_
#define TOUCH_H_

#include <glm/glm.hpp>

class Touch {
public:
	Touch(int tuio_id, glm::dvec3 touchPosition,long touchTimeDown);
	virtual ~Touch();

	int getTouchId();
	glm::dvec3 getTouchPosition(); //potentially want to store in room space so use dvec3
	long getTouchTimeDown(); //timedown unit in milisecond
	long setTouchPosition();

private:
	glm::dvec3 touchPosition;
	int touchId;
	long touchTimeDown;
};

#endif /* TOUCH_H_ */