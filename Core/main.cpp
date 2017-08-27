#include <iostream>
#include <stdexcept>
#include "src\def.h"
#include "src\Graphics\Vulkan\vulkan_base.h"
#include "src\debug.h"
#include <chrono>
#include "src\Math\dyn_mat.h"

using namespace ng;


class Application {
private:
	graphics::Window window;
	graphics::VulkanBase vulkanBase;
public:

	~Application() {
		cleanup();
	}

	void init() {
		window.init(800, 600, "window");
		vulkanBase.createInstance();
		vulkanBase.createDebugCallback();
		window.createSurface(&vulkanBase.instance, &vulkanBase.surface);
		vulkanBase.createDevices();
	}

	void run() {
		window.run();
	}

	void cleanup() {
		vulkanBase.freeDebugCallback();
		vulkanBase.freeInstance();
		glfwDestroyWindow(window.glfwWindowPtr);
		glfwTerminate();
	}

};

int main(int argc, char* argv[]){
#define set_nm(x, n, m, num) (x.rows[n].elements[m]=num)

	ng::math::DynMat dots;
	ng::math::DynMat conector;
	ng::math::DynMat re;

	dots.init(2, 2);
	conector.init(1, 2);
	re.init(1, 2);

	set_nm(dots, 0, 0, 0);
	set_nm(dots, 0, 1, 0);

	set_nm(dots, 1, 0, 4);
	set_nm(dots, 1, 1, 4);

	set_nm(conector, 0, 0, 0);
	set_nm(conector, 1, 0, 1);
	std::cout << dots << "\n" << conector << "\n\n";
	re = *(dots*conector);
	int a = 10;
	std::cout << re << std::endl;

	using namespace ng::graphics;
	Application app;
	try {
		app.init();
		app.run();
	}
	catch (std::runtime_error e) {
		std::cout << e.what() << std::endl;
	}
	getchar();
	return 0;
}


