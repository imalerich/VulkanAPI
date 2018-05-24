#include "imApplication.h"

#define SCREENW 800
#define SCREENH 600
#define APP_NAME "Vulkan Demo"

/*
 * Vulkan Tutorial
 * https://vulkan-tutorial.com
 * Page 69
 */

int main(int argc, char ** argv) {
	imApplication app(SCREENW, SCREENH, APP_NAME);

	try {
		app.Run();
	} catch ( const std::runtime_error &e ) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
