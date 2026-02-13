#pragma once
#include <vector>
#include <memory>
#include <string>

class SyntaxTree {
	struct TreeNode {
		std::string txt;
		std::size_t parent_index;
		//std::vector<std::unique_ptr<TreeNode>> children;
	};
	TreeNode root;

	std::vector<TreeNode> nodes;
public:
	SyntaxTree();

	SyntaxTree(std::string head);

	void addChildRight(std::string txt);

	SyntaxTree operator+(const SyntaxTree& other) const;
};