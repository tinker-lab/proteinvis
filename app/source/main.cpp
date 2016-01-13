#include "AppKit_GLFW/MVREngineGLFW.H"
#include "MVRCore/DataFileUtils.H"
#include <iostream>
#include "app/include/App.h"

int main(int argc, char** argv)
{
	MinVR::DataFileUtils::addFileSearchPath("$(MinVR_DIR)/share/vrsetup");
	MinVR::DataFileUtils::addFileSearchPath("$(MinVR_DIR)/share/shaders");
	MinVR::DataFileUtils::addFileSearchPath("C:/Users/Jenweil/Desktop/AnchoredGestures/app/shaders");
	MinVR::DataFileUtils::addFileSearchPath("C:/vis/sw/user-checkouts/bjackson/src/tinker-lab/AnchoredGestures/app/shaders");

	FreeImage_Initialise();

	MinVR::AbstractMVREngine *engine = new MinVR::MVREngineGLFW();
	engine->init(argc, argv);
	MinVR::AbstractMVRAppRef app(new App());
	engine->runApp(app);

	FreeImage_DeInitialise();
	delete engine;

	return 0;
}
