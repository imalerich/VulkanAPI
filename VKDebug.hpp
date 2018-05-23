#ifndef VK_DEBUG_HPP
#define VK_DEBUG_HPP

class VKDebug {
public:
	static void SetupDebugCallback(VkInstance instance, VkDebugReportCallbackEXT &callback) {
		if (!VALIDATION_LAYERS_ENABLED) { return; }

		VkDebugReportCallbackCreateInfoEXT createInfo = { };
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = DebugCallback;

		if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) 
				!= VK_SUCCESS) {
			throw std::runtime_error("Failed to Set Up Debug Callback!");
		}
	}

	static void DestroyDebugReportCallbackEXT(VkInstance instance, 
			VkDebugReportCallbackEXT &callback, 
			const VkAllocationCallbacks * pAllocator) {
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance,
				"vkDestroyDebugReportCallbackEXT");
		if (func != nullptr) {
			func(instance, callback, pAllocator);
		} else {
			throw std::runtime_error("Failed to find vkDestroyDebugReportCallbackEXT.");
		}
	}

private:
	VKDebug() { }

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugReportFlagsEXT flags,
			VkDebugReportObjectTypeEXT objType,
			uint64_t obj,
			size_t location,
			int32_t code,
			const char * layerPrefix,
			const char * msg,
			void * userData) {
		std::cerr << "Validation Layer: " << msg << std::endl;
		return VK_FALSE;
	}

	static VkResult CreateDebugReportCallbackEXT(VkInstance instance, 
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
};

#endif
