#ifndef VK_SURFACE_SELECTOR_HPP
#define VK_SURFACE_SELECTOR_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <algorithm>
#include <vector>

#include "PREFIX.h"

class VKSurfaceBuilder {
public:

	static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
			const std::vector<VkSurfaceFormatKHR> &availableFormats) {
		// Best case scenario, the surface don't care.
		if (availableFormats.size() == 1 && 
				availableFormats[0].format == VK_FORMAT_UNDEFINED) {
			std::cout << "Format undefined, creating preferred format." << std::endl;
			return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}
		
		// Not best case scenario, but maybe what we want is present anyway.
		for (const auto &f : availableFormats) {
			if (f.format == VK_FORMAT_B8G8R8A8_UNORM 
					&& f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				// Cool beans, we got it. :)
				std::cout << "Preferred format selected manually." << std::endl;
				return f;
			}
		}

		std::cout << "Prefered format not found: " << 
			"Selecting first available format." << std::endl;
		return availableFormats[0];
	}

	static VkPresentModeKHR ChooseSwapPresentMode(
			const std::vector<VkPresentModeKHR> &availablePresentModes) {
		// Some drivers don't properly support this, so we'll prefer
		// IMMEDIATE to this if it is found.
		VkPresentModeKHR best = VK_PRESENT_MODE_FIFO_KHR;

		for (const auto &presentMode : availablePresentModes) {
			if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				// We really like tripple buffering, so we'll select
				// that over all other options.
				return presentMode;
			} else if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
				best = presentMode;
			}
		}

		return best;
	}

	static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}

		VkExtent2D actualExtent = { SCREENW, SCREENH };

		// Clamp the actual screen  width/height to the min/max supported values.
		actualExtent.width = std::max(
			capabilities.minImageExtent.width,
			std::max(capabilities.maxImageExtent.width, actualExtent.width)
		);

		actualExtent.height = std::max(
			capabilities.minImageExtent.height,
			std::max(capabilities.maxImageExtent.height, actualExtent.height)
		);

		return actualExtent;
	}

	static void CreateSwapChain(
			VkPhysicalDevice &pDevice, VkDevice device,
			VkSurfaceKHR &surface, VkSwapchainKHR &swapChain,
			QueueFamilyIndices indices, std::vector<VkImage> &images,
			VkFormat &swapChainImageFormat, VkExtent2D &swapChainExtent) {
		SwapChainSupportDetails details = QuerySwapChainSupport(pDevice, surface);

		// Select the format, present mode, and the extent to use when
		// creating our swap chain.
		VkSurfaceFormatKHR format = ChooseSwapSurfaceFormat(details.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(details.presentModes);
		VkExtent2D extent = ChooseSwapExtent(details.capabilities);

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

		// Now that the swap chain has been created, we can query for 
		// the handles to the images that were created within that swap chain.
		// Note, when the swap chain was created, it may have created more
		// images than we asked for (we only set min # of images) so pull the size again).
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		images.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, images.data());
		swapChainImageFormat = format.format;
		swapChainExtent = extent;
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
	VKSurfaceBuilder() { }
};

#endif
