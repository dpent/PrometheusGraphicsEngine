#include "../../engine/headers/engine.h"
#include <vulkan/vulkan_core.h> 
#include <sstream>
#include "../../engine/headers/modelManager.h"


using namespace Prometheus;

namespace Prometheus{
    uint64_t GameObject::autoIncrementId=0;

    GameObject::GameObject(std::string texturePath, std::string modelPath, int req_comp, VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue)
    {
        this->id=GameObject::autoIncrementId;
        GameObject::autoIncrementId++;
        this->texturePath=texturePath;
        this->meshPath=modelPath;
        this->modelMatrix=glm::mat4(1.0f);

        Engine::textureMutex.lock();
        if (Engine::textureMap.find(texturePath) != Engine::textureMap.end()) {
            Engine::objectIdsByTexture[texturePath].push_back(this->id);
            this->textureVecIndex=Engine::objectIdsByTexture[texturePath].size()-1;

            Engine::textureMap[texturePath].count++;

        } else {
            Engine::textureMap.insert(std::make_pair(texturePath, Texture(texturePath, 4, device, physicalDevice, graphicsQueue)));
            Engine::objectIdsByTexture[texturePath].push_back(this->id);
            this->textureVecIndex=0;
        }
        Engine::textureMutex.unlock();

        if(Engine::meshMap.find(modelPath) == Engine::meshMap.end()){
            ModelManager::loadModel(modelPath); //Also inserts the mesh into meshMap
        }

        Engine::gameObjectMutex.lock();

        Engine::objectsByMesh[modelPath][this->id]=this;
        Engine::gameObjects.push_back(this);
        Engine::gameObjectMap.insert({this->id,this});

        Engine::gameObjectMutex.unlock();

        Engine::recreateInstanceBuffer=true;
        Engine::recreateDescriptors=true;
    }

    GameObject::~GameObject(){
    }

    void GameObject::terminate(VkDevice& device){ //Used for object deletion

        Engine::textureMutex.lock();

        auto it = Engine::textureMap.find(this->texturePath);
        it->second.count--;
        if (it != Engine::textureMap.end() && it->second.count==0) {
            it->second.terminate(device);
        }
        Engine::objectIdsByTexture[this->texturePath].erase(Engine::objectIdsByTexture[this->texturePath].begin()+this->textureVecIndex);

        Engine::textureMutex.unlock();

        Engine::gameObjectMutex.lock();

        Engine::objectsByMesh[meshPath].erase(this->id);
    }

    void GameObject::draw(VkCommandBuffer& commandBuffer, uint32_t instanceCount, uint32_t firstInstance){
        //std::cout<<Engine::meshMap[this->meshPath].toString()<<std::endl;

        vkCmdDrawIndexed(commandBuffer, Engine::meshMap[this->meshPath].indices.size(), 
            instanceCount, Engine::meshMap[this->meshPath].indexOffset, Engine::meshMap[this->meshPath].vertexOffset, firstInstance);
    }

    GameObject::GameObject(){

    }

    std::string GameObject::toString() {
        std::ostringstream oss;
        oss << "GameObject { "
            << "id=" << id << ", "
            << "autoIncrementId=" << autoIncrementId << ", "
            << "textureVecIndex=" << textureVecIndex << ", "
            << "texturePath=\"" << (texturePath=="" ? texturePath : "null") << "\", "
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

    void GameObject::createObjectThreaded(std::string texturePath,std::string modelPath, 
        VkDevice& device, VkPhysicalDevice& physicalDevice,
        VkQueue& graphicsQueue
    ){
        Job j = Job(CREATE_OBJECT);
        j.data.push_back(texturePath);
        j.data.push_back(modelPath);
        j.data.push_back(&device);
        j.data.push_back(&physicalDevice);
        j.data.push_back(&graphicsQueue);

        Engine::threadManager.jobQueue.push(j);
    }
}

