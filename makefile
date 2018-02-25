CFLAGS = -std=c++11
LIBFLAGS = `pkg-config --static --libs glfw3` -lvulkan

VulkanDemo: main.cpp shaders
	g++ $(CFLAGS) -o VulkanDemo main.cpp $(LIBFLAGS)

run: VulkanDemo
	./VulkanDemo

shaders: shaders/shader.vert shaders/shader.frag
	glslangValidator -V shaders/shader.vrt -o vert.spv
	glslangValidator -V shaders/shader.frag -o frag.spv

clean:
	rm -rf VulkanDemo
