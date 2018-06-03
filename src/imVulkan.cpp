#include "imVulkan.h"

uint32_t SCREENW = 800;
uint32_t SCREENH = 600;

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

uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i))
				&& ((memProperties.memoryTypes[i].propertyFlags & properties) 
				== properties)) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}

void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
		VkMemoryPropertyFlags properties, 
		VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
	VkBufferCreateInfo bufferInfo = { };
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer!");
	}

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(device, buffer, &memReqs);

	VkMemoryAllocateInfo allocInfo = { };
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, 
		properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory!");
	}

	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}
