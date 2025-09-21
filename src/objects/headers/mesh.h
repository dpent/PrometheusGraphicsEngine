#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>


namespace Prometheus{
    struct Mesh{
        uint32_t vertexOffset;
        uint32_t vertexCount;
        uint32_t indexOffset;
        uint32_t indexCount;
        std::string meshPath;

        Mesh();
        Mesh(uint32_t vertexOffset, uint32_t vertexCount, uint32_t indexOffset, uint32_t indexCount);
        std::string toString();
    };
}