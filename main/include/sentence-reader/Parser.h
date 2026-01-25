#pragma once


#include <string>
#include <vector>
#include "sentence-reader/Rule.h"
#include <unordered_map>
#include <set>
#include <unordered_set>
class Parser {
public:
	Parser();
	bool parse(
		const std::vector<std::string>& sentence, 
		const std::vector<std::string>& expected);


	
private:
	std::vector<Rule> rules;
	std::vector<Rule> words;

	std::unordered_set<std::string> wordbank;


	struct Record {
		std::vector<std::string> sentence;
		std::vector<std::string> expected;

		bool operator==(Record const& other) const noexcept {
			return sentence == other.sentence && expected == other.expected;
		}
	};

	static inline void hash_combine(std::size_t& seed, std::size_t value) noexcept {
		seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
	}

	struct RecordHash {
		std::size_t operator()(Record const& r) const noexcept {
			std::size_t seed = 0;
			hash_combine(seed, r.sentence.size());
			for (auto const& s : r.sentence) {
				hash_combine(seed, std::hash<std::string>{}(s));
			}
			hash_combine(seed, r.expected.size());
			for (auto const& s : r.expected) {
				hash_combine(seed, std::hash<std::string>{}(s));
			}
			return seed;
		}
	};

	std::unordered_map<Record, bool, RecordHash> cache;

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

	enum CacheResult {
		exists_true,
		exists_false,
		nowhere,
	};

	CacheResult lookup_cache(
		const std::vector<std::string>& sentence,
		const std::vector<std::string>& expected) const;

	void add_to_cache(
		const std::vector<std::string>& sentence,
		const std::vector<std::string>& expected, bool res);
};