#include "../headers/gameObjectOperations.h"
#include "../../engine/headers/stb_image.h"
#include "../../engine/headers/engine.h"
#include <vulkan/vulkan_core.h>
#include "../headers/descriptorOperations.h"


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
    
        Engine::canDeleteObjectMutex.lock();

        if(Engine::gameObjectMap.count(id)!=0){
            object=Engine::gameObjectMap[id];

            Engine::gameObjectMutex.lock();

            std::string meshPath = object->meshPath;
            Engine::gameObjectMap.erase(id);

            Engine::objectsByMesh.at(meshPath).erase(id);

            if(Engine::objectsByMesh.at(meshPath).empty()){
                Engine::meshMutex.lock();
                Engine::meshMap.erase(meshPath);
                Engine::objectsByMesh.erase(meshPath);
                Engine::meshMutex.unlock();
            }

            Engine::gameObjectMutex.unlock();
        }
        
        Engine::canDeleteObjectMutex.unlock();
        
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

    void updateGameObjects(std::unordered_map<std::string,std::unordered_map<uint64_t,GameObject*>>& objectsByMesh,
        std::unordered_map<std::string,MeshBatch>& batchBuffer, Latch& latch){
        
        int i=0;
        int count=0;
        for (auto& [meshName, innerMap] : objectsByMesh) {

            uint64_t currIndex=0;
            std::unordered_map<std::string,uint64_t> textureIndices;
            batchBuffer[meshName]=MeshBatch(meshName);

            for (auto& [id, objPtr] : innerMap) {

                if (textureIndices.find(objPtr->texturePath) == textureIndices.end()) {

                    batchBuffer[meshName].textures.push_back(&Engine::textureMap[objPtr->texturePath]);

                    textureIndices[objPtr->texturePath] = currIndex;
                    currIndex++; 
                }

                batchBuffer[meshName].instances.push_back(
                    InstanceInfo(objPtr->animateCircularMotion(0.0f,0.0f,0.0f,5.0f,2.0f,i*0.25f),textureIndices.at(objPtr->texturePath))
                );
                batchBuffer[meshName].objects.push_back(objPtr);
                i++;
                count++;
            }

            if(batchBuffer[meshName].objects.size()==0){
                batchBuffer.erase(meshName);
            }
        }
        latch.count_down();
    }

    void updateObjectsAndDescriptors(VkDevice& device, sem_t* jobDoneSem, sem_t* safeToMakeInstanceBuffer){

        uint64_t objectsPerThread=0;

        Latch* latch;

        std::vector<std::unordered_map<std::string,std::unordered_map<uint64_t,GameObject*>>> objectPieces;
        std::vector<std::unordered_map<std::string,MeshBatch>> batchPieces;

        uint16_t availableThreads = Engine::threadsAvailable.getValue();

        if(availableThreads<2){

            //Engine::meshMutex.lock();
            Engine::updateGameObjects();
            //Engine::meshMutex.unlock();

            if(Engine::meshBatches.size()!=Engine::descriptorSets.size() || Engine::recreateDescriptors){

                recreateDescriptors(device,jobDoneSem);
            }else{
                sem_post(&Engine::descriptorsReadySemaphore);
            }

            return;
        }

        if(Engine::gameObjectMap.size()>=(availableThreads)){

            objectsPerThread = (Engine::gameObjectMap.size() + availableThreads - 1) / availableThreads; 

            objectPieces.resize(availableThreads);
            batchPieces.resize(availableThreads);

            latch = new Latch(availableThreads);
        }else{
            
            objectsPerThread = Engine::gameObjectMap.size();
            
            objectPieces.resize(1);
            batchPieces.resize(1);

            latch = new Latch(1);
        }

        splitObjectsAndCreateJobs(objectsPerThread,objectPieces,*latch,batchPieces);

        latch->wait();
        //Engine::meshMutex.unlock();
        delete latch;      
        
        mergeAllThreadBatches(batchPieces);

        sem_post(safeToMakeInstanceBuffer);

        if(Engine::meshBatches.size()!=Engine::descriptorSets.size() || Engine::recreateDescriptors){

            recreateDescriptors(device,jobDoneSem);
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
        //Engine::meshMutex.lock();
        
        Engine::meshBatches.clear();

        //std::cout<<"Used "<<objectPieces.size()<<" threads for this update"<<std::endl;

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

    void mergeAllThreadBatches(std::vector<std::unordered_map<std::string,MeshBatch>>& batchPieces){

        std::unordered_map<std::string,MeshBatch> batchBuffer;

        for(size_t i=0; i<batchPieces.size(); i++){
            for (auto& [meshName, data] : batchPieces[i]){

                if(batchBuffer.count(meshName)==0){
                    batchBuffer[meshName]=data;
                }else{
                    batchBuffer[meshName].instances.insert(
                        batchBuffer[meshName].instances.end(),
                        data.instances.begin(),
                        data.instances.end()
                    );

                    batchBuffer[meshName].objects.insert(
                        batchBuffer[meshName].objects.end(),
                        data.objects.begin(),
                        data.objects.end()
                    );

                    batchBuffer[meshName].textures.insert(
                        batchBuffer[meshName].textures.end(),
                        data.textures.begin(),
                        data.textures.end()
                    );
                }
            }
        }

        //Engine::meshMutex.lock();
        VkDeviceSize bufferSize=0;

        //std::cout<<"Mesh paths included in meshBatches"<<std::endl;
        for (auto& [meshName, data] : batchBuffer){
            //std::cout<<meshName<<std::endl;
            Engine::meshBatches.push_back(data);
            bufferSize+=sizeof(InstanceInfo) * data.instances.size();
        }

        if(bufferSize>Engine::instanceBufferSize){
            Engine::recreateInstanceBuffer=true;
        }

        //Engine::meshMutex.unlock();
    }
}