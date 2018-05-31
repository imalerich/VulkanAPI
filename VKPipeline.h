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
	void CreateGraphicsPipeline(VkDevice &device, VkExtent2D extent,
		std::string vertexFile, std::string fragFile);
	void CreateRenderPass(VkDevice &device, VkFormat format);
	void Cleanup(VkDevice &device);

	/// Describes a particular configuration of the graphics pipeline,
	/// holding all saders, fixed function states, render passes, etc.
	VkPipeline graphicsPipeline;
	/// Describes how attachments are used during subpasses in the rendering process.
	VkRenderPass renderPass;
	/// Layout for uniforms in the graphics pipeline.
	VkPipelineLayout pipelineLayout;

private:
	VkShaderModule CreateShaderModule(VkDevice &device, const std::vector<char> &code);

};

#endif
