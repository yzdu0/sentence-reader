#include <string>
#include <vector>
#include "sentence-reader/Rule.h"
class Parser {
public:
	Parser();
	bool parse(
		const std::vector<std::string>& sentence, 
		const std::vector<std::string>& expected);

private:
	std::vector<Rule> rules;
	std::vector<Rule> words;

	int call_depth = 0;

	bool Predictor(
		const std::vector<std::string>& sentence,
		const std::vector<std::string>& expected) const;

	bool Scanner(
		const std::vector<std::string>& sentence,
		const std::vector<std::string>& expected) const;

	bool Completer(
		const std::vector<std::string>& sentence, 
		const std::vector<std::string>& expected) const;

	void print_depth();

	std::string lookup_left(std::vector<std::string> right) const;
};