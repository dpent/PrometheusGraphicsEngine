#include "../headers/gameObjectOperations.h"
#include "../../engine/headers/stb_image.h"
#include "../../engine/headers/engine.h"
#include <vulkan/vulkan_core.h>
#include "../headers/descriptorOperations.h"


using namespace Prometheus;

namespace Prometheus{

    void createObject(std::string texturePath, std::string modelPath, 
        VkDevice &device, VkPhysicalDevice &physicalDevice, VkQueue &graphicsQueue,
        VkCommandPool& commandPool
    ){

        new GameObject(texturePath,modelPath,STBI_rgb_alpha,
        device,physicalDevice,graphicsQueue, commandPool);
    }

    void deleteObject(GameObject* object,VkDevice& device){
    
        Engine::canDeleteObjectMutex.lock();

        uint64_t id = object->id;

        //if(Engine::gameObjectMap.count(id)!=0){

        Engine::gameObjectMutex.lock();

        std::string meshPath = object->meshPath;

        Engine::objectsByMesh.at(meshPath).erase(id);

        if(Engine::objectsByMesh.at(meshPath).empty()){
            Engine::meshMutex.lock();
            Engine::meshMap.erase(meshPath);
            Engine::objectsByMesh.erase(meshPath);
            Engine::meshMutex.unlock();

            Engine::recreateVertexIndexBuffer=true;
        }

        Engine::gameObjectMutex.unlock();
        //}
        
        Engine::canDeleteObjectMutex.unlock();
        
        if (object != nullptr) {
            object->terminate(device);
            delete object->info;
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

                if(Engine::framesSinceTextureQueuedForDeletion[path][i]==Engine::MAX_FRAMES_IN_FLIGHT+2){
                    tex.terminate(device);
                    texVec.erase(texVec.begin()+i);
                    Engine::framesSinceTextureQueuedForDeletion[path]
                    .erase(Engine::framesSinceTextureQueuedForDeletion[path].begin()+i);
                }
            }
        }

        Engine::textureQueuedMutex.unlock();
    }

    void updateGameObjects(Latch* latch, sem_t* setReady){
        
        GameObject* object = Engine::objectDQueue.tail;
        uint64_t objectsToCheck = 0;

        if(Engine::objectDQueue.size % 2 == 0){
            objectsToCheck = Engine::objectDQueue.size >> 1;
        }else{
            objectsToCheck = (Engine::objectDQueue.size >> 1) - 1;
        }

        sem_wait(setReady);

        for(uint64_t i=0; i<objectsToCheck; i++){
            Engine::textureMutex.lock();
            if (Engine::textureIndices.count(object->texturePath)==0) {

                Engine::textureIndices[object->texturePath] = Engine::meshSet[object->meshPath]->textures.size();
                Engine::meshSet[object->meshPath]->textures.push_back(&Engine::textureMap.at(object->texturePath));    
            }
            
            object->updateInstanceInfo(Engine::textureIndices.at(object->texturePath));

            Engine::textureMutex.unlock();

            object->update();

            Engine::meshSet[object->meshPath]->multiThreadMutex.lock();

            Engine::meshSet[object->meshPath]->instances.push_back(
                *(object->info)
            );
            Engine::meshSet[object->meshPath]->objects.push_back(object);

            Engine::meshSet[object->meshPath]->multiThreadMutex.unlock();

            object = object->prev;
        }

        latch->post();
        sem_wait(setReady);

        while(latch->getCount() == 0){
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        object = Engine::objectDQueue.tail;

        for(uint64_t i=0; i<objectsToCheck; i++){
            
            if(object->moved){
                object->checkCollisions();
            }
            
            object = object->prev;
        }
        
        latch->post();
    }

    void updateObjectsAndDescriptors(VkDevice& device, sem_t* jobDoneSem, sem_t* safeToMakeInstanceBuffer,
        Latch* latch, sem_t* setReady){

        for(size_t i=0; i<Engine::meshBatches.size(); i++){
            delete Engine::meshBatches[i];
        }
        Engine::meshBatches.clear();
        Engine::meshSet.clear();
        Engine::textureIndices.clear();
        uint64_t objectsToCheck = 0;

        Engine::meshBatches.reserve(Engine::objectsByMesh.size());
        for(auto& meshName : Engine::objectsByMesh){
            Engine::meshBatches.push_back(new MeshBatch(meshName.first));
            Engine::meshSet.insert({meshName.first,Engine::meshBatches[Engine::meshBatches.size() - 1]});
        }

        sem_post(setReady);
        
        if(Engine::objectDQueue.size % 2 == 0){
            objectsToCheck = Engine::objectDQueue.size >> 1;
        }else{
            objectsToCheck = (Engine::objectDQueue.size >> 1) + 1;
        }

        GameObject* object = Engine::objectDQueue.head;

        for(uint64_t i=0; i<objectsToCheck; i++){
            Engine::textureMutex.lock();
            if (Engine::textureIndices.count(object->texturePath)==0) {

                Engine::textureIndices[object->texturePath] = Engine::meshSet[object->meshPath]->textures.size();
                Engine::meshSet[object->meshPath]->textures.push_back(&Engine::textureMap.at(object->texturePath));    
            }
            
            object->updateInstanceInfo(Engine::textureIndices.at(object->texturePath));
            
            Engine::textureMutex.unlock();

            object->update();

            Engine::meshSet[object->meshPath]->multiThreadMutex.lock();

            Engine::meshSet[object->meshPath]->instances.push_back(
                *(object->info)
            );
            Engine::meshSet[object->meshPath]->objects.push_back(object);

            Engine::meshSet[object->meshPath]->multiThreadMutex.unlock();

            object = object->next;
        }

        latch->wait();
        sem_post(setReady);

        object = Engine::objectDQueue.head;

        for(uint64_t i=0; i<objectsToCheck; i++){
            
            if(object->moved){
                object->checkCollisions();
            }
            
            object = object->next;
        }

        latch->wait();
        delete latch;

        /*std::cout<<"There are "<<Engine::meshBatches.size()<<" batches and "<<Engine::objectDQueue.size<<" objects"<<std::endl;

        for(size_t i=0; i<Engine::meshBatches.size(); i++){
            std::cout<<Engine::meshBatches[i]->meshPath<<" "<<Engine::meshBatches[i]->objects.size()<<" "<<Engine::meshBatches[i]->textures.size()<<std::endl;
        }*/


        sem_post(safeToMakeInstanceBuffer);


        if(Engine::meshBatches.size()!=Engine::descriptorSets.size() || Engine::recreateDescriptors){

            recreateDescriptorSetsAndPool(device,jobDoneSem);
        }else{
            sem_post(&Engine::descriptorsReadySemaphore);
        }
    }

    void splitObjectsAndCreateJobs(uint64_t& objectsPerThread,
        std::vector<std::unordered_map<std::string,std::unordered_map<uint64_t,GameObject*>>>& objectPieces,
        Latch& latch, std::vector<std::unordered_map<std::string,MeshBatch>>& batchPieces)
    {
        int thread = 0;
        uint64_t totalObjects = 0;
        
        Engine::meshBatches.clear();

        for (auto& [meshName, innerMap] : Engine::objectsByMesh) {

            if(innerMap.size()<=objectsPerThread-totalObjects){
                objectPieces[thread][meshName] = innerMap;
                totalObjects+=innerMap.size();

            }else{
                for (auto& [id, objPtr] : innerMap) {

                    objectPieces[thread][meshName][id] = objPtr;
                    totalObjects++;

                    if(totalObjects==objectsPerThread * (thread + 1)){
                        thread++;
                    }
                }
            }
        }

        for(size_t i=0; i<objectPieces.size(); i++){
            Job j = Job(UPDATE_GAME_OBJECTS);
            j.data.emplace_back(std::in_place_type<std::unordered_map<std::string,std::unordered_map<uint64_t,GameObject*>>*>, &objectPieces[i]);
            j.data.emplace_back(std::in_place_type<std::unordered_map<std::string,MeshBatch>*>, &batchPieces[i]);
            j.data.emplace_back(std::in_place_type<Latch*>, &latch);
            
            Engine::queueMutex.lock();
            Engine::jobQueue.push(j);
            Engine::queueMutex.unlock();

            sem_post(&Engine::workInQueueSemaphore);
        }

    }
}