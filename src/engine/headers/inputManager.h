#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>

#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES //REMEMBER THIS IS SUPPOSED TO ALIGN EVERYTHING
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>  

namespace Prometheus {
    class InputManager{
    public:
        static void initInputMode(bool sticky, bool lockKeys, bool disableCursor, bool hiddenCursor,
            bool mouseSticky,GLFWwindow* window);

        static void resetCursor(GLFWwindow* window);

        static void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void mouseEnterCallBack(GLFWwindow* window, int entered);
        static void mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods);
        static void mouseScrollCallBack(GLFWwindow* window, double xoffset, double yoffset);
        static void cursorPosCallBack(GLFWwindow* window, double xpos, double ypos);

        static void consumeInput(GLFWwindow* window);
    };
}