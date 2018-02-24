CFLAGS = -std=c++11
LIBFLAGS = `pkg-config --static --libs glfw3` -lvulkan

VulkanDemo: main.cpp
	g++ $(CFLAGS) -o VulkanDemo main.cpp $(LIBFLAGS)

run: VulkanDemo
	./VulkanDemo

clean:
	rm -rf VulkanDemo
