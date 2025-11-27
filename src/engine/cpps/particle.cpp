#include "../headers/particle.h"
#include "../headers/engine.h"
#include <random>
#include <ctime> 


using namespace Prometheus;

namespace Prometheus{

    Particle::Particle(){}

    Particle::Particle(glm::vec3 position){
        this->position = position;
    }

    void Particle::addDemoParticles(uint64_t number){
        std::default_random_engine rndEngine((unsigned)time(nullptr));
        std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);

        for (size_t i=0; i<number; i++) {
            float r = 0.25f * sqrt(rndDist(rndEngine));
            float theta = rndDist(rndEngine) * 2 * 3.14159265358979323846;
            float x = r * cos(theta) * 100.0f / 200.0f;
            float y = r * sin(theta);

            Particle particle = Particle();

            particle.position = glm::vec3(x, y, 0.0f);

            Engine::particles.push_back(particle);
        }

        if(Engine::particles.size() * sizeof(Particle) > Engine::shaderStorageBufferSize){
            Engine::shaderStorageBufferSize = (Engine::particles.size() * sizeof(Particle)) << 1;
        }

    }

    std::array<VkVertexInputAttributeDescription, 1> Particle::getAttributeDescriptions(){
        std::array<VkVertexInputAttributeDescription, 1> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Particle, position);

        return attributeDescriptions;
    }

    std::array<VkVertexInputBindingDescription,1> Particle::getBindingDescriptions(){
        std::array<VkVertexInputBindingDescription,1> bindingDescriptions = {};

        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Particle);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescriptions;
    }
}