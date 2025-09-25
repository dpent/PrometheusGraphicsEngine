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
        this->modelMatrix=glm::mat4(1.0f);

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
            2, Engine::meshMap[this->meshPath].indexOffset, Engine::meshMap[this->meshPath].vertexOffset, 0); //Remember to see instancing
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

    glm::mat4 GameObject::animateCircularMotion(float centerX, float centerY, float centerZ, float radius, float speed, float offset){
        float time   = glfwGetTime(); // or your own frame timer
        time+=offset;

        float x = centerX + radius * cos(time * speed);
        float y = centerY;             // keep same height
        float z = centerZ + radius * sin(time * speed);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
        return model;
    }
}

