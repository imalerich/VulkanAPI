#include "imMesh.h"
#include "imBuffer.hpp"

void imMesh::CreateVertexBuffer() {
	// Use this buffer as a staging buffer, it's cache coherent 
	// so we'll have memory available as soon as we unmap it.
	VkDeviceSize bufferSize = sizeof(VERTICES[0]) * VERTICES.size();
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	void * data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, VERTICES.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	// This will be the actual buffer holding our data, copied from
	// our staging buffer to a location on the GPU not accessible from the CPU.
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | 
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer, vertexBufferMemory);
	CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	// Clean up the staging buffer, we don't need it anymore.
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void imMesh::Cleanup() {
	vkDestroyBuffer(device, vertexBuffer, nullptr);
	vkFreeMemory(device, vertexBufferMemory, nullptr);
}
