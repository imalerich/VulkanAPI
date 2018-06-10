#ifndef IM_BUFFER_H
#define IM_BUFFER_H

#include "imVulkan.h"

/// Find a memory type that fits the input needs for our physical device.
uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

/// Create a new buffer on the GPU using the given ubuffer usage and memory properties.
void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
		VkMemoryPropertyFlags properties, 
		VkBuffer &buffer, VkDeviceMemory &bufferMemory);

/// Copy 'size' bytes from the source buffer into the destination buffer.
/// Note: This is a device -> device memory transfer operation.
void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

#endif
