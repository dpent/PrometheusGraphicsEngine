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
	style.Colors[ImGuiCol_WindowBg] = Engine::IMGUI_BACKGTOUND_COLOR; // window background
	style.Colors[ImGuiCol_TitleBg] = Engine::IMGUI_ACTIVE_COLOR; // title bar
	style.Colors[ImGuiCol_TitleBgActive] = Engine::IMGUI_HIGHLIGHT_COLOR; // active title bar
	style.Colors[ImGuiCol_Button] = Engine::IMGUI_BACKGTOUND_COLOR; // button
	style.Colors[ImGuiCol_ButtonHovered] = Engine::IMGUI_ACTIVE_COLOR;
	style.Colors[ImGuiCol_ButtonActive] = Engine::IMGUI_HIGHLIGHT_COLOR;
	style.Colors[ImGuiCol_ResizeGrip] = Engine::IMGUI_DARK_COLOR; // resize grib
	style.Colors[ImGuiCol_ResizeGripHovered] = Engine::IMGUI_ACTIVE_COLOR;
	style.Colors[ImGuiCol_ResizeGripActive] = Engine::IMGUI_HIGHLIGHT_COLOR;
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

	GUIManager::createInfoWindow();

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