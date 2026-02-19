#pragma once

#include "../core/headers/Prometheus.h"

struct DockedWindowInfo;

struct limitUpdater {
public:
	virtual ~limitUpdater() = default;
	virtual void updateLimit(DockedWindowInfo& info, ImGuiViewport* vp);
};


struct leftLimitUpdater : limitUpdater{
public:

	void updateLimit(DockedWindowInfo& info, ImGuiViewport* vp) override;
};

struct rightLimitUpdater : limitUpdater {
public:

	void updateLimit(DockedWindowInfo& info, ImGuiViewport* vp) override;
};

struct topLimitUpdater : limitUpdater {
public:

	void updateLimit(DockedWindowInfo& info, ImGuiViewport* vp) override;
};

struct bottomLimitUpdater : limitUpdater {
public:

	void updateLimit(DockedWindowInfo& info, ImGuiViewport* vp) override;
};


struct DockedWindowInfo
{
	const char* name;
	ImGuiDockNode* node;
	ImVec2 pos;
	ImVec2 size;
	limitUpdater* limitUpdater;
};

class GUIManager {

public:

	static size_t currentTextureViewIndex;
	static std::vector<VkDescriptorSet> textureDisplayIds;
	static std::vector<std::string> textureDisplayNames;

	static ImGuiDockNode* mainDockspace;

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

	static void calculateViewportLimitations();
	static ImGuiDockNode* getDockNode(ImGuiID id);
	static void getDockedWindowInfo(ImGuiDockNode* dockNode, std::vector<DockedWindowInfo>& children);
	static limitUpdater* getLimitUpdater(ImGuiDockNode* window);
};