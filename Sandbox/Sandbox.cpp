#include "../Engine/Engine.h"

#define PROJECT_NAME  "Genesis Engine - Sandbox"

int main()
{
	gns::Engine engine;
	engine.InitEngine(PROJECT_NAME);
	engine.Run();
	engine.ShutDown();
}
