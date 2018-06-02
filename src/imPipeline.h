#ifndef IM_PIPELINE_H
#define IM_PIPELINE_H

#include "PREFIX.h"
#include "imVulkan.h"

static std::vector<char> ReadFile(const std::string &filename);

class imPipeline {
public:
	void CreateGraphicsPipeline(VkExtent2D extent,
		std::string vertexFile, std::string fragFile);
	void CreateRenderPass(VkFormat format);
	void Cleanup();

	/// Describes a particular configuration of the graphics pipeline,
	/// holding all saders, fixed function states, render passes, etc.
	VkPipeline graphicsPipeline;
	/// Describes how attachments are used during subpasses in the rendering process.
	VkRenderPass renderPass;
	/// Layout for uniforms in the graphics pipeline.
	VkPipelineLayout pipelineLayout;

private:
	VkShaderModule CreateShaderModule(const std::vector<char> &code);

};

#endif
