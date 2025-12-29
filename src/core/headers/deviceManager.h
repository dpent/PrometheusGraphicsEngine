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

    static void createLogicalDevice();

    static const char* deviceTypeToString(VkPhysicalDeviceType& deviceType);
    static const char* vendorIdToString(uint32_t& vendorId);

	static uint32_t findMemoryType(uint32_t& typeFilter, VkMemoryPropertyFlags properties);
};

struct DeviceInfo {
public:
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;

	VkPhysicalDeviceProperties physicalProperties;
	VkPhysicalDeviceMemoryProperties physicalMemProperties;
	VkPhysicalDeviceFeatures physicalFeatures;
};