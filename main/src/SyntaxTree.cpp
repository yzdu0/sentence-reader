#include "sentence-reader/SyntaxTree.h"
#include <vector>
#include <string>

SyntaxTree::SyntaxTree() {

};

SyntaxTree::SyntaxTree(std::string head) {
	TreeNode temp;
	temp.txt = head;
	temp.parent_index = 0;

	nodes.push_back(temp);
};

void SyntaxTree::addChildRight(std::string txt) {
	TreeNode temp;
	temp.txt = txt;
	temp.parent_index = 0;

	nodes.push_back(temp);
}

SyntaxTree SyntaxTree::operator+(const SyntaxTree& other) const {
	
}