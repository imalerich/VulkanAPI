CFLAGS = -std=c++11 -g
LIBFLAGS = `pkg-config --static --libs glfw3` -lvulkan
OBJ = imApplication.o

VulkanDemo: main.cpp glsl imApplication.o PREFIX.h
	g++ $(CFLAGS) -o VulkanDemo main.cpp $(OBJ) $(LIBFLAGS)

imApplication.o: imApplication.h imApplication.cpp VKBuilder.hpp VKDebug.hpp VKSurfaceBuilder.hpp PREFIX.h
	g++ $(CFLAGS) -c imApplication.cpp

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
