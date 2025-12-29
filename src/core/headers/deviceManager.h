#pragma once

#include "Prometheus.h"
#include "queueInfo.h"
#include "swapChainManager.h"

class Engine;

class DeviceManager {
public:
	static void pickPhysicalDevice();
	static int rateDeviceSuitability(const VkPhysicalDevice& device);

    static bool checkDeviceExtensionSupport(const VkPhysicalDevice& device);

    //Logical device
    static void createLogicalDevice();

    static const char* deviceTypeToString(VkPhysicalDeviceType& deviceType);
    static const char* vendorIdToString(uint32_t& vendorId);
};

struct DeviceInfo {
public:
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;

	VkPhysicalDeviceProperties physicalProperties;
	VkPhysicalDeviceFeatures physicalFeatures;
};