#include "VKPipeline.h"

static std::vector<char> ReadFile(const std::string &filename) {
	// Start at the end of the file, we can immediately judge the size.
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	// Now that we have the size, jump back to the beginning.
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	// All done!
	file.close();
	return buffer;
}

void VKPipeline::LoadShaders(VkDevice &device, 
		std::string vertexFile, std::string fragFile) {
	auto vertCode = ReadFile(vertexFile);
	auto fragCode = ReadFile(vertexFile);

	std::cout << "-----------------------------------------------" << std::endl;
	std::cout << "Loaded Shader " << vertexFile << " with size (" 
		<< vertCode.size() << ")." << std::endl;
	std::cout << "Loaded Shader " << fragFile << " with size (" 
		<< fragCode.size() << ")." << std::endl;
	std::cout << "-----------------------------------------------" << std::endl;

	VkShaderModule vertModule;
	VkShaderModule fragModule;

	vertModule = CreateShaderModule(device, vertCode);
	fragModule = CreateShaderModule(device, fragCode);

	// Create info for the vertex shader stage.
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = { };
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertModule;
	vertShaderStageInfo.pName = "main";

	// Create info for the fragment shader stage.
	VkPipelineShaderStageCreateInfo fragShaderStageInfo = { };
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {
		vertShaderStageInfo, fragShaderStageInfo
	};

	// We have all the programmable stages set up, now we only need to set
	// up the fixed function stages of the pipeline.
	// TODO

	vkDestroyShaderModule(device, fragModule, nullptr);
	vkDestroyShaderModule(device, vertModule, nullptr);
}

VkShaderModule VKPipeline::CreateShaderModule(VkDevice &device, 
		const std::vector<char> &code) {
	VkShaderModuleCreateInfo createInfo = { };
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module!");
	}

	return shaderModule;
}
