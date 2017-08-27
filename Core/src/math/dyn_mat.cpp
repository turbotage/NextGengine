#include "dyn_mat.h"
#include <string>


void ng::math::DynMat::init(unsigned short w, unsigned short h)
{
	height = h;
	width = w;
	rows = new DynVec[h];
	for (short i = 0; i < height; ++i) {
		rows[i].init(w);
	}
}

void ng::math::DynMat::cleanup()
{
	if (rows != nullptr) {
		delete[] rows;
	}
	rows = nullptr;
}

void ng::math::DynMat::setNaN()
{
	for (int i = 0; i < height; ++i) {
		rows[i].setNaN();
	}
}

ng::math::DynMat::DynMat()
{

}

ng::math::DynMat::DynMat(unsigned short w, unsigned short h)
{
	init(w, h);
}

ng::math::DynMat::~DynMat()
{
	cleanup();
}

ng::math::DynMat & ng::math::DynMat::add(const DynMat & other)
{
	if (width == other.width && height == other.height) {
		setNaN();
	}
	for (short i = 0; i < height; ++i) {
		rows[i] += other.rows[i];
	}
	return *this;
}

ng::math::DynMat & ng::math::DynMat::sub(const DynMat & other)
{
	if (width == other.width && height == other.height) {
		setNaN();
	}
	for (unsigned short i = 0; i < height; ++i) {
		rows[i] -= other.rows[i];
	}
	return *this;
}

ng::math::DynMat* ng::math::DynMat::mul(const DynMat & other)
{
	if (height != other.width) {
		setNaN();
		return this;
	}
	DynMat* ret = new DynMat(height, other.width);
	DynVec otherMul(other.height);
	for (unsigned short i = 0; i < height; ++i) {
		for (unsigned short j = 0; j < other.width; ++j) {
			for (unsigned short k = 0; k < other.height; ++k) {
				otherMul.elements[k] = other.rows[k].elements[j];
			}
			ret->rows[i].elements[j] = rows[i].dot(otherMul);
		}
	}
	return ret;
}

bool ng::math::DynMat::operator==(const DynMat & other) const
{
	if (height != other.height || width != other.width) {
		return false;
	}
	for (unsigned short i = 0; i < height; ++i) {
		for (unsigned short j = 0; j < width; ++j) {
			if (rows[i].elements[j] != other.rows[i].elements[j]) {
				return false;
			}
		}
	}
	return true;
}

bool ng::math::DynMat::operator!=(const DynMat & other) const
{
	if (height != other.height || width != other.width) {
		return true;
	}
	for (unsigned short i = 0; i < height; ++i) {
		for (unsigned short j = 0; j < width; ++j) {
			if (rows[i].elements[j] != other.rows[i].elements[j]) {
				return true;
			}
		}
	}
	return false;
}

ng::math::DynMat & ng::math::DynMat::operator+=(const DynMat & other)
{
	return add(other);
}

ng::math::DynMat & ng::math::DynMat::operator-=(const DynMat & other)
{
	return sub(other);
}

ng::math::DynMat ng::math::operator+(DynMat left, const DynMat & right)
{
	return left.add(right);
}

ng::math::DynMat ng::math::operator-(DynMat left, const DynMat & right)
{
	return left.sub(right);
}

ng::math::DynMat* ng::math::operator*(DynMat left, const DynMat & right)
{
	return left.mul(right);
}

std::ostream & ng::math::operator<<(std::ostream & stream, const DynMat & matrix)
{
	stream << "Mat4: (\n";
	for (unsigned short i = 0; i < matrix.height; ++i) {
		for (unsigned short j = 0; j < matrix.width; ++j) {
			stream << matrix.rows[i].elements[j] << "  ";
		}
		stream << "\n";
	}
	stream << ")\n";
	return stream;
}
