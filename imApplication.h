#ifndef IM_APPLICATION_H
#define IM_APPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <functional>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cstring>
#include <limits>
#include <vector>
#include <set>

class imApplication {
public:
	/**
	 * Constructs a new vulkan application with the given window settings.
	 * \param screen_w Horizontal (Width) dimension of the main screen.
	 * \param screen_h Vertical (Height) dimension of the main screen.
	 * \param app_name Window Title to associate with the application.
	 */
	imApplication(size_t screen_w, size_t screen_h, const char * app_name);
	/**
	 * Releases all GLFW and Vulkan assets.
	 */
	~imApplication();

	/**
	 * Main game loop.
	 */
	void Run();

private:
	void InitGLFW(size_t screen_w, size_t screen_h, const char * app_name);
	void InitVulkan();
	void Cleanup();

	/// Reference to the main application window created by GLFW.
	GLFWwindow * window;
	/// Handle to our Vulkan instance. This is the connection between
	/// this application and the Vulkan library.
	VkInstance instance;

	/// Handle to validation layers debug callback.
	VkDebugReportCallbackEXT callback;
};

#endif
