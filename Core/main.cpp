#include <iostream>
#include <stdexcept>
#include "src\graphics\graphics.h"
	
#include "src\math\vec3.h"
#include "src\math\vec4.h"
#include "src\math\mat4.h"
	
#include <chrono>
	
int main(int argc, char* argv[]){
	
	ng::math::Vec3 A(3, 2, 6);
	std::cout << A << std::endl;
	ng::math::Vec4 one(4, 5, 6, 7);
	std::cout << one << std::endl;
	ng::math::Vec4 two(2, 7, 3, 4);
	std::cout << two << std::endl;
	ng::math::Vec4 three(1, 2, 1, 7);
	std::cout << three << std::endl;
	ng::math::Vec4 four(2, 9, 3, 1);
	std::cout << four << std::endl;
	
	ng::math::Mat4 mat1(one, two, three, four);
	std::cout << mat1 << std::endl;
	ng::math::Mat4 mat2(four, two, one, four);
	std::cout << mat2 << std::endl;
	
	int a;
	std::cout << one.mul(two) << std::endl;
	int b;
	std::cout << "wawo" << std::endl;
	
	try {
		ng::graphics::Graphics graphics("waow", 800, 600);
		graphics.run();
	}
	catch (std::runtime_error e) {
		std::cout << e.what() << std::endl;
	}
	std::string a;
	std::cin >> a;
	return 0;
}
