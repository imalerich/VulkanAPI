#ifndef PREFIX_H
#define PREFIX_H

#define APP_NAME "Vulkan Demo"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <functional>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <limits>
#include <vector>
#include <set>

#ifdef NDEBUG
	const bool VALIDATION_LAYERS_ENABLED = false;
#else
	const bool VALIDATION_LAYERS_ENABLED = true;
#endif

const std::vector<const char *> VALIDATION_LAYERS = {
	"VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char *> DEVICE_EXTENSIONS = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#endif
