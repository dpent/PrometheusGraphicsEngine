#pragma once

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <variant>
#include <string>
#include <semaphore>
#include <unordered_map>
#include "../../objects/headers/gameObject.h"
#include "../../objects/headers/mesh.h"
#include "../../engine/headers/latch.h"
#include <barrier>
#include "workerThread.h"

namespace Prometheus{


    struct Job{
    public:

        std::vector<std::variant<std::string,
        VkDevice*,
        VkPhysicalDevice*,
        VkQueue*, 
        uint64_t,
        std::binary_semaphore*,
        std::counting_semaphore<INT_MAX>*,
        std::unordered_map<std::string,std::unordered_map<uint64_t,GameObject*>>*,
        std::unordered_map<std::string,MeshBatch>*, 
        Latch*,
        uint32_t,
        VkCommandBuffer*,
        GameObject*>> data;

        Job();

        virtual void execute(WorkerThread& worker);
        virtual std::string name();
    };

    struct CreateObjectJob : Job {

    public:

        CreateObjectJob();
		void execute(WorkerThread& worker) override;
        std::string name() override;
    };

    struct DeleteObjectJob : Job {
    public:

		DeleteObjectJob();
		void execute(WorkerThread& worker) override;
        std::string name() override;
    };

    struct UpdateTextureDeleteQueueJob : Job {
    public:

		UpdateTextureDeleteQueueJob();
		void execute(WorkerThread& worker) override;
        std::string name() override;
    };

    struct LoadModelJob : Job {
    public:

		LoadModelJob();
		void execute(WorkerThread& worker) override;
        std::string name() override;
    };

    struct UpdateMeshDataStructuresJob : Job {
    public:

        UpdateMeshDataStructuresJob();
        void execute(WorkerThread& worker) override;
        std::string name() override;
    };

    struct UpdateDescriptorDeleteQueueJob : Job {
    public:

        UpdateDescriptorDeleteQueueJob();
        void execute(WorkerThread& worker) override;
        std::string name() override;
    };

    struct RecreateDescriptorsJob : Job {
    public:

        RecreateDescriptorsJob();
        void execute(WorkerThread& worker) override;
        std::string name() override;    
    };

    struct UpdateGameObjectsJob : Job {
    public:

        UpdateGameObjectsJob();
        void execute(WorkerThread& worker) override;
        std::string name() override;
    };

    struct UpdateGameObjectsAndDescriptorsJob : Job {
    public:

        UpdateGameObjectsAndDescriptorsJob();
        void execute(WorkerThread& worker) override;
        std::string name() override;
    };

    struct UpdateVertexIndexBufferJob : Job {
    public:

        UpdateVertexIndexBufferJob();
        void execute(WorkerThread& worker) override;
        std::string name() override;
    };

    struct UpdateInstanceBufferJob : Job {
    public:

        UpdateInstanceBufferJob();
        void execute(WorkerThread& worker) override;
        std::string name() override;
    };

    struct MakeInstanceBufferJob : Job {
    public:

        MakeInstanceBufferJob();
        void execute(WorkerThread& worker) override;
        std::string name() override;
    };
    
    struct RecordCommandBufferJob : Job {
    public:

        RecordCommandBufferJob();
        void execute(WorkerThread& worker) override;
        std::string name() override;
    };

    struct DummyJob : Job {
    public:

        DummyJob();
        void execute(WorkerThread& worker) override;
        std::string name() override;
    };

    struct PrepareForJoinJob : Job {
    public:

        PrepareForJoinJob();
        void execute(WorkerThread& worker) override;
        std::string name() override;
    };
}