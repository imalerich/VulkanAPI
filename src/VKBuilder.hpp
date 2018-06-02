#ifndef VK_BUILDER_HPP
#define VK_BUILDER_HPP

#include "PREFIX.h"
#include "imVulkan.h"
#include "VKDebug.hpp"
#include "imPipeline.h"
#include "imSwapChain.h"

class VKBuilder {
public:

	static void CreateInstance() {
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

	static void CreateSurface() {
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface!");
		}
	}

	static void SelectPhysicalDevice() {
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

			if (IsDeviceSuitable(d)) {
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
		physicalDevice = *suitableDevices.begin();
		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("Failed to find a suitable GPU!");
		}
	}

	static bool IsDeviceSuitable(VkPhysicalDevice pDevice) {
		QueueFamilyIndices indices = FindQueueFamilies(pDevice, surface);
		bool extensionsSupported = CheckDeviceExtensionsSupport(pDevice, surface);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails support = QuerySwapChainSupport(pDevice);
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

	static void CreateLogicalDevice(VkQueue &gQueue, VkQueue &pQueue) {
		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);
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

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
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

	static void CreateCommandPoool(VkCommandPool &pool) {
		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalDevice, surface);
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

	static void CreateCommandBuffers(VkCommandPool &commandPool,
			imPipeline &pipeline, imSwapChain &swapchain,
			std::vector<VkCommandBuffer> &commandBuffers) {

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
			VkClearValue clearColor = { 0.05f, 0.05f, 0.1f, 1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			// Begin the render pass, can now submit drawing commands.
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, 
				VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffers[i], 
				VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.graphicsPipeline);
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
