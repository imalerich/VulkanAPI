#include "imMesh.h"

void imMesh::CreateVertexBuffer() {
	VkDeviceSize bufferSize = sizeof(VERTICES[0]) * VERTICES.size();
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		vertexBuffer, vertexBufferMemory);

	void * data;
	vkMapMemory(device, vertexBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, VERTICES.data(), (size_t)bufferSize);
	vkUnmapMemory(device, vertexBufferMemory);
}

void imMesh::Cleanup() {
	vkDestroyBuffer(device, vertexBuffer, nullptr);
	vkFreeMemory(device, vertexBufferMemory, nullptr);
}
