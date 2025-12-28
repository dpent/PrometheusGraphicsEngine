#pragma once
#include "../headers/Prometheus.h"


class Engine {

public:
	static void run();
	static void initWindow();
	static void initVulkan();
	static void mainLoop();

};