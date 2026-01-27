#include "../headers/debug.h"
#include "../headers/descriptorManager.h"
#include "../headers/engine.h"

Pipeline Debug::debugPipeline;
Descriptor Debug::debugDescriptor;

Buffer Debug::debugVertexBuffer;
Buffer Debug::lineSSBO;

std::vector<DebugVertex> Debug::debugVertices;
std::vector<Line> Debug::lines;


void Debug::drawLine(glm::vec3 start, glm::vec3 finish, glm::vec3 color) {

    Line line(start, finish, color);
    Debug::lines.push_back(line);
}

void Debug::clearDebugData() {

    Debug::lines.clear();
}

void Debug::init() {

    DescriptorManager::createDebugSetLayout();
    PipelineManager::createDebugPipeline();
    Debug::debugVertices.push_back(DebugVertex{ 0.0f });
    Debug::debugVertices.push_back(DebugVertex{ 1.0f });

    Debug::lines.reserve(32);

    BufferManager::createDebugVertexBuffer(sizeof(DebugVertex) * Debug::debugVertices.size());
}

void Debug::drawLineCircle(glm::vec3 center, float radius, uint16_t segments, glm::vec3 color, glm::vec3 normal = {1.0f,0.0f,1.0f}) {

    if (segments < 3) return;

    normal = glm::normalize(normal);

    // Find a vector that is NOT parallel to the normal
    glm::vec3 tangent =
        (fabs(normal.y) < 0.999f)
        ? glm::vec3(0, 1, 0)
        : glm::vec3(1, 0, 0);

    // Build orthonormal basis
    glm::vec3 bitangent = glm::normalize(glm::cross(normal, tangent));
    tangent = glm::cross(bitangent, normal);

    float step = glm::two_pi<float>() / float(segments);

    glm::vec3 prevPoint =
        center + tangent * radius;

    for (uint16_t i = 1; i <= segments; ++i) {
        float angle = step * i;

        glm::vec3 nextPoint =
            center +
            (cos(angle) * tangent +
                sin(angle) * bitangent) * radius;

        Debug::drawLine(prevPoint, nextPoint, color);
        prevPoint = nextPoint;
    }
}