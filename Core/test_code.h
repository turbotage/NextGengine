#pragma once

#include "src\math\Vec3f.h"
#include "src\math\Vec4f.h"
#include "src\math\mat4.h"

void testOfMat4() {
	ng::math::Vec3f A(3, 2, 6);
	std::cout << A << std::endl;
	ng::math::Vec4f one(4, 5, 6, 7);
	std::cout << one << std::endl;
	ng::math::Vec4f two(2, 7, 3, 4);
	std::cout << two << std::endl;
	ng::math::Vec4f three(1, 2, 1, 7);
	std::cout << three << std::endl;
	ng::math::Vec4f four(2, 9, 3, 1);
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


std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(-1, 1);
std::uniform_real_distribution<> dis2(0, 2 * 3.141592);

using namespace ng::math;

#define MAT_SIZE 10000000
Mat4 *rotaionMats1 = new Mat4[MAT_SIZE];
Mat4 *rotaionMats2 = new Mat4[MAT_SIZE];
Mat4 ret;

auto start = std::chrono::system_clock::now();
for (int i = 0; i < MAT_SIZE; ++i) {
	Vec3f rotationAxis1(dis(gen), dis(gen), dis(gen));
	rotationAxis1.normalize();
	Vec3f rotationAxis2(dis(gen), dis(gen), dis(gen));
	rotationAxis2.normalize();
	rotaionMats1[i] = Mat4::rotation(dis2(gen), rotationAxis1);
	rotaionMats2[i] = Mat4::rotation(dis2(gen), rotationAxis1);
	ret = rotaionMats1[i] * rotaionMats2[i];
}
auto end = std::chrono::system_clock::now();
auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
std::cout << elapsed.count() << '\n';

delete[] rotaionMats1;
delete[] rotaionMats2;