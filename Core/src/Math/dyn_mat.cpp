#include "dyn_mat.h"
#include <string>

template<typename T>
void ng::math::DynMat<T>::init(uint16 w, uint16 h)
{
	height = h;
	width = w;
	rows = new DynVec<T>(w)[h];
	for (short i = 0; i < height; ++i) {
		rows[i].init(w);
	}
}

template<typename T>
void ng::math::DynMat<T>::cleanup()
{
	if (rows != nullptr) {
		for (short i = 0; i < height; ++i) {
			rows[i].cleanup();
		}
		rows = nullptr;
	}
}

template<typename T>
void ng::math::DynMat<T>::setNaN()
{
	for (int i = 0; i < height; ++i) {
		rows[i].setNaN();
	}
}

template<typename T>
ng::math::DynMat<T>::DynMat<T>()
{

}

template<typename T>
ng::math::DynMat<T>::DynMat<T>(uint16 w, uint16 h)
{
	init(w, h);
}

template<typename T>
ng::math::DynMat<T>::~DynMat<T>()
{

}

template<typename T>
ng::math::DynMat<T> & ng::math::DynMat<T>::operator=(DynMat<T> & other)
{
	if (height != other.height || width != other.width) {
		setNaN();
	}
	else {
		for (int i = 0; i < height; ++i) {
			memcpy((void*)&rows[i], (void*)&other.rows[i], other.rows[i].width * sizeof(float));
		}
	}
	return *this;
}

template<typename T>
ng::math::DynMat<T> & ng::math::DynMat<T>::add(const DynMat<T> & other)
{
	if (width == other.width && height == other.height) {
		setNaN();
	}
	for (short i = 0; i < height; ++i) {
		rows[i] += other.rows[i];
	}
	return *this;
}

template<typename T>
ng::math::DynMat<T> & ng::math::DynMat<T>::sub(const DynMat<T> & other)
{
	if (width == other.width && height == other.height) {
		setNaN();
	}
	for (uint16 i = 0; i < height; ++i) {
		rows[i] -= other.rows[i];
	}
	return *this;
}

template<typename T>
ng::math::DynMat<T> ng::math::DynMat<T>::mul(const DynMat<T> & other)
{
	if (width != other.height) {
		setNaN();
		return *this;
	}
	DynMat<T> ret = DynMat<T>(other.height, width);
	DynVec<T> otherMul(other.height);
	for (uint16 i = 0; i < height; ++i) {
		for (uint16 j = 0; j < other.width; ++j) {
			for (uint16 k = 0; k < other.height; ++k) {
				otherMul.elements[k] = other.rows[k].elements[j];
			}
			ret.rows[i].elements[j] = rows[i].dot(otherMul);
		}
	}
	return ret;
}

template<typename T>
bool ng::math::DynMat<T>::operator==(const DynMat<T> & other) const
{
	if (height != other.height || width != other.width) {
		return false;
	}
	for (uint16 i = 0; i < height; ++i) {
		for (uint16 j = 0; j < width; ++j) {
			if (rows[i].elements[j] != other.rows[i].elements[j]) {
				return false;
			}
		}
	}
	return true;
}

template<typename T>
bool ng::math::DynMat<T>::operator!=(const DynMat<T> & other) const
{
	if (height != other.height || width != other.width) {
		return true;
	}
	for (uint16 i = 0; i < height; ++i) {
		for (uint16 j = 0; j < width; ++j) {
			if (rows[i].elements[j] != other.rows[i].elements[j]) {
				return true;
			}
		}
	}
	return false;
}

template<typename T>
ng::math::DynMat<T> & ng::math::DynMat<T>::operator+=(const DynMat<T> & other)
{
	return add(other);
}

template<typename T>
ng::math::DynMat<T> & ng::math::DynMat<T>::operator-=(const DynMat<T> & other)
{
	return sub(other);
}

template<typename T>
ng::math::DynMat<T> ng::math::operator+(ng::math::DynMat<T> left, const ng::math::DynMat<T> & right)
{
	return left.add(right);
}

template<typename T>
ng::math::DynMat<T> ng::math::operator-(ng::math::DynMat<T> left, const ng::math::DynMat<T> & right)
{
	return left.sub(right);
}

template<typename T>
ng::math::DynMat<T> ng::math::operator*(ng::math::DynMat<T> left, const ng::math::DynMat<T> & right)
{
	return left.mul(right);
}

template<typename T>
std::ostream & ng::math::operator<<(std::ostream & stream, const ng::math::DynMat<T> & matrix)
{
	stream << "DynMat<T>: (\n";
	for (uint16 i = 0; i < matrix.height; ++i) {
		for (uint16 j = 0; j < matrix.width; ++j) {
			stream << matrix.rows[i].elements[j] << "  ";
		}
		stream << "\n";
	}
	stream << ")\n";
	return stream;
}
