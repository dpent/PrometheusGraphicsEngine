// Prometheus.h : Include file for standard system include files,
// or project specific include files.

#pragma once

//#define RELEASE
//#define RAY_TRACING

#include <iostream>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <thread>
#include <semaphore>
#include <mutex>
#include <unordered_map>
#include <queue>
#include <string>
#include <algorithm>
#include <set>
#include <array>
#include <filesystem>
#include <fstream>
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cmath>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>
#include "../../gameObjects/headers/tiny_obj_loader.h"
#include <stdexcept>
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_stdlib.h"
#include "../../imgui/imgui_impl_vulkan.h"
#include "../../imgui/imgui_impl_glfw.h"
#include "../../imgui/imgui_internal.h"
#include "doubleEndedQueue.h"
#include <list>
#include <chrono>
#include <cstdint>
    
// Common colors
constexpr glm::vec3 COLOR_BLACK = glm::vec3(0.0f, 0.0f, 0.0f);
constexpr glm::vec3 COLOR_WHITE = glm::vec3(1.0f, 1.0f, 1.0f);
constexpr glm::vec3 COLOR_RED = glm::vec3(1.0f, 0.0f, 0.0f);
constexpr glm::vec3 COLOR_GREEN = glm::vec3(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 COLOR_BLUE = glm::vec3(0.0f, 0.0f, 1.0f);
constexpr glm::vec3 COLOR_YELLOW = glm::vec3(1.0f, 1.0f, 0.0f);
constexpr glm::vec3 COLOR_CYAN = glm::vec3(0.0f, 1.0f, 1.0f);
constexpr glm::vec3 COLOR_MAGENTA = glm::vec3(1.0f, 0.0f, 1.0f);

// Grays
constexpr glm::vec3 COLOR_GRAY = glm::vec3(0.5f, 0.5f, 0.5f);
constexpr glm::vec3 COLOR_LIGHT_GRAY = glm::vec3(0.75f, 0.75f, 0.75f);
constexpr glm::vec3 COLOR_DARK_GRAY = glm::vec3(0.25f, 0.25f, 0.25f);

// Orange/Brown
constexpr glm::vec3 COLOR_ORANGE = glm::vec3(1.0f, 0.647f, 0.0f);
constexpr glm::vec3 COLOR_BROWN = glm::vec3(0.647f, 0.165f, 0.165f);
constexpr glm::vec3 COLOR_TAN = glm::vec3(0.824f, 0.706f, 0.549f);

// Pink/Purple
constexpr glm::vec3 COLOR_PINK = glm::vec3(1.0f, 0.753f, 0.796f);
constexpr glm::vec3 COLOR_PURPLE = glm::vec3(0.502f, 0.0f, 0.502f);
constexpr glm::vec3 COLOR_VIOLET = glm::vec3(0.933f, 0.510f, 0.933f);

// Light variants
constexpr glm::vec3 COLOR_LIGHT_RED = glm::vec3(1.0f, 0.5f, 0.5f);
constexpr glm::vec3 COLOR_LIGHT_GREEN = glm::vec3(0.565f, 0.933f, 0.565f);
constexpr glm::vec3 COLOR_LIGHT_BLUE = glm::vec3(0.678f, 0.847f, 0.902f);
constexpr glm::vec3 COLOR_SUN = glm::vec3(1.0f, 204.0f / 255.0f, 51.0f / 255.0f);

// Dark variants
constexpr glm::vec3 COLOR_DARK_RED = glm::vec3(0.545f, 0.0f, 0.0f);
constexpr glm::vec3 COLOR_DARK_GREEN = glm::vec3(0.0f, 0.392f, 0.0f);
constexpr glm::vec3 COLOR_DARK_BLUE = glm::vec3(0.0f, 0.0f, 0.545f);

// Nature
constexpr glm::vec3 COLOR_LIME = glm::vec3(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 COLOR_FOREST = glm::vec3(0.133f, 0.545f, 0.133f);
constexpr glm::vec3 COLOR_OLIVE = glm::vec3(0.502f, 0.502f, 0.0f);
constexpr glm::vec3 COLOR_SKY_BLUE = glm::vec3(0.529f, 0.808f, 0.922f);
constexpr glm::vec3 COLOR_NAVY = glm::vec3(0.0f, 0.0f, 0.502f);

// Metal/Special
constexpr glm::vec3 COLOR_GOLD = glm::vec3(1.0f, 0.843f, 0.0f);
constexpr glm::vec3 COLOR_SILVER = glm::vec3(0.753f, 0.753f, 0.753f);
constexpr glm::vec3 COLOR_BEIGE = glm::vec3(0.961f, 0.961f, 0.863f);


template<int C, int R, typename T, glm::qualifier Q>
constexpr std::string printMatrix(const glm::mat<C, R, T, Q>& m)
{
    std::ostringstream oss;
    for (int row = 0; row < R; ++row)
    {
        for (int col = 0; col < C; ++col)
        {
            oss << m[col][row] << " ";
        }
        oss << '\n';
    }

    return oss.str();
}