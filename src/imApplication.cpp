#include "imApplication.h"
#include "VKBuilder.hpp"

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
		Update();
		// Validation layers require GPU/CPU synchronization.
		// Synchronize after running update code, rather then before,
		// that way can delay synchronization as long as possible.
		if (VALIDATION_LAYERS_ENABLED) { vkDeviceWaitIdle(device); }
		DrawFrame();
	}

	vkDeviceWaitIdle(device);
}

void imApplication::Update() {
	// TODO
}

void imApplication::DrawFrame() {
	uint32_t imageIndex;
	vkAcquireNextImageKHR(device, swapchain.swapChain, 
		std::numeric_limits<uint64_t>::max(),
		imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	VkSubmitInfo submitInfo = { };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	// If we need, we can wait for more than one semaphore to become available.
	VkSemaphore waitSemaphores[] = {
		imageAvailableSemaphore
	};
	VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};

	// Wait for an image before writing to the color attachment.
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	// We only have a single command buffer to run.
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
	// Unlock the render finished semaphore once our commands have all finished.
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit draw command buffer!");
	}

	// --- Presentation ---
	
	VkPresentInfoKHR presentInfo = { };
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapchain.swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	vkQueuePresentKHR(presentQueue, &presentInfo);
}

void imApplication::InitGLFW(size_t screen_w, size_t screen_h, const char * app_name) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(screen_w, screen_h, app_name, nullptr, nullptr);
}

void imApplication::InitVulkan() {
	// Initial Setup - Create instance, debug, and select/create devices.
	VKBuilder::CreateInstance();
	VKBuilder::PrintSupportedExtensions();
	VKBuilder::CreateSurface();
	VKDebug::SetupDebugCallback(callback);
	VKBuilder::SelectPhysicalDevice();
	VKBuilder::CreateLogicalDevice(graphicsQueue, presentQueue);

	// Setup the swap chain and graphics pipeline.
	swapchain.CreateSwapChain();
	swapchain.CreateImageViews();
	pipeline.CreateRenderPass(swapchain.imageFormat);
	pipeline.CreateGraphicsPipeline(swapchain.extent, 
		"shaders/vert.spv", "shaders/frag.spv");
	swapchain.CreateFrameBuffers(pipeline.renderPass);

	// Create the command buffers for submitting commands.
	VKBuilder::CreateCommandPoool(commandPool);
	VKBuilder::CreateCommandBuffers(commandPool, pipeline, swapchain, commandBuffers);
	InitSemaphores();
}

void imApplication::InitSemaphores() {
	VkSemaphoreCreateInfo semaphoreInfo = { };
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) 
			!= VK_SUCCESS || 
		vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) 
			!= VK_SUCCESS) {
		throw std::runtime_error("Failed to create semaphores!");
	}
}

void imApplication::Cleanup() {
	// Vulkan
	
	vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
	vkDestroyCommandPool(device, commandPool, nullptr);
	
	pipeline.Cleanup();
	swapchain.Cleanup();
	
	if (VALIDATION_LAYERS_ENABLED) {
		VKDebug::DestroyDebugReportCallbackEXT(callback, nullptr);
	}

	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);

	// GLFW
	glfwDestroyWindow(window);
	glfwTerminate();
}
