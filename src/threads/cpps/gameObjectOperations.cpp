#include "../headers/gameObjectOperations.h"
#include "../../engine/headers/stb_image.h"
#include "../../engine/headers/engine.h"


using namespace Prometheus;

namespace Prometheus{

    void createObject(std::string texturePath, std::string modelPath, 
        VkDevice &device, VkPhysicalDevice &physicalDevice, VkQueue &graphicsQueue
    ){

        new GameObject(texturePath,modelPath,STBI_rgb_alpha,
        device,physicalDevice,graphicsQueue);
    }

    void deleteObject(uint64_t id,VkDevice& device){
        Engine::gameObjectMap[id]->terminate(device); //This locks gameObjectMutex

        delete Engine::gameObjectMap[id];

        Engine::gameObjectMutex.unlock();
    }
}