#ifndef IM_SWAPCHAIN_H
#define IM_SWAPCHAIN_H

#include "imVulkan.h"
#include "imImage.h"
#include "PREFIX.h"

class imSwapChain {
public:
	/// Initialize a new swap chain and choose a format and extent.
	void CreateSwapChain();

	/// Generate the 'imageViews' array.
	void CreateImageViews();

	/// Generate the 'frameBuffers' array encapsulating
	/// the given render pass.
	void CreateFrameBuffers(VkRenderPass &renderPass);

	/// Create and allocate the depth buffer.
	void CreateDepthBuffer();

	/// Cleanup data stored by the swap chain.
	void Cleanup();

	// -----------------------------------
	// --- Encapsulated Vulkan Handles ---
	// -----------------------------------

	/// Holds all of our render targets, we'll be aiming for tripple buffering.
	VkSwapchainKHR swapChain;

	/// Image format that was used to create the swap chain, useful to keep around.
	VkFormat imageFormat;
	/// Extent of the swap chain images used to create the swap chain.
	VkExtent2D extent;

	/// Image to use for the depth buffer, we only need one.
	VkImage depthImage;
	/// Use to allocate and free memory for the depth buffer.
	VkDeviceMemory depthImageMemory;
	/// View into the depth buffer.
	VkImageView depthImageView;

	/// Handles to each image contained within our swap chain.
	std::vector<VkImage> images;
	/// Views into each image of the swap chain.
	/// This relationship is similar to the idea of having both a physical
	/// device and a logical device to interface with that physical device.
	std::vector<VkImageView> imageViews;
	/// Encapsulates all image views (color, depth, etc) to be drawn to.
	std::vector<VkFramebuffer> frameBuffers;

private:
	// -------------------------------------------------
	// --- Private Utilities for Swap Chain Creation ---
	// -------------------------------------------------

	/// Given all available surface formats for we can use for the swap chain, choose
	/// the most desirable for our application.
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
		const std::vector<VkSurfaceFormatKHR> &availableFormats);
	/// Given a list of presentation modes, choose the best one to use
	/// for our swap chain.
	VkPresentModeKHR ChooseSwapPresentMode(
		const std::vector<VkPresentModeKHR> &availablePresentModes);
	/// Choose a size for the swap chain that best matches the screen resolution
	/// while considering the capabilities of our device.
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
};

#endif
