#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <vector>
#include <cstring>

#define SCREENW 800
#define SCREENH 600
#define APP_NAME "Vulkan Demo"

/*
 * Vulkan Tutorial
 * https://vulkan-tutorial.com
 * Page 64
 */

const std::vector<const char *> VALIDATION_LAYERS = {
	"VK_LAYER_LUNARG_standard_validation"
};

#ifdef NDEBUG
	const bool VALIDATION_LAYERS_ENABLED = false;
#else
	const bool VALIDATION_LAYERS_ENABLED = true;
#endif

VkResult CreateDebugReportCallbackEXT(VkInstance instance, 
		const VkDebugReportCallbackCreateInfoEXT * pCreateInfo,
		const VkAllocationCallbacks * pAllocator,
		VkDebugReportCallbackEXT * pCallback) {
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, 
		"vkCreateDebugReportCallbackEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

VkResult DestroyDebugReportCallbackEXT(VkInstance instance, 
		VkDebugReportCallbackEXT callback,
		const VkAllocationCallbacks * pAllocator) {
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance,
		"vkDestroyDebugReportCallbackEXT");
	if (func != nullptr) { func(instance, callback, pAllocator); }
}

struct QueueFamilyIndecies {
	int graphicsFamily = -1;

	bool isComplete() {
		return graphicsFamily >= 0;
	}

	static QueueFamilyIndecies findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndecies indecies;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto &queueFamily : queueFamilies) {
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indecies.graphicsFamily = i;
			}

			if (indecies.isComplete()) {
				break;
			}

			i++;
		}

		return indecies;
	}
};

class HelloTriangleApplication {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	/// Pointer to this applications main window managed by glfw.
	GLFWwindow * window;
	/// Manages Vulkan's interaction with our debug callback method.
	VkDebugReportCallbackEXT callback;

	/// Manage the connection between this application and the Vulkan library.
	VkInstance instance;
	/// Handle to the physical GPU we have chosen to perform our work
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	/// Handle to the logical device to interface with the GPU.
	VkDevice device;
	/// Handle to the graphics queue for sending commands.
	VkQueue graphicsQueue;

	// -----------------------------
	// Initialization and main loop.
	// -----------------------------

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	void initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window = glfwCreateWindow(SCREENW, SCREENH, APP_NAME, nullptr, nullptr);
	}

	void initVulkan() {
		createInstance();
		setupDebugCallback();
		pickPhysicalDevice();
		createLogicalDevice();
	}

	void cleanup() {
		vkDestroyDevice(device, nullptr);
		DestroyDebugReportCallbackEXT(instance, callback, nullptr);
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	// --------------------------
	// Instance & Extension Setup
	// --------------------------

	void createInstance() {
		// Check that all desired validation layers are supported.
		if (VALIDATION_LAYERS_ENABLED && !checkValidationLayerSupport()) {
			throw std::runtime_error("Validation layers requested, but not available!");
		}

		// Some basic optional information about this application.
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = APP_NAME;
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// Describes what global extensions we will need
		// these will be provided to us by glfw3.
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (VALIDATION_LAYERS_ENABLED) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
			createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Vulkan instance!");
		}

		// (DEBUG) Output support extensions of the current device.
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
		std::cout << "Available extensions for current device:" << std::endl;
		for (const auto &e : availableExtensions) {
			std::cout << "    - " << e.extensionName << std::endl;
		}
		std::cout << std::endl;
	}

	std::vector<const char *> getRequiredExtensions() {
		std::vector<const char *> extensions;

		unsigned glfwExtensionCount = 0;
		const char ** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		for (unsigned int i=0; i < glfwExtensionCount; i++) {
			extensions.push_back(glfwExtensions[i]);
		}

		if (VALIDATION_LAYERS_ENABLED) {
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}

		return extensions;
	}

	// ----------------
	// Device Selection
	// ----------------

	void pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0) {
			throw std::runtime_error("Failed to find a GPU with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto &d : devices) {
			if (isDeviceSuitable(d)) {
				physicalDevice = d;
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("Failed to find a suitable GPU!");
		}
	}

	bool isDeviceSuitable(VkPhysicalDevice device) {
		// We just need a device with a queue family that supports graphics commands.
		QueueFamilyIndecies indecies = QueueFamilyIndecies::findQueueFamilies(device);
		return indecies.isComplete();
	}

	void createLogicalDevice() {
		 QueueFamilyIndecies indecies = QueueFamilyIndecies::findQueueFamilies(physicalDevice);

		 VkDeviceQueueCreateInfo queueCreateInfo = {};
		 queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		 queueCreateInfo.queueFamilyIndex = indecies.graphicsFamily;
		 queueCreateInfo.queueCount = 1;

		 float queuePriority = 1.0f;
		 queueCreateInfo.pQueuePriorities = &queuePriority;

		 VkPhysicalDeviceFeatures deviceFeatures = {};

		 VkDeviceCreateInfo createInfo = {};
		 createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		 createInfo.pQueueCreateInfos = &queueCreateInfo;
		 createInfo.queueCreateInfoCount = 1;
		 createInfo.pEnabledFeatures = &deviceFeatures;

		 createInfo.enabledExtensionCount = 0;
		 if (VALIDATION_LAYERS_ENABLED) {
			 createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
			 createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
		 } else {
			 createInfo.enabledLayerCount = 0;
		 }

		 if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			 throw std::runtime_error("Failed to create logical device!");
		 }

		 vkGetDeviceQueue(device, indecies.graphicsFamily, 0, &graphicsQueue);
	}

	// ----------------------
	// Validation Layer Setup
	// ----------------------

	bool checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		// (DEBUG) Output available validation layers for this system.
		std::cout << "Available validation layers for current system:" << std::endl;
		for (const auto &layerProperties : availableLayers) {
			std::cout << "    - " << layerProperties.layerName << std::endl;
		}
		std::cout << std::endl;

		// Make sure all the layers we want are available on this device.
		for (const char * layerName : VALIDATION_LAYERS) {
			bool layerFound = false;
			for (const auto &layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) { return false; }
		}

		return true;
	}

	void setupDebugCallback() {
		if (!VALIDATION_LAYERS_ENABLED) { return; }
		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)debugCallback;

		if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS) {
			throw std::runtime_error("Failed to set up debug callback!");
		}
	}

	/// Callback function for Vulkan validation layers.
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugReportFlagBitsEXT flags,
			VkDebugReportObjectTypeEXT objType,
			uint64_t obj, size_t location, int32_t code,
			const char * layerPrefix, const char * msg, void * userData) {
		std::cerr << "Validation Layer Reports: " << msg << std::endl;
		return VK_FALSE;
	}
};

int main() {
	HelloTriangleApplication app;

	try {
		app.run();
	} catch (const std::runtime_error &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}