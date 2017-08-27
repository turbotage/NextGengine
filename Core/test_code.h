#pragma once

#include "src\math\vec3.h"
#include "src\math\vec4.h"
#include "src\math\mat4.h"

void testOfMat4() {
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

	std::cout << one.mul(two) << std::endl;
	std::cout << "wawo" << std::endl;
}

void testMath() {
#define set_nm(x, n, m, num) (x.rows[n].elements[m]=num)

	ng::math::DynMat dots;
	ng::math::DynMat conector;
	ng::math::DynMat re;

	dots.init(2, 2);
	conector.init(1, 2);
	re.init(2, 1);

	set_nm(dots, 0, 0, 0);
	set_nm(dots, 0, 1, 0);

	set_nm(dots, 1, 0, 4);
	set_nm(dots, 1, 1, 4);

	set_nm(conector, 0, 0, 0);
	set_nm(conector, 1, 0, 1);
	std::cout << dots << "\n" << conector << "\n\n";
	re = dots*conector;
	std::cout << re << std::endl;
}