#ifndef IM_VULKAN_H
#define IM_VULKAN_H

#include "PREFIX.h"

extern uint32_t SCREENW;
extern uint32_t SCREENH;

/// Reference to the main application window created by GLFW.
extern GLFWwindow * window;
/// Vulkan (extension) handle to the representation of the GLFW created window.
extern VkSurfaceKHR surface;

/// Handle to our Vulkan instance. This is the connection between
/// this application and the Vulkan library.
extern VkInstance instance;
/// Handle to our selected physical device.
/// This will be selected based on a number of parameters,
/// such as desired supported Queue Families.
extern VkPhysicalDevice physicalDevice;
/// Logical device created from our selected physical device.
extern VkDevice device;

/// Stores supported swap chain details for a given physical device.
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

/// Stores whether or not the physical device supports desired queue families.
struct QueueFamilyIndices {
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool IsComplete() {
		return (graphicsFamily >= 0) && (presentFamily >= 0);
	}
};

/// Find the queue family indecies for a given surface and physical device,
/// this need not necessarily be the selected surface/physical device declared above.
QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice &pDevice, VkSurfaceKHR &surface);

/// Query swap chain support for the given physical device.
SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice &pDevice);

/// Find a memory type that fits the input needs for our physical device.
uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

/// Create a new buffer on the GPU using the given ubuffer usage and memory properties.
void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
		VkMemoryPropertyFlags properties, 
		VkBuffer &buffer, VkDeviceMemory &bufferMemory);

#endif
