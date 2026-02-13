#pragma once
#include <vector>

class Util {
public:
	template<typename T>
	static std::vector<T> cartesian_product(const std::vector<T> A, const std::vector<T> B) {
		std::vector<T> out;
		for (auto& a : A) {
			for (auto& b : B) {
				out.push_back(a + b);
			}
		}
		return out;
	}
};