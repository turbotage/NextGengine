#pragma once

#include "../def.h"

#include <complex>
#include <iostream>
#include <valarray>


namespace ng {
	namespace math {

		typedef std::complex<double> Complex;
		typedef std::valarray<Complex> ComplexArray;

		class FFT
		{
		private:
			ComplexArray Array;
		public:

			void transform();
			static void transform(ComplexArray& arr);

			void inverseTransform();
			static void inverseTransform(ComplexArray& arr);

			friend std::ostream& operator<<(std::ostream& stream, const FFT& fourier);

			int getSizeFFT();

			FFT(ComplexArray arr);
		};

	}
}

