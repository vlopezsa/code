#include "Deferred.h"

int main(int argc, char **argv)
{
	Deferred::UniquePtr pRenderer = std::make_unique<Deferred>();
	SampleConfig config;
	config.windowDesc.width = 1280;
	config.windowDesc.height = 720;
	config.windowDesc.resizableWindow = true;
	config.windowDesc.title = "Simple Deferred";

	config.argc = (uint32_t)argc;
	config.argv = argv;
	Sample::run(config, pRenderer);


	return 0;
}