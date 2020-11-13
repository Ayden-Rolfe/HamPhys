#include "HamEngineApp.h"

// Enable leak detection
// #define DEBUG_LEAKDETECT

#ifdef DEBUG_LEAKDETECT

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#endif // Leak Detection


int main() 
{
#ifdef DEBUG_LEAKDETECT
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // Leak Detection

	// allocation
	auto app = new HamEngineApp();

	// initialise and loop
	app->run("AIE", WINDOW_WIDTH, WINDOW_HEIGHT, false);

	// deallocation
	delete app;

	return 0;
}