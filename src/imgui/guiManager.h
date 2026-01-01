#pragma once

#include "../core/headers/Prometheus.h"


class GUIManager {

public:

	static void initImGUI();

	static void check_vk_result(VkResult err);

	static void createGuiDescriptorPool();

	static void startNewFrame();
	static void renderGUI(uint32_t& imageIndex);
};