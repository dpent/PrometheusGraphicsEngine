#include "guiManager.h"
#include "../core/headers/engine.h"
#include "../core/headers/bufferManager.h"

size_t GUIManager::currentTextureViewIndex = 0;
std::vector<VkDescriptorSet> GUIManager::textureDisplayIds;
std::vector<std::string> GUIManager::textureDisplayNames;

ImGuiDockNode* GUIManager::mainDockspace = nullptr;

void GUIManager::initImGUI() {

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui_ImplGlfw_InitForVulkan(Engine::window, true);

	GUIManager::createGuiDescriptorPool();

	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = Engine::vkInstanceInfo.instance;
	initInfo.PhysicalDevice = Engine::deviceInfo.physicalDevice;
	initInfo.Device = Engine::deviceInfo.logicalDevice;
	initInfo.QueueFamily = Engine::queues.graphicsIndex;
	initInfo.Queue = Engine::queues.graphics;
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.DescriptorPool = Engine::imGuiDescriptor.pool;
	initInfo.MinImageCount = 2;
	initInfo.ImageCount = static_cast<uint32_t>(Engine::swapChainInfo.images.size());
	initInfo.Allocator = nullptr;
	initInfo.PipelineInfoMain.RenderPass = Engine::graphicsRenderPass;
	initInfo.PipelineInfoMain.Subpass = 0;
	initInfo.PipelineInfoMain.MSAASamples = Engine::msaaSamples;
	initInfo.CheckVkResultFn = check_vk_result;

	ImGui_ImplVulkan_Init(&initInfo);

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = Engine::IMGUI_BACKGROUND_COLOR; // window background
	style.Colors[ImGuiCol_TitleBg] = Engine::IMGUI_ACTIVE_COLOR; // title bar
	style.Colors[ImGuiCol_TitleBgActive] = Engine::IMGUI_HIGHLIGHT_COLOR; // active title bar
	style.Colors[ImGuiCol_Button] = Engine::IMGUI_BACKGROUND_COLOR; // button
	style.Colors[ImGuiCol_ButtonHovered] = Engine::IMGUI_ACTIVE_COLOR;
	style.Colors[ImGuiCol_ButtonActive] = Engine::IMGUI_HIGHLIGHT_COLOR;
	style.Colors[ImGuiCol_ResizeGrip] = Engine::IMGUI_DARK_COLOR; // resize grib
	style.Colors[ImGuiCol_ResizeGripHovered] = Engine::IMGUI_ACTIVE_COLOR;
	style.Colors[ImGuiCol_ResizeGripActive] = Engine::IMGUI_HIGHLIGHT_COLOR;
	style.Colors[ImGuiCol_ScrollbarBg].w = 0.2f;
	style.Colors[ImGuiCol_ScrollbarGrab].w = 0.5f;
	style.Colors[ImGuiCol_ScrollbarGrabHovered].w = 0.7f;
	style.Colors[ImGuiCol_ScrollbarGrabActive].w = 0.9f;
	style.Colors[ImGuiCol_SliderGrab] = Engine::IMGUI_HIGHLIGHT_COLOR;
	style.Colors[ImGuiCol_SliderGrabActive] = Engine::IMGUI_HIGHLIGHT_COLOR;
	style.Colors[ImGuiCol_FrameBg] = Engine::IMGUI_DARK_COLOR;
	style.Colors[ImGuiCol_FrameBgHovered] = Engine::IMGUI_ACTIVE_COLOR;
	style.Colors[ImGuiCol_FrameBgActive] = Engine::IMGUI_ACTIVE_COLOR;
	style.Colors[ImGuiCol_Tab] = Engine::IMGUI_DARK_COLOR; // inactive tab
	style.Colors[ImGuiCol_TabHovered] = Engine::IMGUI_HIGHLIGHT_COLOR; // hover
	style.Colors[ImGuiCol_TabActive] = Engine::IMGUI_BACKGROUND_COLOR; // active tab
	style.Colors[ImGuiCol_TabUnfocused] = Engine::IMGUI_ACTIVE_COLOR; // inactive when window unfocused
	style.Colors[ImGuiCol_TabUnfocusedActive] = Engine::IMGUI_HIGHLIGHT_COLOR;
	style.Colors[ImGuiCol_DockingPreview] = Engine::IMGUI_HIGHLIGHT_COLOR;
	style.Colors[ImGuiCol_HeaderHovered] = Engine::IMGUI_HIGHLIGHT_COLOR;
	style.Colors[ImGuiCol_HeaderActive] = Engine::IMGUI_ACTIVE_COLOR;
	style.Colors[ImGuiCol_Header] = Engine::IMGUI_BACKGROUND_COLOR;
	style.Colors[ImGuiCol_PopupBg] = Engine::IMGUI_BACKGROUND_COLOR;
	style.Colors[ImGuiCol_Border] = Engine::IMGUI_ACTIVE_COLOR;
}

void GUIManager::check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

void GUIManager::createGuiDescriptorPool(){
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE },
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 0;
	for (VkDescriptorPoolSize& pool_size : pool_sizes)
		pool_info.maxSets += pool_size.descriptorCount;
	pool_info.poolSizeCount = (uint32_t)IM_COUNTOF(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	VkResult err = vkCreateDescriptorPool(Engine::deviceInfo.logicalDevice, &pool_info, nullptr, &Engine::imGuiDescriptor.pool);
	check_vk_result(err);
}

void GUIManager::startNewFrame() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplVulkan_NewFrame();
	ImGui::NewFrame();
}

void GUIManager::renderGUI(uint32_t& imageIndex) {

	GUIManager::createMainDockspace();
	GUIManager::createDockSpaceWindow();
	GUIManager::createInfoWindow();
	GUIManager::createCameraInfoWindow();
	GUIManager::createBufferInfoWindow();

	if (GUIManager::textureDisplayIds.size() != 0) {
		GUIManager::createImageWindow();
	}

	GUIManager::calculateViewportLimitations();

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Engine::command.buffers[Engine::currentFrame]);
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
}

void GUIManager::createInfoWindow() {

	ImGui::Begin("General Info");

	ImGui::SeparatorText("Performance");
	ImGui::Text("FPS: %u", Engine::FPS);
	ImGui::SliderInt("FPS cap", &Engine::targetFPS, 1, 1000);

	ImGui::SeparatorText("GPU");
	ImGui::Text("Selected GPU: %s", Engine::deviceInfo.physicalProperties.deviceName);
	ImGui::Text("Driver version: %u", Engine::deviceInfo.physicalProperties.driverVersion);
	ImGui::Text("Device Type: %s", DeviceManager::deviceTypeToString(Engine::deviceInfo.physicalProperties.deviceType));
	ImGui::Text("Vendor ID: %u", Engine::deviceInfo.physicalProperties.vendorID);
	ImGui::Text("Device ID: %u", Engine::deviceInfo.physicalProperties.deviceID);
	ImGui::Text("MSAA: x%d", Engine::msaaSamples);

	ImGui::SeparatorText("THREADS");
	ImGui::Text("Worker Threads: %zu", Engine::threadPool.size());
	ImGui::Text("Jobs in Queue: %zu", Engine::jobQueue.size());

	ImGui::End();

}

void GUIManager::createCameraInfoWindow() {

	ImGui::Begin("Camera");

	ImGui::SliderFloat("Max speed", &Engine::camera.maxSpeed, 0.0f, 200.0f);
	ImGui::SliderFloat("FOV", &Engine::camera.fov, 0.0f, 180.0f);
	ImGui::Text("Position: X: %.2f Y: %.2f Z: %.2f", Engine::camera.position.x, Engine::camera.position.y, Engine::camera.position.z);
	ImGui::Text("Front: X: %.2f Y: %.2f Z: %.2f", Engine::camera.front.x, Engine::camera.front.y, Engine::camera.front.z);
	ImGui::Text("Yaw: %.2f Pitch: %.2f", Engine::camera.yaw, Engine::camera.pitch);
	ImGui::SliderFloat("Far", &Engine::camera.far, 0.0f, 5000.0f);
	ImGui::SliderFloat("Acceleration", &Engine::camera.acceleration, 0.0f, 10.0f);
	
	ImGui::End();
}
	
void GUIManager::createDockSpaceWindow() {

	ImGui::Begin("Debug");
	ImGuiID dockspaceId = ImGui::GetID("Debug");
	ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f));
	ImGui::End();

	if (Engine::firstFrame)
	{
		Engine::firstFrame = false;

		ImGui::DockBuilderRemoveNode(dockspaceId); // clear any previous layout
		ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetIO().DisplaySize);

		ImGuiID dock_id_top;
		ImGuiID dock_id_mid;
		ImGuiID dock_id_bottom;

		ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Up, 0.5f, &dock_id_top, &dock_id_mid);

		ImGui::DockBuilderSplitNode(dock_id_mid, ImGuiDir_Up, 0.25f, &dock_id_mid, &dock_id_bottom);

		ImGui::DockBuilderDockWindow("General Info", dock_id_top);
		ImGui::DockBuilderDockWindow("Buffers", dock_id_mid);

		ImGui::DockBuilderDockWindow("Camera", dock_id_bottom);

		if (!GUIManager::textureDisplayIds.empty())
			ImGui::DockBuilderDockWindow("Texture view", dock_id_bottom);

		ImGui::DockBuilderFinish(dockspaceId);
	}
}

void GUIManager::createBufferInfoWindow() {
	ImGui::Begin("Buffers");

	ImGui::SeparatorText("VERTEX-INDEX BUFFER");
	ImGui::Text((std::string("Size: ") +Buffer::getSizeHumanReadable(Engine::vertexIndexBuffer.size)).c_str());
	GUIManager::makeBufferDiagram(Engine::vertexIndexHistory, "###VertexIndexHistory");

	ImGui::SeparatorText("STAGING BUFFER");
	ImGui::Text((std::string("Size: ") +Buffer::getSizeHumanReadable(Engine::stagingBuffer.size)).c_str());
	GUIManager::makeBufferDiagram(Engine::stagingHistory, "###StagingHistory");

	ImGui::SeparatorText("INSTANCE DATA SSBO");
	ImGui::Text((std::string("Size: ") +Buffer::getSizeHumanReadable(Engine::instanceDataSSBO.size)).c_str());
	GUIManager::makeBufferDiagram(Engine::instanceDataHistory, "###InstanceDataHistory");
	
	ImGui::SeparatorText("DEBUG LINES SSBO");
	ImGui::Text((std::string("Size: ") + Buffer::getSizeHumanReadable(Debug::lineSSBO.size)).c_str());
	GUIManager::makeBufferDiagram(Engine::instanceDataHistory, "###DebugDataHistory");

	ImGui::End();
}

void GUIManager::createImageWindow() {

	ImGui::Begin("Texture view");
	
	std::string preview = GUIManager::textureDisplayNames[GUIManager::currentTextureViewIndex];
	
	ImGui::SetNextItemWidth(-1);
	
	if (ImGui::BeginCombo("",preview.c_str()))
	{
		for (int i = 0; i < GUIManager::textureDisplayIds.size(); ++i)
		{
			const std::string& label = GUIManager::textureDisplayNames[i];

			bool isSelected = (GUIManager::currentTextureViewIndex == i);
			if (ImGui::Selectable(label.c_str(), isSelected))
				GUIManager::currentTextureViewIndex = i;

			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImVec2 avail = ImGui::GetContentRegionAvail();
	ImGui::Image(GUIManager::textureDisplayIds[GUIManager::currentTextureViewIndex], avail);
	
	ImGui::End();
}

void GUIManager::makeBufferDiagram(std::vector<float>& history, const char* label) {

	ImVec2 avail = ImGui::GetContentRegionAvail();
	float max = Engine::getMax(history);
	ImGui::PlotLines(
		label,
		history.data(),
		static_cast<int>(history.size()),
		0,                // offset
		"(MB)",          // overlay text (optional)
		0.0f,             // scale_min
		max,           // scale_max
		ImVec2(avail.x, 50)     // size of the graph
	);
}

void GUIManager::createMainDockspace() {

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	ImGui::Begin("##RenderDockHost", nullptr, window_flags);

	ImGuiID dockspaceId = ImGui::GetID("RenderDockSpace");
	ImGui::DockSpace(
		dockspaceId,
		ImVec2(0, 0),
		ImGuiDockNodeFlags_PassthruCentralNode
	);

	ImGui::End();

	ImGui::PopStyleVar();


	if (Engine::firstFrame)
	{

		ImGui::DockBuilderRemoveNode(dockspaceId);
		ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetIO().DisplaySize);

		GUIManager::mainDockspace = getDockNode(dockspaceId);

		ImGuiID dock_id_left;
		ImGuiID dock_id_main;
		ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, 0.25f, &dock_id_left, &dock_id_main);

		ImGui::DockBuilderDockWindow("Debug", dock_id_left);

		ImGui::DockBuilderFinish(dockspaceId);
	}

}

void GUIManager::calculateViewportLimitations() {

	std::vector<DockedWindowInfo> infos = GUIManager::getDockedWindowInfo(GUIManager::mainDockspace);

	Engine::viewportLimitsOffsets = glm::vec4(0.0f);
	ImGuiViewport* vp = ImGui::GetMainViewport();

	for (auto info : infos) {

		info.limitUpdater->updateLimit(info, vp);

		delete info.limitUpdater;
	}

}

ImGuiDockNode* GUIManager::getDockNode(ImGuiID id)
{
	ImGuiContext* ctx = ImGui::GetCurrentContext();
	ImGuiDockContext& dock_ctx = ctx->DockContext;

	for (int i = 0; i < dock_ctx.Nodes.Data.Size; i++)
	{
		ImGuiStoragePair& p = dock_ctx.Nodes.Data[i];
		if (p.key == id)
			return (ImGuiDockNode*)p.val_p;
	}

	return nullptr;
}

std::vector<DockedWindowInfo> GUIManager::getDockedWindowInfo(ImGuiDockNode* dockNode)
{
	std::vector<DockedWindowInfo> children;

	// Root has 0..2 children (Binary tree so thats how they do up/down - left/right)
	for (int i = 0; i < 2; i++)
	{
		ImGuiDockNode* child = dockNode->ChildNodes[i];
		if (!child) continue;

		if (child->Windows.Size == 0) continue;

		DockedWindowInfo info;
		info.node = child;
		info.pos = child->Pos;
		info.size = child->Size;
		info.name = child->Windows[0]->Name;
		info.limitUpdater = getLimitUpdater(child);

		children.push_back(info);
	}

	return children;
}

limitUpdater* GUIManager::getLimitUpdater(ImGuiDockNode* window) {

	ImGuiDockNode* parent = window->ParentNode;

	//if (!parent)
		//return "Center";

	bool is_first = (parent->ChildNodes[0] == window);

	if (parent->SplitAxis == ImGuiAxis_X) {

		if (is_first) {
			return new leftLimitUpdater();
		}
		
		return new rightLimitUpdater();
	}


	if (parent->SplitAxis == ImGuiAxis_Y) {

		if (is_first) {
			return new topLimitUpdater();
		}

		return new bottomLimitUpdater();
	}

	return nullptr;
}

void limitUpdater::updateLimit(DockedWindowInfo& info, ImGuiViewport* vp) {
	return;
}

void leftLimitUpdater::updateLimit(DockedWindowInfo& info, ImGuiViewport* vp) {

	float right = (info.pos.x - vp->Pos.x) + info.size.x;

	Engine::viewportLimitsOffsets.x =
		std::max(Engine::viewportLimitsOffsets.x, right);
}

void rightLimitUpdater::updateLimit(DockedWindowInfo& info, ImGuiViewport* vp) {

	float left = info.pos.x - vp->Pos.x;

	Engine::viewportLimitsOffsets.z =
		std::max(Engine::viewportLimitsOffsets.z,
			Engine::swapChainInfo.extent.width - left);
}

void topLimitUpdater::updateLimit(DockedWindowInfo& info, ImGuiViewport* vp) {

	float bottom = (info.pos.y - vp->Pos.y) + info.size.y;

	Engine::viewportLimitsOffsets.y =
		std::max(Engine::viewportLimitsOffsets.y, bottom);
}

void bottomLimitUpdater::updateLimit(DockedWindowInfo& info, ImGuiViewport* vp) {

	float top = info.pos.y - vp->Pos.y;

	Engine::viewportLimitsOffsets.w =
		std::max(Engine::viewportLimitsOffsets.w,
			Engine::swapChainInfo.extent.height - top);
}