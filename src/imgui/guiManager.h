#pragma once

#include "../core/headers/Prometheus.h"


class GUIManager {

public:

	static size_t currentTextureViewIndex;
	static std::vector<VkDescriptorSet> textureDisplayIds;
	static std::vector<std::string> textureDisplayNames;

	static void initImGUI();

	static void check_vk_result(VkResult err);

	static void createGuiDescriptorPool();

	static void startNewFrame();
	static void renderGUI(uint32_t& imageIndex);

	static void createMainDockspace();
	static void createInfoWindow();
	static void createCameraInfoWindow();
	static void createDockSpaceWindow();
	static void createBufferInfoWindow();
	static void makeBufferDiagram(std::vector<float>& history, const char* label);
	static void createImageWindow();
};