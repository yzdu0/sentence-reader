#pragma once
#include <vector>
#include <memory>
#include <string>

class SyntaxTree {
	public:
		struct TreeNode {
			std::string label;
			std::string word = "";
			std::size_t parent_index;
			//std::vector<std::unique_ptr<TreeNode>> children;
		};
		TreeNode root;
		std::vector<TreeNode> nodes;

		std::vector<std::vector<std::size_t>> adj;

		SyntaxTree();

		SyntaxTree(const std::string& head);

		SyntaxTree(const std::string& head, const std::string& word);

		void addChildRight(const std::string& label);

		void addTreeRight(const SyntaxTree& child);

		SyntaxTree& SyntaxTree::operator+=(const SyntaxTree& other);

		SyntaxTree operator+(const SyntaxTree& other) const;

		void simple_print();

		void updateAdjacency();

		void display();

		void dfs(std::size_t u, int depth) const;
};