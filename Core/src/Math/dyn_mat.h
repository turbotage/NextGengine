#pragma once

#include "dyn_vec.h"

namespace ng {
	namespace math {

		template<typename T>
		class DynMat
		{
			uint16 height;
			uint16 width;
			DynVec<T>* rows = nullptr;

			void init(uint16 w, uint16 h);
			void cleanup();
			void setNaN();

			DynMat();
			DynMat(uint16 w, uint16 h);
			~DynMat();

			DynMat& operator=(DynMat<T>& other);
			 
			DynMat& add(const DynMat<T>& other);
			DynMat& sub(const DynMat<T>& other);
			DynMat mul(const DynMat<T>& other);

			friend DynMat operator+(DynMat<T> left, const DynMat<T>& right);
			friend DynMat operator-(DynMat<T> left, const DynMat<T>& right);
			friend DynMat operator*(DynMat<T> left, const DynMat<T>& right);

			bool operator==(const DynMat<T>& other) const;
			bool operator!=(const DynMat<T>& other) const;

			DynMat& operator+=(const DynMat<T>& other);
			DynMat& operator-=(const DynMat<T>& other);

			friend std::ostream& operator<<(std::ostream& stream, const DynMat<T>& matrix);

		};

	}
}

