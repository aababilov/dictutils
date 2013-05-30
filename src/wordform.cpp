#include <config.h>

#include "wordform.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <stdarg.h>

#include <vector>

#include "utils.hpp"

using namespace std;

enum {
	DEPTH = 2
};

string_t word_prefix(const string_t &word)
{
	return word.length() > DEPTH ? word.substr(0, DEPTH) : word;
}

bool wordset_t::have_word(const string_t &word)
{
	return words[word_prefix(word)].count(word) > 0;
}

void wordset_t::add(const string_t &word)
{
	words[word_prefix(word)].insert(word);
}

void wordset_t::print()
{
	for (std::map<string_t, std::set<string_t> >::iterator i = words.begin(); i
			!= words.end(); ++i) {
		for (std::set<string_t>::iterator j = i->second.begin(); j
				!= i->second.end(); ++j) {
			printf("%s ", j->c_str());
		}
		printf("\n");
	}
}

string_t wordset_t::find_approx(const string_t &word, int dist)
{
	set<string> &l2_voc = words[word_prefix(word)];
	int best_dist = -1;
	string_t best_word;
	for (set<string>::iterator i = l2_voc.begin(); i != l2_voc.end(); ++i) {
		int dist = word_distance(*i, word);
		if (best_dist == -1 || dist < best_dist || (dist == best_dist
				&& best_word.length() < i->length())) {
			best_word = *i;
			best_dist = dist;
			if (best_dist <= 1) {
				return best_word;
			}
		}
	}
	//printf("`%s' and `%s'\n", word.c_str(), best_word.c_str());
	return looks_same(best_word, word) ? best_word : "";
}

void WordForm::add(const string_t &word)
{
	base_forms.add(word);
}

void WordForm::add(const string_t &base, const string_t &form)
{
	by_form[form] = base;
	by_base.insert(make_pair(base, form));
	var_forms.add(form);
	//printf("add %s %s\n", base.c_str(), form.c_str());
}

void WordForm::add_pair(const string_t &k_word, string_t trans_part)
{
	if (k_word.empty() || trans_part.empty())
		return;
	//printf("pair %s %s\n", k_word.c_str(), trans_part.c_str());
	char *saveptr;
	const char *delim = " \t\n\v";
	char *token = strtok_r(&trans_part[0], delim, &saveptr);
	base_forms.add(k_word);
	while (token) {
		for (int i = 0; token[i]; ++i) {
			token[i] = tolower(token[i]);
		}
		add(k_word, token);
		//printf("added %s %s\n", k_word.c_str(),  p);
		token = strtok_r(NULL, delim, &saveptr);
	}
}

void WordForm::load(const string_t &filename)
{
	by_form.clear();

	FILE *f_wordform = fopen(filename.c_str(), "rt");
	if (f_wordform == NULL)
		return;

	string_t k_word, trans_part;
	char s[1024 * 4];
	while (!feof(f_wordform) && fgets(s, sizeof(s), f_wordform) != NULL) {
		if (s[0] == 0)
			continue;
		if (s[0] == '~') {
			//this is the key
			add_pair(k_word, trans_part);
			char *p = &s[3];
			while (*p && like_letter(*p))
				++p;
			*p = 0;
			k_word = &s[1];
			trans_part = "";
		} else {
			trans_part += s;
		}
	}
	add_pair(k_word, trans_part);
	fclose(f_wordform);
}

bool WordForm::is_wordform(const string_t &word)
{
	return by_form.count(word) > 0;
}

bool has_end(const string_t &a, const string_t &b)
{
	if (a.length() < b.length())
		return false;
	return a.compare(a.length() - b.length(), b.length(), b) == 0;
}
class endings_t {
public:
	static void add(vector<string_t> &vec, va_list vl)
	{
		for (;;) {
			char *s = va_arg(vl, char*);
			if (s == NULL)
				break;
			vec.push_back(s);
		}
	}

	void add_var(const char_t *s, ...)
	{
		va_list vl;
		va_start(vl, s);
		var.clear();
		var.push_back(s);
		add(var, vl);
		va_end(vl);
	}
	void add_base(const char_t *s, ...)
	{
		va_list vl;
		va_start(vl, s);
		base.clear();
		base.push_back(s);
		add(base, vl);
		va_end(vl);
	}
	void clear()
	{
		var.clear();
		base.clear();
	}

	vector<string_t> var, base;
};

vector<endings_t> all_endings;

void prepare_endings()
{
	if (all_endings.size() > 0)
		return;

	endings_t endings;
	endings.add_base("are", "ire", "ere", NULL);
	endings.add_var("o", "i", "iamo", NULL);
	all_endings.push_back(endings);

	endings.add_base("ire", "ere", NULL);
	endings.add_var("e", "ite", "endo", "ente", NULL);
	all_endings.push_back(endings);

	endings.add_base("are", NULL);
	endings.add_var("a", "ate", "ando", "ante","ato",NULL);
	all_endings.push_back(endings);

	endings.add_base("are", "ere", NULL);
	endings.add_var("ero", "erai", "era", "eremo", "erete", "eranno", NULL);
	all_endings.push_back(endings);

	endings.add_base("ire", NULL);
	endings.add_var("ite","ito", "isco", "isci", "isce", "iscono", "iro", "irai",
			"ira", "iremo", "irete", "iranno",NULL);
	all_endings.push_back(endings);

	endings.add_base("ere", NULL);
	endings.add_var("ete","uto", NULL);
	all_endings.push_back(endings);
}

WordForm::WordForm()
{
	prepare_endings();
}

string_t WordForm::get_base(const string_t &word)
{
	//"à""é""ù"
	//word.rep
	//printf("%s\n", word.c_str());
	if (base_forms.have_word(word))
		return word;
	//printf("not base\n");
	if (var_forms.have_word(word))
		return by_form[word];

	string_t s;
	string_t first = word.substr(0, word.length() - 1);
	char_t last = word[word.length() - 1];

	if (last == 'a' || last == 'i') {
		if (base_forms.have_word(s = first + 'o'))
			return s;
		if (base_forms.have_word(s = first + 'e'))
			return s;
	}
	if (last == 'e') {
		if (base_forms.have_word(s = first + 'o'))
			return s;
		if (base_forms.have_word(s = first + 'a'))
			return s;
	}

	for (size_t i = 0; i < all_endings.size(); ++i) {
		for (size_t k = 0; k < all_endings[i].var.size(); ++k) {
			if (has_end(word, all_endings[i].var[k])) {
				first = word.substr(0, word.length()
						- all_endings[i].var[k].length());
				for (size_t j = 0; j < all_endings[i].base.size(); ++j) {
					if (base_forms.have_word(s = first + all_endings[i].base[j]))
						return s;
				}
			}
		}
	}

	s = base_forms.find_approx(word, 3);
	if (!s.empty())
		return s;

	s = var_forms.find_approx(word, 3);
	if (!s.empty())
		return by_form[s];

	return "";
}
