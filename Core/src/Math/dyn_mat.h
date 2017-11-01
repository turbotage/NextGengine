#pragma once

#include "dyn_vec.h"

namespace ng {
	namespace math {
		struct DynMat
		{
			unsigned short height;
			unsigned short width;
			DynVec* rows = nullptr;

			void init(unsigned short w, unsigned short h);
			void cleanup();
			void setNaN();

			DynMat();
			DynMat(unsigned short w, unsigned short h);
			~DynMat();

			DynMat& operator=(DynMat& other);

			DynMat& add(const DynMat& other);
			DynMat& sub(const DynMat& other);
			DynMat mul(const DynMat& other);

			friend DynMat operator+(DynMat left, const DynMat& right);
			friend DynMat operator-(DynMat left, const DynMat& right);
			friend DynMat operator*(DynMat left, const DynMat& right);

			bool operator==(const DynMat& other) const;
			bool operator!=(const DynMat& other) const;

			DynMat& operator+=(const DynMat& other);
			DynMat& operator-=(const DynMat& other);

			friend std::ostream& operator<<(std::ostream& stream, const DynMat& matrix);

		};
	}
}

