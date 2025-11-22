#include "../headers/debug.h"
#include "../../engine/headers/engine.h"

using namespace Prometheus;

namespace Prometheus{
    
    void Debug::drawLine(glm::vec3 startPos, glm::vec3 endPos, glm::vec3 color){

        Vertex startVertex = {};

        startVertex.pos = startPos;
        startVertex.color = color;

        if(Engine::debugVertSet.count(startVertex) == 0){
            Engine::debugVertSet[startVertex] = static_cast<uint32_t>(Engine::debugVertSet.size());
            Engine::debugVertices.push_back(startVertex);
        }

        Engine::debugIndices.push_back(Engine::debugVertSet[startVertex]);

        Vertex endVertex = {};

        endVertex.pos = endPos;
        endVertex.color = color;

        if(Engine::debugVertSet.count(endVertex) == 0){
            Engine::debugVertSet[endVertex] = static_cast<uint32_t>(Engine::debugVertSet.size());
            Engine::debugVertices.push_back(endVertex);
        }

        Engine::debugIndices.push_back(Engine::debugVertSet[endVertex]);
    }

    void Debug::drawLine(glm::vec2 startPos, glm::vec2 endPos, glm::vec3 color){

        Vertex startVertex = {};

        startVertex.pos = glm::vec3(startPos, 0.0f);
        startVertex.color = color;

        if(Engine::debugVertSet.count(startVertex) == 0){
            Engine::debugVertSet[startVertex] = static_cast<uint32_t>(Engine::debugVertSet.size());
            Engine::debugVertices.push_back(startVertex);
        }

        Engine::debugIndices.push_back(Engine::debugVertSet[startVertex]);

        Vertex endVertex = {};

        endVertex.pos = glm::vec3(endPos, 0.0f);
        endVertex.color = color;

        if(Engine::debugVertSet.count(endVertex) == 0){
            Engine::debugVertSet[endVertex] = static_cast<uint32_t>(Engine::debugVertSet.size());
            Engine::debugVertices.push_back(endVertex);
        }

        Engine::debugIndices.push_back(Engine::debugVertSet[endVertex]);
    }
}