#include <app/include/Touch.h>

using namespace MinVR;

glm::dvec3 touchPosition;
int touchId;
long touchTimeDown;

Touch::Touch(int tuio_id, glm::dvec3 touchPosition,long touchTimeDown){
	touchPosition = touchPosition;
	touchId = tuio_id;
	touchTimeDown = touchTimeDown;
}

Touch::~Touch(){

}

int Touch::getTouchId(){
	return touchId;
}

glm::dvec3 getTouchPosition(){
	return touchPosition;
}

long getTouchTimeDown(){
	return touchTimeDown;
}

long setTouchPosition(){
	//more to code here, wait until get OpenGL reponding to touch
}