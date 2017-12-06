#pragma once

#include <functional>

namespace ng {
	namespace math {
		template <class T>
		inline void hashCombine(std::size_t& seed, const T& v) {
			std::hash<T> hasher;
			seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}

		template <class T>
		inline std::size_t hashCombineMany(std::vector<T>* v) {
			std::hash<T> hasher;
			size_t seed = hasher(v[0]);
			for (int i = 1; i < v.size(); ++i) {
				seed ^= hasher(v[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			return seed;
		}

	}
}

