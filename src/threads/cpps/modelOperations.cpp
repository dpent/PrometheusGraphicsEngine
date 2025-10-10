#include "../headers/modelOperations.h"
#include "../../engine/headers/engine.h"
#include "../../engine/headers/modelManager.h"

using namespace Prometheus;

namespace Prometheus{
    void loadModel(std::string modelPath, sem_t& meshLoadSemaphore){

        ModelManager::loadModel(modelPath, meshLoadSemaphore);
    }

    void removeUnusedMeshes(){

        Engine::gameObjectMutex.lock();
        for (auto it = Engine::objectsByMesh.begin(); it != Engine::objectsByMesh.end(); ) {

            if (it->second.empty()) {

                Engine::meshMutex.lock();
                Engine::meshMap.erase(it->first);
                Engine::meshMutex.unlock();

                it = Engine::objectsByMesh.erase(it);

                Engine::recreateVertexIndexBuffer=true;

            } else {
                it++;
            }
        }
        Engine::gameObjectMutex.unlock();
    }
}