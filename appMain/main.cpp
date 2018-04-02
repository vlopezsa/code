#include "Deferred.h"

int main(int argc, char **argv)
{
	Deferred renderer;
	SampleConfig config;
	config.windowDesc.width = 1280;
	config.windowDesc.height = 720;
	config.windowDesc.resizableWindow = true;
	config.windowDesc.title = "Simple Deferred";

	renderer.run(config, argc, argv);


	return 0;
}