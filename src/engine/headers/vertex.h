#include <array>
#include <glm/glm.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace Prometheus{
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        static std::array<VkVertexInputBindingDescription,2> getBindingDescription();

        static std::array<VkVertexInputAttributeDescription, 8> getAttributeDescriptions();
    };

    struct CameraObject
    {
        glm::mat4 view;
        glm::mat4 proj;   
    };
    

    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
}