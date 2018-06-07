#include "fft.h"

void ng::math::FFT::transform()
{
	unsigned int N = Array.size();
	unsigned int k = N;
	unsigned int n;

	//DFT
	double thetaT = 3.14159265358979323846264338328L / N;
	Complex phiT = Complex(cos(thetaT), -sin(thetaT));
	Complex T;
	Complex temp;
	while (k > 1) {
		n = k;
		k >>= 1;
		phiT = phiT * phiT;
		T = 1.0L;
		for (unsigned int l = 0; l < k; ++l) {
			for (unsigned int a = l; a < N; a += n) {
				unsigned int b = a + k;
				temp = Array[a] - Array[b];
				Array[a] += Array[b];
				Array[b] = temp * T;
			}
			T *= phiT;
		}
	}
	
	unsigned int m = (unsigned int)log2(N);
	for (unsigned int a = 0; a < N; ++a) {
		unsigned int b = a;
		//Reverse bits
		b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
		b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
		b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
		b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
		b = ((b >> 16) | (b << 16)) >> (32 - m);
		if (b > a) {
			temp = Array[a];
			Array[a] = Array[b];
			Array[b] = temp;
		}
	}
	//fourier done

}

void ng::math::FFT::transform(ComplexArray& arr)
{
	unsigned int N = arr.size();
	unsigned int k = N;
	unsigned int n;

	//DFT
	double thetaT = 3.14159265358979323846264338328L / N;
	Complex phiT = Complex(cos(thetaT), -sin(thetaT));
	Complex T;
	Complex temp;
	while (k > 1) {
		n = k;
		k >>= 1;
		phiT = phiT * phiT;
		T = 1.0L;
		for (unsigned int l = 0; l < k; ++l) {
			for (unsigned int a = l; a < N; a += n) {
				unsigned int b = a + k;
				temp = arr[a] - arr[b];
				arr[a] += arr[b];
				arr[b] = temp * T;
			}
			T *= phiT;
		}
	}

	unsigned int m = (unsigned int)log2(N);
	for (unsigned int a = 0; a < N; ++a) {
		unsigned int b = a;
		//Reverse bits
		b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
		b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
		b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
		b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
		b = ((b >> 16) | (b << 16)) >> (32 - m);
		if (b > a) {
			temp = arr[a];
			arr[a] = arr[b];
			arr[b] = temp;
		}
	}
	//fourier done
}

void ng::math::FFT::inverseTransform()
{
	Array = Array.apply(std::conj);
	transform();
	Array = Array.apply(std::conj);
	Array /= Array.size();
	//inverse fourier done
}

void ng::math::FFT::inverseTransform(ComplexArray& arr)
{
	arr = arr.apply(std::conj);
	transform(arr);
	arr = arr.apply(std::conj);
	arr /= arr.size();
	//inverse fourier done
}

int ng::math::FFT::getSizeFFT()
{
	return Array.size();
}

ng::math::FFT::FFT(ComplexArray arr)
	: Array(arr)
{

}

std::ostream & ng::math::operator<<(std::ostream & stream, const FFT & fourier)
{
	stream << "FFT: (";
	for (int i = 0; i < fourier.Array.size(); ++i) {
		stream << fourier.Array[i] << " ";
	}
	stream << ")";
	return stream;
}
