#ifndef IM_VERTEX_HPP
#define IM_VERTEX_HPP

#include "imVulkan.h"

#include <cstddef>
#include <array>

/// Defines the format we'll use to send vertex data to the gPU.
class imVertex {
public:
	glm::vec2 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription GetBindingDescription() {
		VkVertexInputBindingDescription bindingDesc = { };
		bindingDesc.binding = 0;
		bindingDesc.stride = sizeof(imVertex);
		bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDesc;
	}

	static std::array<VkVertexInputAttributeDescription, 3> GetAttrDescription() {
		std::array<VkVertexInputAttributeDescription, 3> attrDesc = { };

		attrDesc[0].binding = 0;
		attrDesc[0].location = 0;
		attrDesc[0].format = VK_FORMAT_R32G32_SFLOAT;
		attrDesc[0].offset = offsetof(imVertex, pos);

		attrDesc[1].binding = 0;
		attrDesc[1].location = 1;
		attrDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attrDesc[1].offset = offsetof(imVertex, color);

		attrDesc[2].binding = 0;
		attrDesc[2].location = 2;
		attrDesc[2].format = VK_FORMAT_R32G32_SFLOAT;
		attrDesc[2].offset = offsetof(imVertex, texCoord);

		return attrDesc;
	}
};

/// Temporary constant array of vertices for testing.
const std::vector<imVertex> VERTICES = {
	{ { -1.0f, -1.0f },	{ 1.0f, 0.2f, 0.0f },	{ 1.0f, 0.0f } },
	{ {  1.0f, -1.0f },	{ 0.0f, 0.4f, 0.6f },	{ 0.0f, 0.0f } },
	{ {  1.0f,  1.0f },	{ 0.0f, 1.0f, 1.0f },	{ 0.0f, 1.0f } },
	{ { -1.0f,  1.0f },	{ 0.2f, 0.1f, 0.5f },	{ 1.0f, 1.0f } },
};

const std::vector<uint16_t> INDICES = {
	0, 1, 2, 2, 3, 0
};

#endif
