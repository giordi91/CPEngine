#pragma once
#include "CPEngine/application.h"
#include "CPEngine/core/logging.h"

extern cp::Application* cp::createApplication();

int main(int argc, char** argv)
{
	auto* app= cp::createApplication();

	cp::initializeLogging();
	cp::logCoreInfo("Initializing engine");
	app->run();


	cp::cleanUpLogging();
	delete app;
	return 0;
}
