#ifndef VK_BUILDER_HPP
#define VK_BUILDER_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

#include "PREFIX.h"
#include "VKDebug.hpp"
#include "VKSurfaceBuilder.hpp"

class VKBuilder {
public:

	static void CreateInstance(VkInstance &instance) {
		if (VALIDATION_LAYERS_ENABLED && !CheckValidationLayerSupport()) {
			throw std::runtime_error("Validation Layers requested, but not available!");
		}

		VkApplicationInfo appInfo = { };
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = { };
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// Get and set the required extensions for GLFW to work with Vulkan.
		auto extensions = GetRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (VALIDATION_LAYERS_ENABLED) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
			createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("Failed to Create Instance!");
		}
	}

	static void PrintSupportedExtensions() {
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		std::cout << "Available Extensions: " << std::endl;
		for (const auto &extension : extensions) {
			std::cout << "\t- " << extension.extensionName << std::endl;
		}

		std::cout << "-----------------------------------------------" << std::endl;
	}

	static void CreateSurface(VkInstance &instance, 
			GLFWwindow * window, VkSurfaceKHR &surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface!");
		}
	}

	static void SelectPhysicalDevice(VkInstance &instance, 
			VkSurfaceKHR &surface, VkPhysicalDevice &pDevice) {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		// Create a set of containing all suitable devices for our application.
		std::set<VkPhysicalDevice> suitableDevices;
		for (const auto &d : devices) {
			// Print some information about each vulkan supported device.
			VkPhysicalDeviceProperties prop;
			vkGetPhysicalDeviceProperties(d, &prop);
			std::cout << "Device Found: " << prop.deviceName << std::endl;
			std::cout << "\t- API Version: " << prop.apiVersion << std::endl;
			std::cout << "\t- Device Type: " << prop.deviceType << std::endl;

			if (IsDeviceSuitable(d, surface)) {
				suitableDevices.insert(d);
				break;
			}

			std::cout << "-----------------------------------------------" << std::endl;
		}

		// Some interesting debug output about the devices we found.
		std::cout << "Found " << deviceCount << " Device" 
			<< (deviceCount == 1 ? "" : "s")
			<< "!" << std::endl;
		std::cout << suitableDevices.size() << " of them "
			<< ((suitableDevices.size() == 1) ? "is" : "are")
			<< " suitable." << std::endl;
		std::cout << "-----------------------------------------------" << std::endl;

		// Don't really care right now, could provide an interface for the
		// user to select their preferred device by presenting each deviceName.
		pDevice = *suitableDevices.begin();
		if (pDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("Failed to find a suitable GPU!");
		}
	}

	static bool IsDeviceSuitable(VkPhysicalDevice pDevice, VkSurfaceKHR &surface) {
		QueueFamilyIndices indices = FindQueueFamilies(pDevice, surface);
		bool extensionsSupported = CheckDeviceExtensionsSupport(pDevice, surface);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails support = VKSurfaceBuilder::QuerySwapChainSupport(
				pDevice, surface);
			swapChainAdequate = !support.formats.empty() && !support.presentModes.empty();
		}

		return indices.IsComplete() && swapChainAdequate && extensionsSupported;
	}

	static bool CheckDeviceExtensionsSupport(
			VkPhysicalDevice &pDevice, VkSurfaceKHR &surface) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(pDevice, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(pDevice, nullptr, 
			&extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(
			DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());
		for (const auto &extension : availableExtensions) {
			std::cout << "\t- Found extension " << extension.extensionName << 
				" for device." << std::endl;
			requiredExtensions.erase(extension.extensionName);
		}
		std::cout << "-----------------------------------------------" << std::endl;

		return requiredExtensions.empty();
	}

	static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice pDevice, 
			VkSurfaceKHR &surface) {
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

	static void CreateLogicalDevice(VkPhysicalDevice &pDevice, VkSurfaceKHR &surface,
			VkDevice &device, VkQueue &gQueue, VkQueue &pQueue) {
		QueueFamilyIndices indices = FindQueueFamilies(pDevice, surface);
		float queuePriority = 1.0f;

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int> uniqueQueueFamilies = { 
			indices.graphicsFamily, indices.presentFamily 
		};

		// Need to create a graphics and a presentation queue.
		for (int queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = { };
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// Any additional features we need (e.g. Geometry Shaders).
		VkPhysicalDeviceFeatures deviceFeatures = { };

		VkDeviceCreateInfo createInfo = { };
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
		createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

		if (VALIDATION_LAYERS_ENABLED) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
			createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(pDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("Falide to create logical device!");
		}

		vkGetDeviceQueue(device, indices.graphicsFamily, 0, &gQueue);
		vkGetDeviceQueue(device, indices.presentFamily, 0, &pQueue);
	}

	static bool CheckValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char * layerName : VALIDATION_LAYERS) {
			bool layerFound = false;
			for (const auto & layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) { return false; }
		}

		return true;
	}

	static std::vector<const char *> GetRequiredExtensions() {
		std::vector<const char *> extensions;

		unsigned glfwExtensionCount = 0;
		const char ** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		for (unsigned i = 0; i < glfwExtensionCount; i++) {
			extensions.push_back(glfwExtensions[i]);
		}

		if (VALIDATION_LAYERS_ENABLED) {
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}

		return extensions;
	}

	static void CreateImageViews(
			std::vector<VkImage> &images,
			std::vector<VkImageView> &imageViews,
			VkFormat format, VkDevice &device) {
		imageViews.resize(images.size());

		for (size_t i = 0; i < images.size(); i++) {
			VkImageViewCreateInfo createInfo = { };
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = images[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = format;

			// Can move color channels around as we please, or set
			// them to constant values, here we'll just use the default.
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device, &createInfo, nullptr, &imageViews[i]) 
					!= VK_SUCCESS) {
				throw std::runtime_error("Failed to create image views!");
			}
		} 
	}

	static void CreateFrameBuffers(VkDevice &device,
			std::vector<VkFramebuffer> &frameBuffers,
			std::vector<VkImageView> &imageViews,
			VkRenderPass &renderPass, VkExtent2D extent) {

		frameBuffers.resize(imageViews.size());
		for (size_t i = 0; i < imageViews.size(); i++) {
			// We only have a color attachment, but we could
			// also include a depth attachment here.
			VkImageView attachments[] = {
				imageViews[i]
			};

			VkFramebufferCreateInfo fbInfo = { };
			fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fbInfo.renderPass = renderPass;
			fbInfo.attachmentCount = 1;
			fbInfo.pAttachments = attachments;
			fbInfo.width = extent.width;
			fbInfo.height = extent.height;
			fbInfo.layers = 1;

			if (vkCreateFramebuffer(device, &fbInfo, nullptr, &frameBuffers[i]) 
					!= VK_SUCCESS) {
				throw std::runtime_error("Failed to create frame buffer!");
			}
		}
	}

	static void CreateCommandPoool(
			VkDevice &device,
			VkPhysicalDevice &pDevice, VkSurfaceKHR &surface,
			VkCommandPool &pool) {
		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(pDevice, surface);
		VkCommandPoolCreateInfo poolInfo = { };

		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		// Command pool can only submit to one queue, we'll be submitting
		// graphics calls.
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
		poolInfo.flags = 0; // Optional

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create command pool!");
		}
	}

	static void CreateCommandBuffers(VkDevice &device, VkCommandPool &commandPool,
			VkRenderPass &renderPass, VkExtent2D extent, VkPipeline pipeline,
			std::vector<VkCommandBuffer> &commandBuffers,
			std::vector<VkFramebuffer> &frameBuffers) {

		commandBuffers.resize(frameBuffers.size());

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
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = frameBuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = extent;
			VkClearValue clearColor = { 0.05f, 0.05f, 0.1f, 1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			// Begin the render pass, can now submit drawing commands.
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, 
				VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipeline);
			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

			// End the render pass, stop submitting draw commands.
			vkCmdEndRenderPass(commandBuffers[i]);

			// Stop recording to the command buffer.
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to record command buffer!");
			}
		}
	}

private:
	VKBuilder() { }
};

#endif
