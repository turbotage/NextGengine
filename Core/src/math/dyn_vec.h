#pragma once

#include <vector>

namespace ng {
	namespace math {
		struct DynVec
		{
			unsigned short width;
			float* elements = nullptr;

			DynVec();
			DynVec(unsigned short width);
			~DynVec();

			void init(unsigned short w);
			void cleanup();
			void setNaN();
			
			DynVec& add(const DynVec& other);
			DynVec& sub(const DynVec& other);
			DynVec& mul(const DynVec& other);
			DynVec& mul(float multiplier);
			DynVec& div(const DynVec& other);

			float dot(const DynVec& other);

			friend DynVec operator+(DynVec left, const DynVec& right);
			friend DynVec operator-(DynVec left, const DynVec& right);
			friend DynVec operator*(DynVec left, const DynVec& right);
			friend DynVec operator/(DynVec left, const DynVec& right);

			bool operator==(const DynVec& other) const;
			bool operator!=(const DynVec& other) const;

			DynVec& operator+=(const DynVec& other);
			DynVec& operator-=(const DynVec& other);
			DynVec& operator*=(const DynVec& other);
			DynVec& operator/=(const DynVec& other);

			friend std::ostream& operator<<(std::ostream& stream, const DynVec& vector);
		};
	}
}

