#pragma once

#include <functional>

template <class T>
inline void hashCombine(std::size_t& seed, const T& v) {
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <class T>
inline std::size_t hashCombineMany(std::vector<T>* v) {
	std::hash<T> hasher;

}

