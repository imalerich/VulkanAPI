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
/// Manages the memory of allocated command buffers.
extern VkCommandPool commandPool;

/// Handle to the graphics queue for sending graphics command pools.
extern VkQueue graphicsQueue;
/// Handle to the presentation queue for presenting to the GLFW window.
extern VkQueue presentQueue;


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

/// True if the input format has a stencil component.
bool HasStencilComponent(VkFormat format);
/// Choose the best format available to use for the depth buffer.
VkFormat FindDepthFormat();
/// Find available formats to use when creating the depth buffer.
VkFormat FindSupportedFormat(const std::vector<VkFormat> &candidates,
	VkImageTiling tiling, VkFormatFeatureFlags features);

/// Describes a buffer of data we want to pass globally to our shaders.
struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

/// Create a command bufer which we only wish to submit once.
VkCommandBuffer BeginSingleTimeCommands();
/// Submit, wait, then free the input command buffer.
void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

#endif
