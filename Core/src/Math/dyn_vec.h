#pragma once

#include "../def.h"
#include <vector>

namespace ng {
	namespace math {

		template<typename T>
		class DynVec
		{
		public:
			uint16 width;
			T* elements = nullptr;

			DynVec();
			DynVec(uint16 width);
			~DynVec();

			void init(uint16 w);
			void cleanup();
			void setNaN();
			
			DynVec& operator=(DynVec<T>& other);

			DynVec& add(const DynVec<T>& other);
			DynVec& sub(const DynVec<T>& other);
			DynVec& mul(const DynVec<T>& other);
			DynVec& div(const DynVec<T>& other);

			DynVec& mul(float multiplier);
			DynVec& div(float multiplier);

			float dot(const DynVec<T>& other);

			friend DynVec operator+(DynVec<T> left, const DynVec<T>& right);
			friend DynVec operator-(DynVec<T> left, const DynVec<T>& right);
			friend DynVec operator*(DynVec<T> left, const DynVec<T>& right);
			friend DynVec operator/(DynVec<T> left, const DynVec<T>& right);

			friend DynVec operator*(const float& left, DynVec<T> right);
			friend DynVec operator/(DynVec<T> left, const float& right);

			bool operator==(const DynVec<T>& other) const;
			bool operator!=(const DynVec<T>& other) const;

			DynVec& operator+=(const DynVec<T>& other);
			DynVec& operator-=(const DynVec<T>& other);
			DynVec& operator*=(const DynVec<T>& other);
			DynVec& operator/=(const DynVec<T>& other);

			friend std::ostream& operator<<(std::ostream& stream, const DynVec<T>& vector);
		};
	}
}

