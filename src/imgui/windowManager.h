#pragma once
#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"

namespace Prometheus{
    class WindowManager{

    public:
        static void initGUI(VkInstance& instance, VkQueue& graphicsQueue,
        VkDevice& device, VkPhysicalDevice& physicalDevice);
        static void checkVkResult(VkResult err);
        static void startNewFrame();
        static void renderWindows();
        static void cleanup();

        static void renderGeneralInfoWIndow();
        static void renderEditor();

    };
}