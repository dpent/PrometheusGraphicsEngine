#include "../headers/job.h"
#include "../headers/gameObjectOperations.h"
#include "../headers/modelOperations.h"
#include "../headers/descriptorOperations.h"
#include "../headers/bufferOperations.h"


using namespace Prometheus;


namespace Prometheus{

	Job::Job() {}

    void Job::execute(WorkerThread& worker) {
        std::cout << "Regular job done" << std::endl;
    }

    std::string Job::name() {
        return "Job";
    }

    CreateObjectJob::CreateObjectJob(){}

    void CreateObjectJob::execute(WorkerThread& worker) {
        
        createObject(std::get<GameObject*>(this->data[0]),
            *std::get<VkDevice*>(this->data[1]),
            *std::get<VkPhysicalDevice*>(this->data[2]),
            *std::get<VkQueue*>(this->data[3]),
            worker.commandPool);
    }

    std::string CreateObjectJob::name() {
        return "CreateObjectJob";
    }

	DeleteObjectJob::DeleteObjectJob() {}

    void DeleteObjectJob::execute(WorkerThread& worker) {

        deleteObject(std::get<GameObject*>(this->data[0]),
            *std::get<VkDevice*>(this->data[1])
        );
    }

    std::string DeleteObjectJob::name() {
        return "DeleteObjectJob";
    }

	UpdateTextureDeleteQueueJob::UpdateTextureDeleteQueueJob() {}

    void UpdateTextureDeleteQueueJob::execute(WorkerThread& worker) {
        updateTextureDeleteQueue(*std::get<VkDevice*>(this->data[0]));
    }

    std::string UpdateTextureDeleteQueueJob::name() {
        return "UpdateTextureDeleteQueueJob";
    }

	LoadModelJob::LoadModelJob() {}

    void LoadModelJob::execute(WorkerThread& worker) {
        loadModel(std::get<std::string>(this->data[0]),
            *std::get<std::binary_semaphore*>(this->data[1]));
    }

    std::string LoadModelJob::name() {
        return "LoadModelJob";
    }

    UpdateMeshDataStructuresJob::UpdateMeshDataStructuresJob(){}

    void UpdateMeshDataStructuresJob::execute(WorkerThread& worker) {
        removeUnusedMeshes();
    }

    std::string UpdateMeshDataStructuresJob::name() {
        return "UpdateMeshDataStructuresJob";
    }

    UpdateDescriptorDeleteQueueJob::UpdateDescriptorDeleteQueueJob() {}

    void UpdateDescriptorDeleteQueueJob::execute(WorkerThread& worker) {
        updateDescriptorDeleteQueue(*std::get<VkDevice*>(this->data[0]));
    }

    std::string UpdateDescriptorDeleteQueueJob::name() {
        return "UpdateDescriptorDeleteQueueJob";
    }

    RecreateDescriptorsJob::RecreateDescriptorsJob() {}

    void RecreateDescriptorsJob::execute(WorkerThread& worker) {
        recreateDescriptorSetsAndPool(*std::get<VkDevice*>(this->data[0]),
            std::get<std::counting_semaphore<INT_MAX>*>(this->data[1]));
    }

    std::string RecreateDescriptorsJob::name() {
        return "RecreateDescriptorsJob";
    }

    UpdateGameObjectsJob::UpdateGameObjectsJob() {}

    void UpdateGameObjectsJob::execute(WorkerThread& worker) {
        updateGameObjects(
            std::get<Latch*>(this->data[0]),
            std::get<std::binary_semaphore*>(this->data[1])
        );
    }

    std::string UpdateGameObjectsJob::name() {
        return "UpdateGameObjectsJob";
    }

    UpdateGameObjectsAndDescriptorsJob::UpdateGameObjectsAndDescriptorsJob() {}

    void UpdateGameObjectsAndDescriptorsJob::execute(WorkerThread& worker) {
        updateObjectsAndDescriptors(*std::get<VkDevice*>(this->data[0]),
            std::get<std::counting_semaphore<INT_MAX>*>(this->data[1]),
            std::get<std::binary_semaphore*>(this->data[2]),
            std::get<Latch*>(this->data[3]),
            std::get<std::binary_semaphore*>(this->data[4])
        );
    }

    std::string UpdateGameObjectsAndDescriptorsJob::name() {
        return "UpdateGameObjectsAndDescriptorsJob";
    }

    UpdateVertexIndexBufferJob::UpdateVertexIndexBufferJob() {}

    void UpdateVertexIndexBufferJob::execute(WorkerThread& worker) {
        updateVertexIndexBuffer(*std::get<VkDevice*>(this->data[0]),
            *std::get<VkPhysicalDevice*>(this->data[1]),
            *std::get<VkQueue*>(this->data[2]),
            worker.commandPool
        );
    }

    std::string UpdateVertexIndexBufferJob::name() {
        return "UpdateVertexIndexBufferJob";
    }

    UpdateInstanceBufferJob::UpdateInstanceBufferJob() {}

    void UpdateInstanceBufferJob::execute(WorkerThread& worker) {
        updateInstanceBuffer(std::get<uint64_t>(this->data[0]));
    }

    std::string UpdateInstanceBufferJob::name() {
        return "UpdateInstanceBufferJob";
    }

    MakeInstanceBufferJob::MakeInstanceBufferJob() {}

    void MakeInstanceBufferJob::execute(WorkerThread& worker) {
        recreateInstanceBuffers(*std::get<VkDevice*>(this->data[0]),
            *std::get<VkPhysicalDevice*>(this->data[1]),
            std::get<std::binary_semaphore*>(this->data[2])
        );
    }

    std::string MakeInstanceBufferJob::name() {
        return "MakeInstanceBufferJob";
    }

    RecordCommandBufferJob::RecordCommandBufferJob() {}

    void RecordCommandBufferJob::execute(WorkerThread& worker) {
        recordCommandBuffer(*std::get<VkCommandBuffer*>(this->data[0]),
            std::get<uint32_t>(this->data[1]),
            *std::get<VkDevice*>(this->data[2]),
            *std::get<VkPhysicalDevice*>(this->data[3])
        );
    }

    std::string RecordCommandBufferJob::name() {
        return "RecordCommandBufferJob";
    }

    DummyJob::DummyJob() {}

    void DummyJob::execute(WorkerThread& worker) {
        std::cout << "Dummy job executed" << std::endl;
    }

    std::string DummyJob::name() {
        return "DummyJob";
    }

    PrepareForJoinJob::PrepareForJoinJob() {}

    void PrepareForJoinJob::execute(WorkerThread& worker) {
        cleanup(*std::get<VkDevice*>(this->data[0]),
            worker.commandPool
        );
    }

    std::string PrepareForJoinJob::name() {
        return "PrepareForJoinJob";
    }
}
