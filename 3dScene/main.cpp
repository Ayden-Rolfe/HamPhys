#include "_3dSceneApp.h"

int main() {
	
	// allocation
	auto app = new _3dSceneApp();

	// initialise and loop
	app->run("AIE", 1280, 720, false);

	// deallocation
	delete app;

	return 0;
}