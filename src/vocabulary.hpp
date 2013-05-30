#ifndef VOCABULARY_HPP
#define VOCABULARY_HPP

#include <string>
#include <map>
#include <set>

#include "utils.hpp"

typedef std::set<string_t> Translations;
typedef std::map<string_t, Translations> TranslationMap;

class Vocabulary {
public:
	Vocabulary();

	Translations &find(const string_t &word);
	string_t get_trans(const string_t &word);
	bool contains(const string_t &word);
	void load(const string_t &filename);
	void load(FILE *f_voc);

	int &max_trans() { return _max_trans; }
	int max_trans() const { return _max_trans; }

	TranslationMap &translation_map() { return _translation_map; }

	template<class T>
	void dump_words(T &kw)
	{
		FOR_EACH (TranslationMap, i, _translation_map) {
			kw.add(i->first);
		}
	}
private:	
	void add_pair(const string_t &k_word, string_t trans_part);
	string_t get_trans_internal(const string_t &word,
						  std::set<string_t> &printed);

	int _max_trans;
	TranslationMap _translation_map;
};

#endif
