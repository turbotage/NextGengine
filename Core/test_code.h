#pragma once

#include "src\Math\vec3f.h"
#include "src\Math\vec4f.h"
#include "src\Math\mat4f.h"
#include "src\Math\dyn_mat.h"

void testOfMat4() {
	ngm::Vec3f A(3, 2, 6);
	std::cout << A << std::endl;
	ngm::Vec4f one(4, 5, 6, 7);
	std::cout << one << std::endl;
	ngm::Vec4f two(2, 7, 3, 4);
	std::cout << two << std::endl;
	ngm::Vec4f three(1, 2, 1, 7);
	std::cout << three << std::endl;
	ngm::Vec4f four(2, 9, 3, 1);
	std::cout << four << std::endl;

	ngm::Mat4f mat1(one, two, three, four);
	std::cout << mat1 << std::endl;
	ngm::Mat4f mat2(four, two, one, four);
	std::cout << mat2 << std::endl;

	std::cout << one.mul(two) << std::endl;
	std::cout << "wawo" << std::endl;
}

