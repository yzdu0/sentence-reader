#include "sentence-reader/SyntaxTree.h"
#include "sentence-reader/Util.h"
#include <vector>
#include <string>
#include <iostream>

SyntaxTree::SyntaxTree() {

};

SyntaxTree::SyntaxTree(const std::string& head) {
	TreeNode temp;
	temp.label = head;
	temp.parent_index = 0;

	nodes.push_back(temp);
};

SyntaxTree::SyntaxTree(const std::string& head, const std::string& word) {
	TreeNode temp;
	temp.label = head;
	temp.parent_index = 0;
	temp.word = word;

	nodes.push_back(temp);
};

void SyntaxTree::addChildRight(const std::string& label) {
	TreeNode temp;
	temp.label = label;
	temp.parent_index = 0;

	nodes.push_back(temp);
}

void SyntaxTree::addTreeRight(const SyntaxTree& child) {
	if (child.nodes.empty()) return;

	std::size_t offset = nodes.size();

	nodes.insert(nodes.end(), child.nodes.begin(), child.nodes.end());

	for (std::size_t i = offset; i < nodes.size(); ++i) {
		nodes[i].parent_index += offset;
	}

	nodes[offset].parent_index = 0;
}

SyntaxTree& SyntaxTree::operator+=(const SyntaxTree& other) {
	addTreeRight(other);
	return *this;
}

SyntaxTree SyntaxTree::operator+(const SyntaxTree& other) const {
	SyntaxTree result(*this); 
	result += other;
	return result;           
}

void SyntaxTree::simple_print() {
	for (std::size_t i = 0; i < nodes.size(); i ++) {
		std::cout << "Node " << i << ": " << nodes[i].label << " with parent " << nodes[i].parent_index << "\n";
	}
	std::cout << "\n";
}

void SyntaxTree::updateAdjacency() {
	adj.resize(nodes.size());
	for (std::size_t i = 1; i < nodes.size(); i++) {
		adj[nodes[i].parent_index].push_back(i);
	}
}
//
void SyntaxTree::display() {
	updateAdjacency();
	dfs(1, 0);
}

void SyntaxTree::dfs(std::size_t u, int depth) const {
	Util::print_stack_depth(depth);
	std::cout << nodes[u].label;
	if (nodes[u].word.size() > 0) std::cout << " -> " << nodes[u].word;
	std::cout << "\n";

	for (const std::size_t v : adj[u]) {
		dfs(v, depth + 1);
	}
}

/*
I saw the man with my telescope
I never 

*/