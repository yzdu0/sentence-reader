#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include "sentence-reader/Lexicon.h"

Word::Word(std::string word, std::string POS) :
	word(word), POS(POS) {

	if (POS.back() == ':') {
		POS.pop_back();
	}
}
//
//std::string Word::get_POS() {
//	switch (pos) {
//	case Verb:
//		return "V";
//	case Noun:
//		return "N";
//	case Adjective:
//		return "Adj";
//	case Pronoun:
//		return "Pron";
//	case Determiner:
//		return "Det";
//	case Conjunction:
//		return "Conj";
//	case Negation:
//		return "Neg";
//	case AuxillaryVerb:
//		return "Aux";
//	case Adverb:
//		return "Adv";
//	case CopulaVerb:
//		return "Cop";
//	case TO:
//		return "TO";
//	case FOR:
//		return "FOR";
//	}
//}
Lexicon::Lexicon() {
	std::ifstream file("data/vocab.txt");
	std::string str;

	while (std::getline(file, str)) {
		if (str.size() == 0) continue;

		if (str[0] == '#') continue;

		std::stringstream ss(str);
		std::string cur;
		std::vector<std::string> line;
		while (getline(ss, cur, ' ')) {
			if (cur != "|") {
				line.push_back(cur);
			}
		}

		if (line[0] == "N:") {
			create_nouns(line);
		}
		else if (line[0] == "V:") {
			create_verbs(line);
		}
		else if (line[0] == "Adj:") {
			create_adjectives(line);
		}
		else {
			create_misc(line);
		}
	}
}

void Lexicon::create_nouns(std::vector<std::string> input) {
	// We currently care about two forms - singular and plural. 
	// We assume input is the singular form, then tack on an S for the plural.
	// Basically have overloading support for weirder nouns
	std::string base_word = input[1];
	dictionary[base_word] = Word(base_word, input[0]);
	dictionary[base_word].plurality = Word::Plurality::Singular;

	std::string plural_version = find_overload(input, "plural");

	if (plural_version == "-") {
		plural_version = base_word;
		plural_version.push_back('s');
	}

	dictionary[plural_version] = Word(plural_version, input[0]);
	dictionary[plural_version].plurality = Word::Plurality::Plural;
}

void Lexicon::create_verbs(std::vector<std::string> input) {
	std::string base_word = input[1];
	dictionary[base_word] = Word(base_word, input[0]);
}

void Lexicon::create_adjectives(std::vector<std::string> input) {
	std::string base_word = input[1];
	dictionary[base_word] = Word(base_word, input[0]);
}

void Lexicon::create_misc(std::vector<std::string> input) {
	for (std::size_t i = 1; i < input.size(); i++) {
		dictionary[input[i]] = Word(input[i], input[0]);
	}
}

std::string Lexicon::find_overload(const std::vector<std::string>& input, const std::string& key) {
	for (const std::string& token : input) {
		std::string candidate_key = "";
		std::size_t pos = 0;
		for (std::size_t i = 0; i < token.size(); i++) {
			if (token[i] == '=') {
				pos = i + 1;
				break;
			}
			candidate_key.push_back(token[i]);
		}
		std::string res = "";
		if (candidate_key == key) {
			for (std::size_t j = pos; j < token.size(); j++) {
				res.push_back(token[j]);
			}
		}
		return res;
	}
	return "-";
}
/*
TODO:
Verb past tense, present tense, gerund, past participle

Noun plural

Adjective comparative, superlative

Given a string, we should be able to look up the word, and determine its information. 
(At the very least, its part of speech, which we already have.)

When generating words, we should also create all of the related words (e.g. the noun "bird" also 
creates the noun "birds".  ). 
*/