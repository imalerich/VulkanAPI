CFLAGS = -std=c++11 -g
LIBFLAGS = `pkg-config --static --libs glfw3` -lvulkan
OBJ = imApplication.o imPipeline.o imSwapChain.o imVulkan.o imMesh.o imBuffer.o imImage.o

VulkanDemo: src/main.cpp glsl imApplication.o
	g++ $(CFLAGS) -o VulkanDemo src/main.cpp $(OBJ) $(LIBFLAGS)

APPDEPS = imPipeline.o imSwapChain.o imVulkan.o imMesh.o imBuffer.o src/VKBuilder.hpp src/VKDebug.hpp imImage.o
imApplication.o: src/imApplication.h src/imApplication.cpp $(APPDEPS)
	g++ $(CFLAGS) -c src/imApplication.cpp

# Utility classes which encapsulate Vulkan data.

imPipeline.o: src/imPipeline.h src/imPipeline.cpp src/imVertex.hpp imVulkan.o
	g++ $(CFLAGS) -c src/imPipeline.cpp

imSwapChain.o: src/imSwapChain.h src/imSwapChain.cpp imVulkan.o src/imImage.h
	g++ $(CFLAGS) -c src/imSwapChain.cpp

imMesh.o: src/imMesh.h src/imMesh.cpp imVulkan.o src/imVertex.hpp imBuffer.o
	g++ $(CFLAGS) -c src/imMesh.cpp

imImage.o: src/imImage.h src/imImage.cpp imVulkan.o imBuffer.o
	g++ $(CFLAGS) -c src/imImage.cpp

imBuffer.o: src/imBuffer.h src/imBuffer.cpp imVulkan.o
	g++ $(CFLAGS) -c src/imBuffer.cpp

# Base dependency, everything depends on this.
# Declares global Vulkan constants.

imVulkan.o: src/imVulkan.h src/imVulkan.cpp src/PREFIX.h
	g++ $(CFLAGS) -c src/imVulkan.cpp

run: VulkanDemo
	./VulkanDemo

glsl: shaders/shader.vert shaders/shader.frag
	glslangValidator -V shaders/shader.vert -o shaders/vert.spv
	glslangValidator -V shaders/shader.frag -o shaders/frag.spv

clean:
	rm -rf VulkanDemo
	rm -rf shaders/vert.spv
	rm -rf shaders/frag.spv
	rm -f *.o
