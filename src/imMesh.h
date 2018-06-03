#ifndef IM_MESH_H
#define IM_MESH_H

#include "imVulkan.h"
#include "imVertex.hpp"

class imMesh {
public:
	void CreateVertexBuffer();
	void Cleanup();

	VkBuffer vertexBuffer;

private:
	VkDeviceMemory vertexBufferMemory;
};

#endif
