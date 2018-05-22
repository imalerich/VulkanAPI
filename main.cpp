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

#define SCREENW 800
#define SCREENH 600
#define APP_NAME "Vulkan Demo"

using namespace std;

/*
 * Vulkan Tutorial
 * https://vulkan-tutorial.com
 * Page 37
 */

int main(int argc, char ** argv) {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow * window = glfwCreateWindow(SCREENW, SCREENH, APP_NAME, nullptr, nullptr);

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	cout << extensionCount << " extensions supported" << endl;

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
