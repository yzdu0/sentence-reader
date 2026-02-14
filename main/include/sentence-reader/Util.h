#pragma once
#include <vector>
#include <iostream>

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

	static void print_stack_depth(int depth) {
		for (int i = 0; i < depth; i++) {
			std::cout << "|   ";
		}
	}

	
};