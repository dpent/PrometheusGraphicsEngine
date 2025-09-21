#include "../../engine/headers/engine.h"
#include <vulkan/vulkan_core.h>


using namespace Prometheus;

namespace Prometheus{
    uint64_t GameObject::autoIncrementId=0;

    GameObject::GameObject(const char* texturePath,int req_comp, VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue){
        this->id=GameObject::autoIncrementId;
        GameObject::autoIncrementId++;
        this->texturePath=texturePath;

         if (Engine::textureMap.find(texturePath) != Engine::textureMap.end()) {
        } else {
            Engine::textureMap.insert(std::make_pair(std::string(texturePath), Texture(texturePath, 4, device, physicalDevice, graphicsQueue)));
        }
    }

    GameObject::~GameObject(){
    }

    void GameObject::terminate(VkDevice& device){ //Used for object deletion
        auto it = Engine::textureMap.find(this->texturePath);
        if (it != Engine::textureMap.end()) {
            it->second.terminate(device);
        }

    }
}

