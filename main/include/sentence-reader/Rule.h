#include <vector>
#include <string>
#include <sstream>
#include <iostream>

class Rule {
public:
	std::string left = "";
	std::vector<std::string> right;

	Rule(std::string input);

	void print() const;

	std::string lookup_left(std::vector<std::string> right);
private:
};