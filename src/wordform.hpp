#ifndef WORDFORM_HPP
#define WORDFORM_HPP

#include <string>
#include <map>
#include <set>

#include "utils.hpp"

class wordset_t {
public:
	bool have_word(const string_t &word);
	void add(const string_t &word);
	string_t find_approx(const string_t &word, int dist);

	void print();
private:
	std::map<string_t, std::set<string_t> > words;

	enum {
		DEPTH = 2
	};
};


class WordForm {
public:
	typedef std::map<string_t, string_t> by_form_map_t;
	typedef std::multimap<string_t, string_t> by_base_map_t;
public:
	WordForm();

	bool is_wordform(const string_t &word);
	string_t get_base(const string_t &word);

	void add(const string_t &word);
	void add(const string_t &base, const string_t &form);

	void load(const string_t &filename);
	by_base_map_t &get_by_base() {return by_base; }
	by_form_map_t &get_by_form() {return by_form; }
private:
	void add_pair(const string_t &k_word, string_t trans_part);

	by_form_map_t by_form;
	by_base_map_t by_base;
	wordset_t base_forms, var_forms;
};

#endif
