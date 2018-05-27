#ifndef VK_BUILDER_HPP
#define VK_BUILDER_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

#include "VKSurfaceSelector.hpp"
#include "PREFIX.h"

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
			std::cout << "Found extension " << extension.extensionName << 
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

	static void CreateSwapChain(
			VkPhysicalDevice &pDevice, VkDevice device,
			VkSurfaceKHR &surface, VkSwapchainKHR &swapChain) {
		SwapChainSupportDetails details = QuerySwapChainSupport(pDevice, surface);

		// Select the format, present mode, and the extent to use when
		// creating our swap chain.
		VkSurfaceFormatKHR format = VKSurfaceSelector::ChooseSwapSurfaceFormat(
			details.formats);
		VkPresentModeKHR presentMode = VKSurfaceSelector::ChooseSwapPresentMode(
			details.presentModes);
		VkExtent2D extent = VKSurfaceSelector::ChooseSwapExtent(details.capabilities);

		// We also need to select how many images that will be in our queue.
		// For tripple buffering, we want one more than minimum if possible.
		uint32_t imageCount = details.capabilities.minImageCount + 1;
		if (details.capabilities.maxImageCount > 0 // 0 => No Limit sans memory.
				&& imageCount > details.capabilities.maxImageCount) {
			// Uh-Oh! We want too many images! :(
			imageCount = details.capabilities.maxImageCount;
		}

		// Now we can actually create our swap chain!!!
		VkSwapchainCreateInfoKHR createInfo = { };
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = format.format;
		createInfo.imageColorSpace = format.colorSpace;
		createInfo.imageExtent = extent;
		// Will be more than 1 for stereoscopic 3D applications.
		createInfo.imageArrayLayers = 1;
		// If we're doing off screen rendering for post-processing etc
		// we'll be setting this to VK_IMAGE_USAGE_TRANSFER_DST_BIT but
		// for now we're just rendering to the screen, so just use this.
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = FindQueueFamilies(pDevice, surface);
		uint32_t queueFamilyIndices[] = {
			(uint32_t)indices.graphicsFamily,
			(uint32_t)indices.presentFamily
		};

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		} else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		// Could instead tell the swap chain to perform a transform 
		// such as rotations and horizontal/vertical flips.
		createInfo.preTransform = details.capabilities.currentTransform;
		// If we want blending with other windows in the window system
		// for whatever reason. Obviously we'll always ignore this.
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		// If another window overlays ours clip obscurred pixels (don't
		// render to that pixel). Offers better performance, but I can
		// image it may effect some post processing (especially blurs)
		// if we're running in windowed mode.
		// We'll just take the better performance for now.
		createInfo.clipped = VK_TRUE;
		// If window is resized, we'll need to create a new swap chain.
		// We'll learn more about this later.
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		// Finally! We're reading to create the swap chain!
		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) 
				!= VK_SUCCESS) {
			throw std::runtime_error("Failed to create Swap Chain!");
		}
	}

private:
	VKBuilder() { }
};

#endif
