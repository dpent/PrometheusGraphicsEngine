#include "guiManager.h"
#include "../core/headers/engine.h"

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

	GUIManager::createDockSpaceWindow();
	GUIManager::createInfoWindow();
	GUIManager::createCameraInfoWindow();

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Engine::command.buffers[Engine::currentFrame]);
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
}

void GUIManager::createInfoWindow() {

	ImGui::Begin("Info");

	ImGui::SeparatorText("GPU INFO");
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

		// Split the dockspace vertically: top 70% for Info, bottom 30% for Camera Info
		ImGuiID dock_id_main = dockspaceId;
		ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Down, 0.3f, nullptr, &dock_id_main);

		// Dock your windows
		ImGui::DockBuilderDockWindow("Info", dock_id_main);
		ImGui::DockBuilderDockWindow("Camera", dock_id_bottom);

		ImGui::DockBuilderFinish(dockspaceId);
	}
}