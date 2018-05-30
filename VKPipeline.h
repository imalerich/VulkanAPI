#ifndef PIPELINE_H
#define PIPELINE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

static std::vector<char> ReadFile(const std::string &filename);

class VKPipeline {
public:
	void LoadShaders(VkDevice &device, VkExtent2D extent,
		std::string vertexFile, std::string fragFile);

	VkShaderModule CreateShaderModule(VkDevice &device, const std::vector<char> &code);

private:
};

#endif
