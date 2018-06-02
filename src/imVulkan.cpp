#include "imVulkan.h"

GLFWwindow * window = nullptr;
VkSurfaceKHR surface = nullptr;

VkInstance instance = VK_NULL_HANDLE;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkDevice device = VK_NULL_HANDLE;

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice &pDevice, VkSurfaceKHR &surface) {
	QueueFamilyIndices indicies;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, 
		queueFamilies.data());

	int i = 0;
	// Iterate all queue families searching for one
	// with the desired capability bit (e.g. graphics) set.
	for (const auto &queueFamily : queueFamilies) {
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, i, surface, &presentSupport);

		if (queueFamily.queueCount > 0 && presentSupport) {
			indicies.presentFamily = i;
		}

		if (queueFamily.queueCount > 0 && 
				(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
			indicies.graphicsFamily = i;
		}

		i++;
	}

	return indicies;
}

SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice &pDevice) {

	SwapChainSupportDetails details;

	// Surface Capabilites (# buffers, min/max sizes, etc).
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice, surface, 
		&details.capabilities);

	// Surface Formats (RGB, 8/16/32bit etc).
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, 
			&formatCount, details.formats.data());
	}

	// Presentation Modes.
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface, 
		&presentModeCount, nullptr);

	if (formatCount != 0) {
		details.presentModes.resize(formatCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface, 
			&presentModeCount, details.presentModes.data());
	}

	return details;
}
