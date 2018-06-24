#ifndef IM_IMAGE_H
#define IM_IMAGE_H

#include "imVulkan.h"
#include <string>

class imImage {
public:
	void Create(std::string filename);

	static void Allocate(uint32_t width, uint32_t height, VkFormat imageFormat,
		VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
		VkImage &image, VkDeviceMemory &memory);
	static VkImageView CreateView(VkImage image, VkFormat format);

	void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
	void CopyBufferToImage(VkBuffer buffer);
	void CreateSampler();
	
	void Cleanup();

	VkImage image;
	VkImageView view;
	VkSampler sampler;
	VkDeviceMemory memory;
	VkFormat imageFormat;
	uint32_t width;
	uint32_t height;
};

#endif
