#include "../headers/debug.h"
#include "../../engine/headers/engine.h"

using namespace Prometheus;

namespace Prometheus{
    
    void Debug::drawLine(glm::vec3 startPos, glm::vec3 endPos, glm::vec3 color){

        Vertex startVertex = {};

        startVertex.pos = startPos;
        startVertex.color = color;

        Engine::debugVertices.push_back(startVertex);
        Engine::debugIndices.push_back(Engine::debugIndices.size());

        Vertex endVertex = {};

        endVertex.pos = endPos;
        endVertex.color = color;

        Engine::debugVertices.push_back(endVertex);
        Engine::debugIndices.push_back(Engine::debugIndices.size());
    }

    void Debug::drawLine(glm::vec2 startPos, glm::vec2 endPos, glm::vec3 color){

        Vertex startVertex = {};

        startVertex.pos = glm::vec3(startPos, 0.0f);
        startVertex.color = color;

        Engine::debugVertices.push_back(startVertex);
        Engine::debugIndices.push_back(Engine::debugIndices.size());

        Vertex endVertex = {};

        endVertex.pos = glm::vec3(endPos, 0.0f);;
        endVertex.color = color;

        Engine::debugVertices.push_back(endVertex);
        Engine::debugIndices.push_back(Engine::debugIndices.size());
    }
}