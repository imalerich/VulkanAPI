#include "imApplication.h"
#include "PREFIX.h"

/*
 * Vulkan Tutorial
 * https://vulkan-tutorial.com
 * Page 125
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
