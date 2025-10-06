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
}