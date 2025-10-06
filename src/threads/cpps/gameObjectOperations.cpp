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
        GameObject* object = nullptr;
    
        Engine::gameObjectMutex.lock();

        if(Engine::gameObjectMap.count(id)!=0){
            object=Engine::gameObjectMap[id];

            std::string meshPath = object->meshPath;
            Engine::gameObjectMap.erase(id);

            if(Engine::objectsByMesh.count(meshPath)!=0){
                Engine::objectsByMesh[meshPath].erase(id);

                if(Engine::objectsByMesh[meshPath].size()==0){
                    Engine::objectsByMesh.erase(meshPath);
                }
            }
        }

        Engine::gameObjectMutex.unlock();
        
        if (object != nullptr) {
            object->terminate(device);
            delete object;
        }
    }

    void updateTextureDeleteQueue(VkDevice& device){

        Engine::textureQueuedMutex.lock();

        for (auto& [path, texVec] : Engine::texturesQueuedForDeletion) {
            // iterate from the back to the front
            for (int i = static_cast<int>(texVec.size()) - 1; i >= 0; --i) {

                auto& tex = texVec[i];
                Engine::framesSinceTextureQueuedForDeletion[path][i]++;

                if(Engine::framesSinceTextureQueuedForDeletion[path][i]==Engine::MAX_FRAMES_IN_FLIGHT){
                    tex.terminate(device);
                    texVec.erase(texVec.begin()+i);
                    Engine::framesSinceTextureQueuedForDeletion[path]
                    .erase(Engine::framesSinceTextureQueuedForDeletion[path].begin()+i);
                }
            }
        }

        Engine::textureQueuedMutex.unlock();
    }

    void loadModel(std::string modelPath, sem_t& meshLoadSemaphore){

        ModelManager::loadModel(modelPath, meshLoadSemaphore);
    }
}