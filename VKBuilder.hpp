#ifndef VK_BUILDER_HPP
#define VK_BUILDER_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

const std::vector<const char *> VALIDATION_LAYERS = {
	"VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char *> DEVICE_EXTENSIONS = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
	const bool VALIDATION_LAYERS_ENABLED = false;
#else
	const bool VALIDATION_LAYERS_ENABLED = true;
#endif

#include "VKDebug.hpp"

struct QueueFamilyIndices {
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool IsComplete() {
		return (graphicsFamily >= 0) && (presentFamily >= 0);
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

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
			std::cout << "\t" << extension.extensionName << std::endl;
		}

		std::cout << "-----------------------------------------------" << std::endl;
	}

	static void CreateSurface(VkInstance &instance, 
			GLFWwindow * window, VkSurfaceKHR &surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface!");
		}
	}

	static void SelectPhysicalDevice(VkInstance &instance, VkSurfaceKHR &surface, 
			VkPhysicalDevice &pDevice) {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");
		}

		std::cout << "Found " << deviceCount << " Device(s)!" << std::endl;
		std::cout << "-----------------------------------------------" << std::endl;

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto &d : devices) {
			if (IsDeviceSuitable(d, surface)) {
				pDevice = d;
				break;
			}
		}

		if (pDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("Failed to find a suitable GPU!");
		}
	}

	static bool IsDeviceSuitable(VkPhysicalDevice pDevice, VkSurfaceKHR &surface) {
		QueueFamilyIndices indices = FindQueueFamilies(pDevice, surface);
		bool extensionsSupported = CheckDeviceExtensionsSupport(pDevice, surface);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails support = QuerySwapChainSupport(pDevice, surface);
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
			requiredExtensions.erase(extension.extensionName);
		}

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
		std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

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
		createInfo.enabledExtensionCount = 0;

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

	static SwapChainSupportDetails QuerySwapChainSupport
			(VkPhysicalDevice &pDevice,VkSurfaceKHR &surface) {
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

private:
	VKBuilder() { }
};

#endif
