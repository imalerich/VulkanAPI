#ifndef IM_APPLICATION_H
#define IM_APPLICATION_H

#include "PREFIX.h"
#include "imVulkan.h"

#include "imMesh.h"
#include "imPipeline.h"
#include "imSwapChain.h"

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
	void InitSemaphores();

	void CleanupSwapChain();
	void RecreateSwapChain();

	static void OnWindowResized(GLFWwindow * window, int width, int height);

	void Update();
	void DrawFrame();
	void Cleanup();

	/// Handle to the graphics queue for sending graphics command pools.
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	/// Handle to the presentation queue for presenting to the GLFW window.
	VkQueue presentQueue = VK_NULL_HANDLE;

	/// Will hold a basic configuration for our graphics pipeline.
	imPipeline pipeline;
	/// Will hold a basic configuratio for our swap chain.
	imSwapChain swapchain;

	/// Stores mesh data we wish to render.
	imMesh mesh;

	/// Manages the memory of allocated command buffers.
	VkCommandPool commandPool;
	/// We need one command buffer for each framebuffer in the swap chain.
	std::vector<VkCommandBuffer> commandBuffers;

	/// Holds rendering until an image is ready to render to.
	VkSemaphore imageAvailableSemaphore;
	/// Holds presentation until we are finished rendering.
	VkSemaphore renderFinishedSemaphore;

	/// Handle to validation layers debug callback.
	VkDebugReportCallbackEXT callback;
};

#endif
