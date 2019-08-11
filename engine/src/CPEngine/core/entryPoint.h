#pragma once
#include "core.h"
#include "CPEngine/application.h"
#include "CPEngine/core/logging.h"

extern cp::Application* cp::createApplication();

int main(int , char** )
{
	cp::core::coreStartUp();
	auto* app= cp::createApplication();
	app->run();
	delete app;

	cp::core::coreShutDown();
	return 0;
}
