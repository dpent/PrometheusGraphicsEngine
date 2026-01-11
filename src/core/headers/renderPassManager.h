#pragma once

#include "Prometheus.h"

class Engine;

class RenderPassManager {
public:
	static void createRenderPass();
	static void createShadowRenderPass();
};