#pragma once
#include <vector>
#include "sentence-reader/Rule.h"
class Grammar {
public:
	Grammar();

	Grammar ChomskyNormalForm(Grammar input);

	std::vector<Rule> rules;

	std::vector<Rule> basic_rules;

	void enumerate();
private:
};