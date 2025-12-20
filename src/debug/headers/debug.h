#pragma once
#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES //REMEMBER THIS IS SUPPOSED TO ALIGN EVERYTHING
#include <glm/glm.hpp>  

namespace Prometheus
{
    class Debug{
        public:

        static void drawLine(glm::vec3 startPos, glm::vec3 endPos, glm::vec3 color);
        static void drawLine(glm::vec2 startPos, glm::vec2 endPos, glm::vec3 color);
    };

    // Common colors
    constexpr glm::vec3 COLOR_BLACK       = glm::vec3(0.0f, 0.0f, 0.0f);
    constexpr glm::vec3 COLOR_WHITE       = glm::vec3(1.0f, 1.0f, 1.0f);
    constexpr glm::vec3 COLOR_RED         = glm::vec3(1.0f, 0.0f, 0.0f);
    constexpr glm::vec3 COLOR_GREEN       = glm::vec3(0.0f, 1.0f, 0.0f);
    constexpr glm::vec3 COLOR_BLUE        = glm::vec3(0.0f, 0.0f, 1.0f);
    constexpr glm::vec3 COLOR_YELLOW      = glm::vec3(1.0f, 1.0f, 0.0f);
    constexpr glm::vec3 COLOR_CYAN        = glm::vec3(0.0f, 1.0f, 1.0f);
    constexpr glm::vec3 COLOR_MAGENTA     = glm::vec3(1.0f, 0.0f, 1.0f);

    // Grays
    constexpr glm::vec3 COLOR_GRAY        = glm::vec3(0.5f, 0.5f, 0.5f);
    constexpr glm::vec3 COLOR_LIGHT_GRAY  = glm::vec3(0.75f, 0.75f, 0.75f);
    constexpr glm::vec3 COLOR_DARK_GRAY   = glm::vec3(0.25f, 0.25f, 0.25f);

    // Orange/Brown
    constexpr glm::vec3 COLOR_ORANGE      = glm::vec3(1.0f, 0.647f, 0.0f);
    constexpr glm::vec3 COLOR_BROWN       = glm::vec3(0.647f, 0.165f, 0.165f);
    constexpr glm::vec3 COLOR_TAN         = glm::vec3(0.824f, 0.706f, 0.549f);

    // Pink/Purple
    constexpr glm::vec3 COLOR_PINK        = glm::vec3(1.0f, 0.753f, 0.796f);
    constexpr glm::vec3 COLOR_PURPLE      = glm::vec3(0.502f, 0.0f, 0.502f);
    constexpr glm::vec3 COLOR_VIOLET      = glm::vec3(0.933f, 0.510f, 0.933f);

    // Light variants
    constexpr glm::vec3 COLOR_LIGHT_RED   = glm::vec3(1.0f, 0.5f, 0.5f);
    constexpr glm::vec3 COLOR_LIGHT_GREEN = glm::vec3(0.565f, 0.933f, 0.565f);
    constexpr glm::vec3 COLOR_LIGHT_BLUE  = glm::vec3(0.678f, 0.847f, 0.902f);

    // Dark variants
    constexpr glm::vec3 COLOR_DARK_RED    = glm::vec3(0.545f, 0.0f, 0.0f);
    constexpr glm::vec3 COLOR_DARK_GREEN  = glm::vec3(0.0f, 0.392f, 0.0f);
    constexpr glm::vec3 COLOR_DARK_BLUE   = glm::vec3(0.0f, 0.0f, 0.545f);

    // Nature
    constexpr glm::vec3 COLOR_LIME        = glm::vec3(0.0f, 1.0f, 0.0f);
    constexpr glm::vec3 COLOR_FOREST      = glm::vec3(0.133f, 0.545f, 0.133f);
    constexpr glm::vec3 COLOR_OLIVE       = glm::vec3(0.502f, 0.502f, 0.0f);
    constexpr glm::vec3 COLOR_SKY_BLUE    = glm::vec3(0.529f, 0.808f, 0.922f);
    constexpr glm::vec3 COLOR_NAVY        = glm::vec3(0.0f, 0.0f, 0.502f);

    // Metal/Special
    constexpr glm::vec3 COLOR_GOLD        = glm::vec3(1.0f, 0.843f, 0.0f);
    constexpr glm::vec3 COLOR_SILVER      = glm::vec3(0.753f, 0.753f, 0.753f);
    constexpr glm::vec3 COLOR_BEIGE       = glm::vec3(0.961f, 0.961f, 0.863f);
}


