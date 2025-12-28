// Prometheus.cpp : Defines the entry point for the application.
//

#include "../headers/Prometheus.h"
#include "../headers/engine.h"

using namespace std;

int main()
{
	Engine* engine = new Engine();

	Engine::run(engine);

	delete engine;
	return 0;
}
