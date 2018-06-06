#ifndef IM_MESH_H
#define IM_MESH_H

#include "imVulkan.h"
#include "imVertex.hpp"

class imMesh {
public:
	void Create();
	void CreateVertexBuffer();
	void CreateIndexBuffer();
	void Cleanup();

	VkBuffer vertexBuffer;
	VkBuffer indexBuffer;

private:
	VkDeviceMemory vertexBufferMemory;
	VkDeviceMemory indexBufferMemory;
};

#endif
