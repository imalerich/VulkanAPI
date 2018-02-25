CFLAGS = -std=c++11
LIBFLAGS = `pkg-config --static --libs glfw3` -lvulkan

VulkanDemo: main.cpp glsl
	g++ $(CFLAGS) -o VulkanDemo main.cpp $(LIBFLAGS)

run: VulkanDemo
	./VulkanDemo

glsl: shaders/shader.vert shaders/shader.frag
	glslangValidator -V shaders/shader.vert -o shaders/vert.spv
	glslangValidator -V shaders/shader.frag -o shaders/frag.spv

clean:
	rm -rf VulkanDemo
	rm -rf shaders/vert.spv
	rm -rf shaders/frag.spv
