#include "../../engine/headers/engine.h"
#include <vulkan/vulkan_core.h> 
#include <sstream>


using namespace Prometheus;

namespace Prometheus{
    uint64_t GameObject::autoIncrementId=0;

    GameObject::GameObject(const char* texturePath,int req_comp, VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue,
    std::string meshPath)
    {
        this->id=GameObject::autoIncrementId;
        GameObject::autoIncrementId++;
        this->texturePath=texturePath;
        this->meshPath=meshPath;

        if (Engine::textureMap.find(texturePath) != Engine::textureMap.end()) {
            Engine::objectIdsByTexture[texturePath].push_back(this->id);
            this->textureVecIndex=Engine::objectIdsByTexture[texturePath].size()-1;

            Engine::textureMap[texturePath].count++;

        } else {
            Engine::textureMap.insert(std::make_pair(std::string(texturePath), Texture(texturePath, 4, device, physicalDevice, graphicsQueue)));
            Engine::objectIdsByTexture[texturePath].push_back(this->id);
            this->textureVecIndex=0;
        }
    }

    GameObject::~GameObject(){
    }

    void GameObject::terminate(VkDevice& device){ //Used for object deletion

        auto it = Engine::textureMap.find(this->texturePath);
        it->second.count--;
        if (it != Engine::textureMap.end() && it->second.count==0) {
            it->second.terminate(device);
        }
        Engine::objectIdsByTexture[this->texturePath].erase(Engine::objectIdsByTexture[this->texturePath].begin()+this->textureVecIndex);
    }

    void GameObject::draw(VkCommandBuffer& commandBuffer){
        //std::cout<< "Mesh path " << Engine::meshMap[this->meshPath].toString()<<std::endl;
        vkCmdDrawIndexed(commandBuffer, Engine::meshMap[this->meshPath].indexCount, 
            1, Engine::meshMap[this->meshPath].indexOffset, Engine::meshMap[this->meshPath].vertexOffset, 0);
    }

    GameObject::GameObject(){

    }

    std::string GameObject::toString() {
        std::ostringstream oss;
        oss << "GameObject { "
            << "id=" << id << ", "
            << "autoIncrementId=" << autoIncrementId << ", "
            << "textureVecIndex=" << textureVecIndex << ", "
            << "texturePath=\"" << (texturePath ? texturePath : "null") << "\", "
            << "meshPath=\"" << meshPath << "\" }";
        return oss.str();
    }
}

