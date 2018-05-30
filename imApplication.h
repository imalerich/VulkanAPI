#ifndef IM_APPLICATION_H
#define IM_APPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <functional>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cstring>
#include <limits>
#include <vector>
#include <set>

#include "PREFIX.h"
#include "VKPipeline.h"

class imApplication {
public:
	/**
	 * Constructs a new vulkan application with the given window settings.
	 * \param screen_w Horizontal (Width) dimension of the main screen.
	 * \param screen_h Vertical (Height) dimension of the main screen.
	 * \param app_name Window Title to associate with the application.
	 */
	imApplication(size_t screen_w, size_t screen_h, const char * app_name);
	/**
	 * Releases all GLFW and Vulkan assets.
	 */
	~imApplication();

	/**
	 * Main game loop.
	 */
	void Run();

private:
	void InitGLFW(size_t screen_w, size_t screen_h, const char * app_name);
	void InitVulkan();
	void Cleanup();

	/// Reference to the main application window created by GLFW.
	GLFWwindow * window;
	/// Handle to our Vulkan instance. This is the connection between
	/// this application and the Vulkan library.
	VkInstance instance = VK_NULL_HANDLE;
	/// Handle to our selected physical device.
	/// This will be selected based on a number of parameters,
	/// such as desired supported Queue Families.
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	/// Logical device created from our selected physical device.
	VkDevice device = VK_NULL_HANDLE;

	/// Handle to the graphics queue for sending graphics command pools.
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	/// Handle to the presentation queue for presenting to the GLFW window.
	VkQueue presentQueue = VK_NULL_HANDLE;

	/// Vulkan (extension) handle to the representation of the GLFW created window.
	VkSurfaceKHR surface;
	/// Holds all of our render targets, we'll be aiming for tripple buffering.
	VkSwapchainKHR swapChain;
	/// Handles to each image contained within our swap chain.
	std::vector<VkImage> swapChainImages;
	/// Views into each image of the swap chain.
	/// This relationship is similar to the idea of having both a physical
	/// device and a logical device to interface with that physical device.
	std::vector<VkImageView> swapChainImageViews;

	/// Image format that was used to create the swap chain, useful to keep around.
	VkFormat swapChainImageFormat;
	/// Extent of the swap chain images used to create the swap chain.
	VkExtent2D swapChainExtent;

	/// Will hold a basic configuration for our graphics pipeline.
	VKPipeline pipeline;

	/// Handle to validation layers debug callback.
	VkDebugReportCallbackEXT callback;
};

#endif
