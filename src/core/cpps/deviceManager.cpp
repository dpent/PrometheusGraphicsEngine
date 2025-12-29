#include "../headers/deviceManager.h"
#include "../headers/engine.h"

void DeviceManager::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(Engine::vkInstanceInfo.instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(Engine::vkInstanceInfo.instance, &deviceCount, devices.data());

    std::vector<std::pair<int, VkPhysicalDevice>> candidates;

    std::cout << "Rating available devices..." << std::endl;

    for (const auto& device : devices) {
        int score = rateDeviceSuitability(device);
        if (score > 0) {  // Only add suitable devices
            candidates.push_back(std::make_pair(score, device));
        }
    }

    if (candidates.empty()) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    std::sort(candidates.begin(), candidates.end(),
        [](const auto& a, const auto& b) { return a.first > b.first; });
    // Select the highest scoring device
    Engine::deviceInfo.physicalDevice = candidates[0].second;

    vkGetPhysicalDeviceProperties(Engine::deviceInfo.physicalDevice, &Engine::deviceInfo.physicalProperties);
    vkGetPhysicalDeviceFeatures(Engine::deviceInfo.physicalDevice, &Engine::deviceInfo.physicalFeatures);
    vkGetPhysicalDeviceMemoryProperties(Engine::deviceInfo.physicalDevice, &Engine::deviceInfo.physicalMemProperties);

    Engine::msaaSamples = Engine::getMaxUsableSampleCount();

    std::cout << "=======================================" << std::endl;
    std::cout << "Selected GPU: " << Engine::deviceInfo.physicalProperties.deviceName << std::endl;
    std::cout << "  Driver version: " << Engine::deviceInfo.physicalProperties.driverVersion << std::endl;
    std::cout << "  Device Type: " << Engine::deviceInfo.physicalProperties.deviceType << " (" << DeviceManager::deviceTypeToString(Engine::deviceInfo.physicalProperties.deviceType) << ")" << std::endl;
    std::cout << "  Vendor ID: " << Engine::deviceInfo.physicalProperties.vendorID << " (" << DeviceManager::vendorIdToString(Engine::deviceInfo.physicalProperties.vendorID) << ")" << std::endl;
    std::cout << "  Device ID: " << Engine::deviceInfo.physicalProperties.deviceID << std::endl;
    std::cout << "  MSSAx" << Engine::msaaSamples << std::endl;
}

int DeviceManager::rateDeviceSuitability(const VkPhysicalDevice& device) {

    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    QueueFamilyIndices indices = QueueFamilyIndices::findQueueFamilies(device, Engine::surface);

    bool extensionsSupported = DeviceManager::checkDeviceExtensionSupport(device);

    int score = 0;

    if (indices.isComplete()) {
        score += 50;
    }

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = SwapChainSupportDetails::querySwapChainSupport(device, Engine::surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    if (!deviceFeatures.samplerAnisotropy) {
        score -= 50;
    }

    // Application can't function without geometry shaders
    if (!deviceFeatures.geometryShader || !extensionsSupported || !swapChainAdequate) {
        return 0;
    }

    /*std::cout<<"Evaluating device: "<<deviceProperties.deviceName<<std::endl;
    std::cout<<"  Device Type: "<<deviceProperties.deviceType<<" ("<<DeviceManager::deviceTypeToString(deviceProperties.deviceType)<<")"<<std::endl;
    std::cout<<"  Vendor ID: "<<deviceProperties.vendorID<<" ("<<DeviceManager::vendorIdToString(deviceProperties.vendorID)<<")"<<std::endl;
    std::cout<<"  Final score: "<<score<<"\n"<<std::endl;*/

    return score;
}

bool DeviceManager::checkDeviceExtensionSupport(const VkPhysicalDevice& device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(Engine::deviceExtensions.begin(), Engine::deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

const char* DeviceManager::deviceTypeToString(VkPhysicalDeviceType& deviceType) {
    switch (deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        return "Other";
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return "Integrated GPU";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return "Discrete GPU";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        return "Virtual GPU";
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return "CPU/Software Renderer";
    default:
        return "Unknown";
    }
}

const char* DeviceManager::vendorIdToString(uint32_t& vendorId) {
    switch (vendorId) {
    case 0x10DE:
        return "NVIDIA";
    case 0x1002:
        return "AMD";
    case 0x8086:
        return "Intel";
    case 0x10005:
        return "Mesa/Software";
    default:
        return "Unknown";
    }
}

void DeviceManager::createLogicalDevice() {
    QueueFamilyIndices indices = QueueFamilyIndices::findQueueFamilies(Engine::deviceInfo.physicalDevice, Engine::surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};//TODO FILL THE FIELDS RIGHT NOW ALL ARE FALSE
    if (!Engine::deviceInfo.physicalFeatures.samplerAnisotropy) {
        deviceFeatures.samplerAnisotropy = VK_FALSE;
    }
    else {
        deviceFeatures.samplerAnisotropy = VK_TRUE;
    } //We need this if we use it
    //deviceFeatures.sampleRateShading = VK_TRUE; //Enable sample shading feature for the device

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(Engine::deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = Engine::deviceExtensions.data();

    if (InstanceManager::enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(InstanceManager::validationLayers.size());
        createInfo.ppEnabledLayerNames = InstanceManager::validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(Engine::deviceInfo.physicalDevice, &createInfo, nullptr, &Engine::deviceInfo.logicalDevice) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(Engine::deviceInfo.logicalDevice, indices.graphicsFamily.value(), 0, &Engine::queues.graphics); //The 0 means we get the 0th queue from each family
    vkGetDeviceQueue(Engine::deviceInfo.logicalDevice, indices.presentFamily.value(), 0, &Engine::queues.present);
    vkGetDeviceQueue(Engine::deviceInfo.logicalDevice, indices.computeFamily.value(), 0, &Engine::queues.compute);
}

uint32_t DeviceManager::findMemoryType(uint32_t& typeFilter, VkMemoryPropertyFlags properties) {

    for (uint32_t i = 0; i < Engine::deviceInfo.physicalMemProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (Engine::deviceInfo.physicalMemProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
}