#include "../headers/engine.h"
#include "../../threads/headers/workerThread.h"
#include "../../threads/headers/job.h"

#ifdef __linux__
    std::filesystem::path Engine::exeDir = std::filesystem::canonical("/proc/self/exe").parent_path();
#elif _WIN32
    std::filesystem::path Engine::exeDir = std::filesystem::current_path();
#endif

//CORE
VulkanInstance Engine::vkInstanceInfo;
DeviceInfo Engine::deviceInfo;
SwapChain Engine::swapChainInfo;

VkSampleCountFlagBits Engine::msaaSamples;

QueueHolder Engine::queues;

VkRenderPass Engine::graphicsRenderPass;
Image Engine::depthResource;
Image Engine::colorResource;

Pipeline Engine::graphicsPipeLine;
Descriptor Engine::graphicsDescriptor;

Descriptor Engine::imGuiDescriptor;

DoubleEndedQueue<Material*> Engine::materials;
DoubleEndedQueue<Texture*> Engine::textures;
DoubleEndedQueue<GameObject*> Engine::gameObjects;
DoubleEndedQueue<Mesh*> Engine::meshes;
std::vector<InstanceInfo> Engine::instanceData;

CommandPool Engine::command;

VertexData Engine::vertexIndexData;
Buffer Engine::vertexIndexBuffer;

Buffer Engine::stagingBuffer;
Buffer Engine::instanceDataSSBO;

uint8_t Engine::currentFrame;

std::vector<VkFence> Engine::inFlightFences;
std::vector<VkSemaphore> Engine::imageAvailableSemaphores;
std::vector<VkSemaphore> Engine::renderFinishedSemaphores;

VkSampler Engine::linearSampler;

std::vector<bool> Engine::pressed;

bool Engine::firstFrame = true;
bool Engine::remakeDescriptors = false;
bool Engine::remakeVertexIndexBuffer = false;
bool Engine::remakeInstanceDataSSBO = false;
bool Engine::fullscreen = false;

uint64_t Engine::frameCount = 0;

GarbageQueues Engine::garbage;

int Engine::targetFPS = 60;
uint16_t Engine::FPS = 0;

//LIGHTING
DoubleEndedQueue<Light*> Engine::lights;
DoubleEndedQueue<Light*> Engine::shadowCreatingLights;

Buffer Engine::uniformLightBuffer;
LightUBOData Engine::lightData;
Buffer Engine::uniformShadowLightBuffer;
ShadowLightUBOData Engine::shadowLightData;

Descriptor Engine::shadowLightsDescriptor;

ImageVector* Engine::shadowMaps;
bool Engine::recreateShadowResources;
uint32_t Engine::shadowRes = 2048;
std::vector<VkFramebuffer> Engine::shadowFrameBuffers;

VkRenderPass Engine::shadowRenderPass;
Pipeline Engine::shadowPipeline;
Descriptor Engine::shadowDescriptor;

Image* Engine::dummyImage;

//WINDOW
GLFWwindow* Engine::window = nullptr;
GLFWcursor* Engine::cursor = nullptr;
const int Engine::WIDTH = 1280;
const int Engine::HEIGHT = 720;

std::pair<double, double> Engine::lastKnownMousePos;
bool Engine::rightMouseFirstPress;
bool Engine::rightMousePressedLastFrame;

bool Engine::framebufferResized = false;
VkSurfaceKHR Engine::surface;

Camera Engine::camera;

#ifdef RELEASE
    bool Engine::displayGUI = false;
#else
    bool Engine::displayGUI = true;
#endif

glm::vec4 Engine::viewportLimitsOffsets{ 0.0f };

//EDITOR SPECIFIC
std::vector<float> Engine::vertexIndexHistory;
std::vector<float> Engine::stagingHistory;
std::vector<float> Engine::instanceDataHistory;
std::vector<float> Engine::debugDataHistory;

uint16_t Engine::maxSamples = 20;

//SYNC OBJECTS
std::counting_semaphore<INT_MAX> Engine::jobInQueueSem(0);
std::mutex Engine::jobQueueMutex;

std::mutex Engine::objectCreateMutex;
std::mutex Engine::materialMutex;
std::mutex Engine::meshMutex;
std::mutex Engine::textureMutex;

//THREADS
std::unordered_map<std::thread::id, WorkerThread*> Engine::threadPool;
std::queue<Job*> Engine::jobQueue;

void Engine::run(Engine* engine) {

	Engine::initWindow(engine);
	Engine::initVulkan();
	Engine::mainLoop();
}

void Engine::initWindow(Engine* engine) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    Engine::window = glfwCreateWindow(WIDTH, HEIGHT, "Prometheus", nullptr, nullptr);
    glfwSetWindowUserPointer(Engine::window, engine);
    glfwSetFramebufferSizeCallback(Engine::window, frameBufferResizeCallback);

    Engine::cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    glfwSetCursor(Engine::window, Engine::cursor);

    Engine::lastKnownMousePos = std::pair<double, double>(0.0f, 0.0f);
    Engine::rightMouseFirstPress = true;    
    Engine::rightMousePressedLastFrame = false;

    InputManager::initInputMode(false, false, false, false, false, Engine::window);
}

void Engine::initVulkan() {

    InstanceManager::createInstance(Engine::vkInstanceInfo.instance);
    InstanceManager::setupDebugMessenger(Engine::vkInstanceInfo.instance, Engine::vkInstanceInfo.debugMessenger);

    Engine::createSurface();

    DeviceManager::pickPhysicalDevice();
    DeviceManager::createLogicalDevice();

    Engine::createSyncObjects();

    SwapChainManager::createSwapChain(Engine::swapChainInfo.chain);

    Engine::command.initialize();

    RenderPassManager::createRenderPass();
    RenderPassManager::createShadowRenderPass();

    GUIManager::initImGUI();
    ImageManager::createImageSampler(Engine::linearSampler);

    SwapChainManager::createSwapChainImageViews();

    ImageManager::createDepthResources();
    ImageManager::createColorResources();

    BufferManager::createFrameBuffers(
        Engine::swapChainInfo.frameBuffers,
        Engine::swapChainInfo.imageViews,
        Engine::swapChainInfo.extent,
        Engine::graphicsRenderPass,
        Engine::colorResource.view,
        Engine::depthResource.view
    );

    DescriptorManager::createGraphicsDescriptorSetLayout();
    DescriptorManager::createShadowLightsSetLayout();
    
    PipelineManager::createGraphicsPipeline();
    PipelineManager::createShadowPipeline();

    BufferManager::createStagingBuffer(8192, Engine::stagingBuffer); //8KB TO START

    Engine::instanceData.reserve(256);

    Engine::initThreadPool(std::thread::hardware_concurrency() - 1);

    BufferManager::createUniformBuffer(Engine::uniformLightBuffer, sizeof(LightUBOData));
    BufferManager::createUniformBuffer(Engine::uniformShadowLightBuffer, sizeof(ShadowLightUBOData));
    
    #ifndef RELEASE
    Debug::init();
    #endif

    ImageManager::createDummyImage();

    DescriptorManager::createShadowLightsPool();
    DescriptorManager::createShadowLightsSets();
}

void Engine::mainLoop() {

    Engine::loadDemoScene();
    uint64_t framesThisSecond = 0;

    auto nextFrameTime = std::chrono::steady_clock::now();
    auto secondStart = std::chrono::steady_clock::now();

    while (!glfwWindowShouldClose(Engine::window)) {
        glfwPollEvents();
        InputManager::consumeInput(Engine::window);
        Engine::camera.updateCameraVectors();

        if (Engine::gameObjects.size != 0) {
            nextFrameTime += std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(1.0 / Engine::targetFPS));
            Engine::drawFrame();
            std::this_thread::sleep_until(nextFrameTime);
        }

        framesThisSecond++;
        auto now = std::chrono::steady_clock::now();
        if (now - secondStart >= std::chrono::seconds(1)) {
            secondStart = now;
            Engine::FPS = static_cast<uint16_t>(framesThisSecond);
            framesThisSecond = 0;
        }

        #ifndef RELEASE
        if (Engine::frameCount % 60 == 0)
        {
            Engine::updateSampleVectors();
        }
        #endif

        Engine::createUpdateGarbageJob();

        Engine::frameCount++;
    }

    Engine::killThreadPool();
}

void Engine::frameBufferResizeCallback(GLFWwindow* window, int width, int height) {
    Engine::framebufferResized = true;
}

void Engine::initThreadPool(uint16_t poolSize) {

    std::cout << "\nThreads in pool: " << poolSize << "\n" << std::endl;
    for (uint16_t i = 0; i < poolSize; i++) {

        WorkerThread* wt = new WorkerThread();

        Engine::threadPool[wt->id] = wt;
    }
}

void Engine::killThreadPool() {

    Engine::jobQueueMutex.lock();

    while (!Engine::jobQueue.empty()) {
        Engine::jobQueue.pop();
    }

    for (size_t i = 0; i < Engine::threadPool.size(); i++) {
    
        PrepareForJoinJob* j = new PrepareForJoinJob();
        Engine::jobQueue.push(j);
    }

    Engine::jobQueueMutex.unlock();

    for (const auto& pair : Engine::threadPool) {
        pair.second->alive = false;
    }

    for (size_t i = 0; i < Engine::threadPool.size(); i++) {
        Engine::jobInQueueSem.release();
    }

    int j = 0;
    for (const auto& pair : Engine::threadPool) {
        if (pair.second->thread.joinable()) {
            j++;
            pair.second->thread.join();
        }
    }

    for (const auto& pair : Engine::threadPool) {
        delete pair.second;
    }

}

void Engine::createSurface() {
    if (glfwCreateWindowSurface(Engine::vkInstanceInfo.instance, Engine::window, nullptr, &Engine::surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}


VkSampleCountFlagBits Engine::getMaxUsableSampleCount() {

    VkSampleCountFlags counts = Engine::deviceInfo.physicalProperties.limits.framebufferColorSampleCounts & Engine::deviceInfo.physicalProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

VkFormat Engine::findDepthFormat() {
    return Engine::findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkFormat Engine::findShadowFormat() {

    return Engine::findSupportedFormat(
        {
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D16_UNORM
        },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT |
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT
    );
}

VkFormat Engine::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {

    for (VkFormat format : candidates) {

        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(Engine::deviceInfo.physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

std::vector<char> Engine::readFile(const std::string& filename) {

    std::ifstream file((Engine::exeDir / filename).lexically_normal().string(),
        std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModule Engine::createShaderModule(const std::vector<char>& code) {

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(Engine::deviceInfo.logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }
    return shaderModule;
}

VkPipelineShaderStageCreateInfo Engine::createShaderStageInfo(VkStructureType sType,
    VkShaderStageFlagBits stage,
    VkShaderModule& module,
    const char* pName,
    const VkSpecializationInfo* pSpecializationInfo
) 
{
    VkPipelineShaderStageCreateInfo ShaderStageInfo{};
    ShaderStageInfo.sType = sType;
    ShaderStageInfo.stage = stage;
    ShaderStageInfo.module = module;
    ShaderStageInfo.pName = pName; //Define the entrypoint function (for us its main())
    ShaderStageInfo.pSpecializationInfo = pSpecializationInfo; /*It allows you to specify values for shader constants.
                                                        You can use a single shader module where its behavior
                                                        can be configured at pipeline creation by specifying
                                                        different values for the constants used in it. This
                                                        is more efficient than configuring the shader using
                                                        variables at render time, because the compiler can do
                                                        optimizations like eliminating if statements that depend
                                                        on these values. */
    return ShaderStageInfo;
}

void Engine::drawFrame() {

    if (Engine::displayGUI) {
        GUIManager::startNewFrame();
    }

    vkWaitForFences(Engine::deviceInfo.logicalDevice, 1, &Engine::inFlightFences[Engine::currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(Engine::deviceInfo.logicalDevice, Engine::swapChainInfo.chain, UINT64_MAX,
        Engine::imageAvailableSemaphores[Engine::currentFrame],
        VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        SwapChainManager::recreateSwapChain();
        framebufferResized = false;
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { //Lacks logic for suboptimal swap chains
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(Engine::deviceInfo.logicalDevice, 1, &Engine::inFlightFences[Engine::currentFrame]);

    Engine::meshMutex.lock();
    Engine::textureMutex.lock();
    Engine::materialMutex.lock();
    Engine::objectCreateMutex.lock();

    Engine::prepareFrameData();
    BufferManager::recordCommandBuffer(Engine::command.buffers[Engine::currentFrame], imageIndex);

    Engine::meshMutex.unlock();
    Engine::textureMutex.unlock();
    Engine::materialMutex.unlock();
    Engine::objectCreateMutex.unlock();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] =
    {
        Engine::imageAvailableSemaphores[Engine::currentFrame]
    };

    VkPipelineStageFlags waitStages[] =
    {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &Engine::command.buffers[Engine::currentFrame];

    VkSemaphore signalSemaphores[] = { Engine::renderFinishedSemaphores[Engine::currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(Engine::queues.graphics, 1, &submitInfo, Engine::inFlightFences[Engine::currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { Engine::swapChainInfo.chain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    result = vkQueuePresentKHR(Engine::queues.present, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || Engine::framebufferResized) {
        SwapChainManager::recreateSwapChain();
        framebufferResized = false;
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    Engine::currentFrame = (Engine::currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    
    #ifndef RELEASE
    Debug::clearDebugData();
    #endif 
}

void Engine::prepareFrameData() {

    #ifndef RELEASE
    if (Debug::lines.size() != 0 && BufferManager::createSSBOCheckSize(Debug::lineSSBO, Debug::lines)) {
        DescriptorManager::createDebugDescriptorPool();
        DescriptorManager::createDebugDescriptorSets();
    }
    #endif
    if (Engine::remakeVertexIndexBuffer) {
        Engine::recreateVertexIndexData();
        BufferManager::createVertexIndexBufferCheckSize(Engine::vertexIndexData.vertices.size() * sizeof(Vertex) + Engine::vertexIndexData.indices.size() * sizeof(uint32_t));
        Engine::remakeVertexIndexBuffer = false;
    }

    if (Engine::remakeInstanceDataSSBO) {

        BufferManager::createSSBOCheckSize(Engine::instanceDataSSBO, Engine::instanceData) ? Engine::remakeDescriptors = true : Engine::remakeDescriptors = false;
    }

    if (Engine::recreateShadowResources) {

        ImageManager::createShadowMapResources();

        DescriptorManager::createShadowLightsPool();
        DescriptorManager::createShadowLightsSets();

        Engine::recreateShadowResources = false;
    }

    if (Engine::remakeDescriptors) {
        DescriptorManager::createGraphicsDescriptorPool();
        DescriptorManager::createGraphicsDescriptorSets();

        Engine::remakeDescriptors = false;
    }

    Engine::updateObjects();
    Engine::updateLightData();

    BufferManager::updateUniformBuffer(Engine::uniformLightBuffer, Engine::lightData);
    BufferManager::updateUniformBuffer(Engine::uniformShadowLightBuffer, Engine::shadowLightData);

    if (!Engine::remakeInstanceDataSSBO) {

        BufferManager::updateSSBO(Engine::instanceDataSSBO, Engine::instanceData);
        return;
    }

    Engine::remakeInstanceDataSSBO = false;
}

void Engine::createSyncObjects() {

    Engine::imageAvailableSemaphores.resize(Engine::MAX_FRAMES_IN_FLIGHT);
    Engine::renderFinishedSemaphores.resize(Engine::MAX_FRAMES_IN_FLIGHT);
    Engine::inFlightFences.resize(Engine::MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < Engine::MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(Engine::deviceInfo.logicalDevice, &semaphoreInfo, nullptr, &Engine::imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(Engine::deviceInfo.logicalDevice, &semaphoreInfo, nullptr, &Engine::renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(Engine::deviceInfo.logicalDevice, &fenceInfo, nullptr, &Engine::inFlightFences[i]) != VK_SUCCESS) {

            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void Engine::recreateVertexIndexData() {

    Engine::vertexIndexData.vertices.clear();
    Engine::vertexIndexData.indices.clear();

    Mesh* mesh = Engine::meshes.head;
    while(mesh!=nullptr) {

        mesh->vertexOffset = (uint32_t)Engine::vertexIndexData.vertices.size();
        mesh->indexOffset = (uint32_t)Engine::vertexIndexData.indices.size();

        for (size_t i = 0; i < mesh->vertices.size(); i++) {
            Engine::vertexIndexData.vertices.push_back(mesh->vertices[i]);
        }

        for (size_t i = 0; i < mesh->indices.size(); i++) {
            Engine::vertexIndexData.indices.push_back(mesh->indices[i]);
        }

        mesh = mesh->next;
    }
}

void Engine::updateObjects() {

    int count = 0;
    GameObject* obj = Engine::gameObjects.head;

    while(obj!=nullptr) 
    {
        obj->update();
        Engine::instanceData[obj->instanceIndex].modelMatrix = obj->transform->getModelMatrix();
        Engine::instanceData[obj->instanceIndex].materialIndex = obj->material->texture->index;
        Engine::instanceData[obj->instanceIndex].hasNormal = obj->hasNormalMap;
        count++;

        obj = obj->next;
    }

}

void Engine::enterFullscreen() {
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwSetWindowMonitor(
        Engine::window,
        monitor,
        0, 0,
        mode->width,
        mode->height,
        mode->refreshRate
    );
}

void Engine::exitFullscreen() {

    glfwSetWindowMonitor(
        Engine::window,
        nullptr,           // <-- NULL monitor = windowed
        200,
        200,
        Engine::WIDTH,
        Engine::HEIGHT,
        0
    );
}

void Engine::updateSampleVectors() {
    Engine::vertexIndexHistory.push_back(static_cast<float>(Engine::vertexIndexBuffer.size) / (1024.0f * 1024.0f));
    if (Engine::vertexIndexHistory.size() > Engine::maxSamples)
        Engine::vertexIndexHistory.erase(Engine::vertexIndexHistory.begin());

    Engine::stagingHistory.push_back(static_cast<float>(Engine::stagingBuffer.size) / (1024.0f * 1024.0f));
    if (Engine::stagingHistory.size() > Engine::maxSamples)
        Engine::stagingHistory.erase(Engine::stagingHistory.begin());

    Engine::instanceDataHistory.push_back(static_cast<float>(Engine::instanceDataSSBO.size) / (1024.0f * 1024.0f));
    if (Engine::instanceDataHistory.size() > Engine::maxSamples)
        Engine::instanceDataHistory.erase(Engine::instanceDataHistory.begin());

    Engine::debugDataHistory.push_back(static_cast<float>(Debug::lineSSBO.size) / (1024.0f * 1024.0f));
    if (Engine::debugDataHistory.size() > Engine::maxSamples)
        Engine::debugDataHistory.erase(Engine::debugDataHistory.begin());
}

float Engine::getMax(std::vector<float>& vector) {

    if (vector.empty()) return 1.0f;
    return *std::max_element(vector.begin(), vector.end());
}

void GarbageQueues::lock() {
    this->mutex.lock();
}

void GarbageQueues::unlock() {
    this->mutex.unlock();
}

void GarbageQueues::update() {
    mutex.lock();

    auto itTex = textures.begin();
    auto itTCount = textureFramesPassed.begin();

    while (itTex != textures.end()) {

        (*itTCount)++;  

        if (*itTCount == Engine::MAX_FRAMES_IN_FLIGHT) {

            if ((*itTex)->image.image != VK_NULL_HANDLE) {
                delete *itTex;
            }

            itTex = textures.erase(itTex);
            itTCount = textureFramesPassed.erase(itTCount);
        }
        else {
            itTex++;
            itTCount++;
        }
    }

    auto itDesc = descriptors.begin();
    auto itDCount = descriptorFramesPassed.begin();

    while (itDesc != descriptors.end()) {

        (*itDCount)++;

        if (*itDCount == Engine::MAX_FRAMES_IN_FLIGHT) {

            if (*itDesc != VK_NULL_HANDLE) {
                vkDestroyDescriptorPool(Engine::deviceInfo.logicalDevice, *itDesc, nullptr);
            }

            itDesc = descriptors.erase(itDesc);
            itDCount = descriptorFramesPassed.erase(itDCount);
        }
        else {
            itDesc++;
            itDCount++;
        }
    }

    auto itBuff = buffers.begin();
    auto itBCount = bufferFramesPassed.begin();

    while (itBuff != buffers.end()) {

        (*itBCount)++;

        if (*itBCount == Engine::MAX_FRAMES_IN_FLIGHT) {

            if (itBuff->buffer != VK_NULL_HANDLE) {
                itBuff->destroy();
            }

            itBuff = buffers.erase(itBuff);
            itBCount = bufferFramesPassed.erase(itBCount);
        }
        else {
            itBuff++;
            itBCount++;
        }
    }

    auto itImgVec = imageVectors.begin();
    auto itIVCount = imageVectorFramesPassed.begin();

    while (itImgVec != imageVectors.end()) {

        (*itIVCount)++;

        if (*itIVCount == Engine::MAX_FRAMES_IN_FLIGHT) {

            if ((*itImgVec)->images.size() != 0) {
                (*itImgVec)->destroyAllItems();
            }

            delete* itImgVec;

            itImgVec = imageVectors.erase(itImgVec);
            itIVCount = imageVectorFramesPassed.erase(itIVCount);
        }
        else {
            itImgVec++;
            itIVCount++;
        }
    }

    mutex.unlock();

}

void Engine::createUpdateGarbageJob() {

    UpdateGarbageJob* job = new UpdateGarbageJob();

    Engine::jobQueueMutex.lock();
    Engine::jobQueue.push(job);
    Engine::jobQueueMutex.unlock();

    Engine::jobInQueueSem.release();
}

void Engine::updateLightData() {

    Light* light = Engine::lights.head;

    for (size_t i = 0; i < Engine::lights.size; i++) {

        light->update();

        Engine::lightData.positions[i] = light->position;
        Engine::lightData.colors[i] = light->color;
        Engine::lightData.ambientLightColors[i] = light->ambientLightColor;
        Engine::lightData.intensities[i] = glm::vec4(light->intensity);
        Engine::lightData.lightVPs[i] = light->getLightVP();

        size_t idx = i / 4;        // which uint32_t
        uint32_t bytePos = i % 4;    // which byte in that uint32_t
        uint32_t shift = 8 * (3 - bytePos);
        uint32_t mask = 0xFFu << shift;

        uint32_t  uvecIndex = static_cast<uint32_t>(idx) / 4;      // which uvec4
        uint32_t  uvecElement = idx % 4;    // which uint inside that uvec4

        Engine::lightData.types[uvecIndex][uvecElement] =
            (Engine::lightData.types[uvecIndex][uvecElement] & ~mask) |
            ((uint32_t)light->type << shift);

        light = light->next;
    }

    Engine::lightData.lightCount = static_cast<uint32_t>(Engine::lights.size);

    light = Engine::shadowCreatingLights.head;

    for (size_t i = 0; i < Engine::shadowCreatingLights.size; i++) {

        light->update();

        Engine::shadowLightData.positions[i] = light->position;
        Engine::shadowLightData.colors[i] = light->color;
        Engine::shadowLightData.ambientLightColors[i] = light->ambientLightColor;
        Engine::shadowLightData.intensities[i] = glm::vec4(light->intensity);
        Engine::shadowLightData.lightVPs[i] = light->getLightVP();

        size_t  idx = i / 4;        // which uint32_t
        uint32_t bytePos = i % 4;    // which byte in that uint32_t
        uint32_t shift = 8 * (3 - bytePos);
        uint32_t mask = 0xFFu << shift;

        uint32_t  uvecIndex = static_cast<uint32_t>(idx) / 4;      // which uvec4
        uint32_t  uvecElement = idx % 4;    // which uint inside that uvec4

        Engine::shadowLightData.types[uvecIndex][uvecElement] =
            (Engine::shadowLightData.types[uvecIndex][uvecElement] & ~mask) |
            ((uint32_t)light->type << shift);

        Engine::shadowLightData.shadowMapIndices[uvecIndex][uvecElement] =
            (Engine::shadowLightData.shadowMapIndices[uvecIndex][uvecElement] & ~mask) |
            ((uint32_t)i << shift);

        light = light->next;
    }

    Engine::shadowLightData.lightCount = static_cast<uint32_t>(Engine::shadowCreatingLights.size);

}

void Engine::loadDemoScene() {

    DirectionalLight* sun = new DirectionalLight(glm::vec4(-10.0f), glm::vec4(COLOR_WHITE, 10.0f), 2.0f);

    ImageManager::createSolidColorFilePNG("brown.png", 92, 64, 51);

    GameObject* floor = new GameObject();
    InitInfo* fInfo = new InitInfo("cube.obj", nullptr, "brown.png", nullptr, "dirt_normal.jpg");
    floor->transform->position = glm::vec3(10.0f, -0.5f, 10.0f);
    floor->scale(glm::vec3(20.0f, 0.5f, 20.0f));
    GameObject::createInitiasationJob(floor, fInfo);
    delete fInfo;

    GameObject* barrels = new GameObject();
    InitInfo* bInfo = new InitInfo("seven_barrels.obj", nullptr, "barrel_basecolor.png", nullptr, "barrel_normal.png");
    barrels->transform->position = glm::vec3(20.0f, 0.0f, 20.0f);
    barrels->scale(glm::vec3(0.03f));
    GameObject::createInitiasationJob(barrels, bInfo);
    delete bInfo;

    GameObject* barrel = new GameObject();
    InitInfo* b2Info = new InitInfo("Medieval_Barrels.obj", nullptr, "barrel_basecolor.png", nullptr, "barrel_normal.png");
    barrel->transform->position = glm::vec3(25.0f, 0.0f, 25.0f);
    barrel->scale(glm::vec3(0.03f));
    GameObject::createInitiasationJob(barrel, b2Info);
    delete b2Info;

    GameObject* house = new GameObject();
    InitInfo* hInfo = new InitInfo("Cottage_FREE.obj", nullptr, "Cottage_Clean_Base_Color.png", nullptr, "Cottage_Clean_Normal.png");
    house->transform->position = glm::vec3(5.0f, 0.0f, 5.0f);
    house->scale(glm::vec3(2.0f));
    house->rotate(glm::angleAxis(glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    GameObject::createInitiasationJob(house, hInfo);
    delete hInfo;
}