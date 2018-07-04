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
		UpdateUniformBuffer();
		DrawFrame();
	}

	vkDeviceWaitIdle(device);
}

void imApplication::Update() {
	// TODO
}

void imApplication::UpdateUniformBuffer() {
	// Compute the total runtime of this application.
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(
		currentTime - startTime).count();

	// Next start generating our MVP matrices.
	UniformBufferObject ubo = { };
	// rotate the identity, at 90 degrees/second, about the positive z axis
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
		glm::vec3(0.0f, 0.0f, 1.0f));
	// look position, camera position, up vector
	ubo.view = glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f));
	// projection matrix with 45 degrees of FOV, swap chain aspect ratio, near
	// plan and far plane distances.
	ubo.proj = glm::perspective(glm::radians(45.0f), swapchain.extent.width /
		(float)swapchain.extent.height, 0.1f, 10.0f);
	// OpenGL -> Vulkan space conversion.
	ubo.proj[1][1] *= -1;

	// Now we can transfer this data to the GPU.
	void * data;
	vkMapMemory(device, uniformBufferMemory, 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, uniformBufferMemory);
}

void imApplication::DrawFrame() {
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapchain.swapChain, 
		std::numeric_limits<uint64_t>::max(),
		imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	// Check if Vulkan thiks we need to recreate our swap chain.
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		RecreateSwapChain();
		return;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

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

	result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		RecreateSwapChain();
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swap chain image!");
	}
}

void imApplication::InitGLFW(size_t screen_w, size_t screen_h, const char * app_name) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(screen_w, screen_h, app_name, nullptr, nullptr);

	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, imApplication::OnWindowResized);
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
	VKBuilder::CreateDescriptorSetLayout(descriptorSetLayout);
	pipeline.CreateGraphicsPipeline(swapchain.extent, 
		"shaders/vert.spv", "shaders/frag.spv", descriptorSetLayout);

	// Create the command buffers for submitting commands.
	VKBuilder::CreateCommandPoool(commandPool);
	mesh.Create();
	swapchain.CreateDepthBuffer();
	swapchain.CreateFrameBuffers(pipeline.renderPass);
	image.Create("tex/caco.png");
	VKBuilder::CreateUniformBuffer(uniformBuffer, uniformBufferMemory);
	VKBuilder::CreateDescriptorPool(descriptorPool);
	VKBuilder::CreateDescriptorSet(descriptorPool, descriptorSet, 
		uniformBuffer, descriptorSetLayout, image);
	CreateCommandBuffers();
	InitSemaphores();
}

void imApplication::CleanupSwapChain() {
	vkFreeCommandBuffers(device, commandPool, 
		static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	pipeline.Cleanup();
	swapchain.Cleanup();
}

void imApplication::RecreateSwapChain() {
	vkDeviceWaitIdle(device);

	std::cout << "Recreating Swap Chain" << std::endl;
	std::cout << "-----------------------------------------------" << std::endl;
	CleanupSwapChain();

	swapchain.CreateSwapChain();
	swapchain.CreateImageViews();
	pipeline.CreateRenderPass(swapchain.imageFormat);
	pipeline.CreateGraphicsPipeline(swapchain.extent, 
		"shaders/vert.spv", "shaders/frag.spv", descriptorSetLayout);
	swapchain.CreateDepthBuffer();
	swapchain.CreateFrameBuffers(pipeline.renderPass);
	CreateCommandBuffers();
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
	
	CleanupSwapChain();
	mesh.Cleanup();
	image.Cleanup();

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	vkDestroyBuffer(device, uniformBuffer, nullptr);
	vkFreeMemory(device, uniformBufferMemory, nullptr);
	
	vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
	vkDestroyCommandPool(device, commandPool, nullptr);
	
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

void imApplication::OnWindowResized(GLFWwindow * window, int width, int height) {
	if (window == 0 || height == 0) { return; }

	// Update our global constants for the screen size.
	SCREENH = width;
	SCREENW = width;

	imApplication * app = reinterpret_cast<imApplication *>(
		glfwGetWindowUserPointer(window));
	app->RecreateSwapChain();
}

void imApplication::CreateCommandBuffers() {
	commandBuffers.resize(swapchain.frameBuffers.size());

	VkCommandBufferAllocateInfo allocInfo = { };
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) 
			!= VK_SUCCESS) {
		throw std::runtime_error("Failed to create command buffers!");
	}

	for (size_t i = 0; i < commandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = { };
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr; // Optional

		// Begin recording to the command buffer (implicitly reset buffer).
		vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

		VkRenderPassBeginInfo renderPassInfo = { };
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = pipeline.renderPass;
		renderPassInfo.framebuffer = swapchain.frameBuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapchain.extent;

		std::array<VkClearValue, 2> clearValues;
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());;
		renderPassInfo.pClearValues = clearValues.data();

		// Begin the render pass, can now submit drawing commands.
		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, 
			VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, 
			pipeline.pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
		vkCmdBindPipeline(commandBuffers[i], 
			VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.graphicsPipeline);

		// Bind the vertex buffer for rendering.
		VkBuffer vertexBuffers[] = { mesh.vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffers[i], mesh.indexBuffer, 
			0, VK_INDEX_TYPE_UINT16);

		vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(INDICES.size()),
			1, 0, 0, 0);

		// End the render pass, stop submitting draw commands.
		vkCmdEndRenderPass(commandBuffers[i]);

		// Stop recording to the command buffer.
		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer!");
		}
	}
}
