#include "../headers/gameObject.h"
#include "../../engine/headers/textureManager.h"
#include <vulkan/vulkan_core.h>


using namespace Prometheus;

namespace Prometheus{
    uint64_t GameObject::autoIncrementId=0;

    GameObject::GameObject(const char* texturePath,int req_comp, VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue){
        this->id=GameObject::autoIncrementId;
        GameObject::autoIncrementId++;
        this->texturePath=texturePath;

        TextureManager::createTextureImage(texturePath, req_comp, device, physicalDevice,
        this->textureImage,this->textureImageMemory,graphicsQueue);
    }

    GameObject::~GameObject(){
    }
}

