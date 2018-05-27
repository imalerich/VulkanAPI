#ifndef VK_SURFACE_SELECTOR_HPP
#define VK_SURFACE_SELECTOR_HPP

#include <algorithm>
#include "PREFIX.h"

class VKSurfaceSelector {
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

private:
	VKSurfaceSelector() { }
};

#endif
