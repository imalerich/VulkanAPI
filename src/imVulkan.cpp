#include "imVulkan.h"

uint32_t SCREENW = 800;
uint32_t SCREENH = 600;

GLFWwindow * window = nullptr;
VkSurfaceKHR surface = nullptr;

VkInstance instance = VK_NULL_HANDLE;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkDevice device = VK_NULL_HANDLE;
VkCommandPool commandPool = VK_NULL_HANDLE;
VkQueue graphicsQueue = VK_NULL_HANDLE;
VkQueue presentQueue = VK_NULL_HANDLE;

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

bool HasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat FindSupportedFormat(const std::vector<VkFormat> &candidates,
		VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && 
				(props.linearTilingFeatures & features) == features) {
			return format;
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
				(props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}
	
	throw std::runtime_error("Failed to find a supported depth buffer format!");
}

VkFormat FindDepthFormat() {
	return FindSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

VkCommandBuffer BeginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}
