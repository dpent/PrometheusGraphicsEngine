#include "windowManager.h"
#include "../engine/headers/engine.h"
#include "imgui.h"

using namespace Prometheus;

namespace Prometheus{

    void WindowManager::initGUI(VkInstance& instance, VkQueue& graphicsQueue,
        VkDevice& device, VkPhysicalDevice& physicalDevice){
        VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        if (vkCreateDescriptorPool(device, &pool_info, nullptr, &Engine::imGUIPool) !=VK_SUCCESS){
            throw std::runtime_error("failed to create imgui descriptor pool!");
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        ImGui::StyleColorsDark();

        io.Fonts->AddFontDefault();

        ImGui_ImplGlfw_InitForVulkan(Engine::window, true);

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.ApiVersion = VK_API_VERSION_1_3;
        init_info.Instance = instance;
        init_info.PhysicalDevice = physicalDevice;
        init_info.Device = device;
        init_info.QueueFamily = Engine::graphicsFamilyIndex;
        init_info.Queue = graphicsQueue;
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = Engine::imGUIPool;
        init_info.MinImageCount = 2;
        init_info.ImageCount = Engine::swapChainImages.size();
        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = checkVkResult;
        init_info.PipelineInfoMain.RenderPass = Engine::renderPass;
        init_info.PipelineInfoMain.Subpass = 0;
        init_info.PipelineInfoMain.MSAASamples = Engine::msaaSamples;
        init_info.UseDynamicRendering = false;
        init_info.MinAllocationSize = 1024 * 1024;

        bool initResult = ImGui_ImplVulkan_Init(&init_info);
        if (!initResult) {
            throw std::runtime_error("ImGui_ImplVulkan_Init failed!");
        }
    }

    void WindowManager::checkVkResult(VkResult err){
        if (err == VK_SUCCESS)
            return;

        const char* errorStr = nullptr;
        switch (err)
        {
            case VK_NOT_READY: errorStr = "VK_NOT_READY"; break;
            case VK_TIMEOUT: errorStr = "VK_TIMEOUT"; break;
            case VK_EVENT_SET: errorStr = "VK_EVENT_SET"; break;
            case VK_EVENT_RESET: errorStr = "VK_EVENT_RESET"; break;
            case VK_INCOMPLETE: errorStr = "VK_INCOMPLETE"; break;
            case VK_ERROR_OUT_OF_HOST_MEMORY: errorStr = "VK_ERROR_OUT_OF_HOST_MEMORY"; break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY: errorStr = "VK_ERROR_OUT_OF_DEVICE_MEMORY"; break;
            case VK_ERROR_INITIALIZATION_FAILED: errorStr = "VK_ERROR_INITIALIZATION_FAILED"; break;
            case VK_ERROR_DEVICE_LOST: errorStr = "VK_ERROR_DEVICE_LOST"; break;
            case VK_ERROR_MEMORY_MAP_FAILED: errorStr = "VK_ERROR_MEMORY_MAP_FAILED"; break;
            case VK_ERROR_LAYER_NOT_PRESENT: errorStr = "VK_ERROR_LAYER_NOT_PRESENT"; break;
            case VK_ERROR_EXTENSION_NOT_PRESENT: errorStr = "VK_ERROR_EXTENSION_NOT_PRESENT"; break;
            case VK_ERROR_FEATURE_NOT_PRESENT: errorStr = "VK_ERROR_FEATURE_NOT_PRESENT"; break;
            case VK_ERROR_INCOMPATIBLE_DRIVER: errorStr = "VK_ERROR_INCOMPATIBLE_DRIVER"; break;
            case VK_ERROR_TOO_MANY_OBJECTS: errorStr = "VK_ERROR_TOO_MANY_OBJECTS"; break;
            case VK_ERROR_FORMAT_NOT_SUPPORTED: errorStr = "VK_ERROR_FORMAT_NOT_SUPPORTED"; break;
            case VK_ERROR_FRAGMENTED_POOL: errorStr = "VK_ERROR_FRAGMENTED_POOL"; break;
            default: errorStr = "Unknown Vulkan error"; break;
        }

        fprintf(stderr, "[Vulkan Error] %s (%d)\n", errorStr, err);
        fflush(stderr);

        if (err < 0)
            abort();
    }

    void WindowManager::startNewFrame(){
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void WindowManager::renderWindows(){
        WindowManager::renderGeneralInfoWIndow();

        ImGui::Render();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Engine::commandBuffers[Engine::currentFrame]);
    }

    void WindowManager::cleanup(){
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void WindowManager::renderGeneralInfoWIndow(){
        ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_FirstUseEver);
        ImGui::Begin("Info");

        ImGui::Text("Frame time %.2f ms", (Engine::frameTime * 1000) );
        ImGui::Checkbox("Cap FPS", &Engine::capFPS);
        ImGui::SliderInt("Set fps cap", &Engine::fpsCap, 1, 300);
        ImGui::DragFloat("Camera speed", &Engine::camera.velocity,0.1f, 0.0f, FLT_MAX);
        ImGui::DragFloat("FOV", &Engine::camera.fov, 1.0f, 0.0f, FLT_MAX);
        ImGui::DragFloat("View distance", &Engine::camera.far, 1.0f, 0.0f, FLT_MAX);

        ImGui::End();
    }

    void WindowManager::renderEditor(){
        
    }
}