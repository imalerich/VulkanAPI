#include "imApplication.h"

#include "VKBuilder.hpp"
#include "VKSurfaceBuilder.hpp"

imApplication::imApplication(size_t screen_w, size_t screen_h, const char * app_name) {
	InitGLFW(screen_w, screen_h, app_name);
	InitVulkan();
}

imApplication::~imApplication() {
	Cleanup();
}

void imApplication::Run() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
}

void imApplication::InitGLFW(size_t screen_w, size_t screen_h, const char * app_name) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(screen_w, screen_h, app_name, nullptr, nullptr);
}

void imApplication::InitVulkan() {
	VKBuilder::CreateInstance(instance);
	VKBuilder::PrintSupportedExtensions();
	VKBuilder::CreateSurface(instance, window, surface);
	VKDebug::SetupDebugCallback(instance, callback);
	VKBuilder::SelectPhysicalDevice(instance, surface, physicalDevice);
	VKBuilder::CreateLogicalDevice(physicalDevice, surface, 
		device, graphicsQueue, presentQueue);
	VKSurfaceBuilder::CreateSwapChain(physicalDevice, device, surface, 
		swapChain, VKBuilder::FindQueueFamilies(physicalDevice, surface),
		swapChainImages, swapChainImageFormat, swapChainExtent);
	VKBuilder::CreateImageViews(swapChainImages, swapChainImageViews, 
		swapChainImageFormat, device);
}

void imApplication::Cleanup() {
	// Vulkan
	if (VALIDATION_LAYERS_ENABLED) {
		VKDebug::DestroyDebugReportCallbackEXT(instance, callback, nullptr);
	}

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		vkDestroyImageView(device, swapChainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);

	// GLFW
	glfwDestroyWindow(window);
	glfwTerminate();
}
