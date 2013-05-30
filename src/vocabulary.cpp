#include <config.h>

#include "vocabulary.hpp"

#include <cstring>
#include <cstdio>
#include <limits>
#include "utils.hpp"

using namespace std;

Vocabulary::Vocabulary()
{
	_max_trans = numeric_limits<int>::max();
}

Translations &Vocabulary::find(const string_t &word)
{
	return _translation_map[word];
}

bool Vocabulary::contains(const string_t &word)
{
	return _translation_map.find(word) != _translation_map.end();
}

string_t Vocabulary::get_trans(const string_t &word)
{
	set < string_t > printed;
	return get_trans_internal(word, printed);
}

string_t Vocabulary::get_trans_internal(const string_t &word,
		std::set<string_t> &printed)
{
	string_t ret;
	Translations bnd = _translation_map[word];
	int tr_count = 0;
	for (Translations::iterator j = bnd.begin(); j != bnd.end(); ++j) {
		if (j != bnd.begin())
			ret += "; ";
		string_t tr = *j;
		//printf("tr: %s\n", tr.c_str());		
		if (tr[0] == '\1') {
			tr = tr.substr(1, tr.length() - 1);
			if (printed.count(tr) == 0) {
				printed.insert(tr);
				ret += "(" + tr + " - " + get_trans_internal(tr, printed) + ")";
			}
		} else {
			ret += tr;
		}
		++tr_count;
		if (tr_count >= _max_trans)
			break;
	}
	return ret;
}

string first_word(string w)
{
	//printf("fw `%s'\n", w.c_str());
	string ret;
	int i = 0;
	for (; w[i] && !like_letter(w[i]); ++i)
		;
	for (; w[i]; ++i) {
		if (strchr(",;", w[i]) == NULL) {
			ret += w[i];
		} else {
			break;
		}
	}
	//printf("===irupt: %c(0x%x)\nfw `%s'\n", w[i], 0xff & w[i], ret.c_str());
	return ret;
}

string cut_tags(char *s)
{
	string ret;
	const char *stress = "<nu />'<nu />";
	//printf("got `%s'\n", s);
	for (char *p = s; *p; ++p) {
		if (strncmp(p, stress, strlen(stress)) == 0) {
			p += strlen(stress) - 1;
		} else if (*p == '<') {
			string tag = "</";
			++p;
			while (*p) {
				tag += *p;
				if (*p == '>')
					break;
				++p;
			}
			//printf("tag `%s'\n", tag.c_str());
			p = strstr(p, tag.c_str());
			if (!p)
				break;
			p += tag.length() - 1;
		} else {
			ret += *p;
		}
	}
	return ret;
}

void Vocabulary::add_pair(const string_t &k_word, string_t trans_part)
{
	if (k_word.empty())
		return;
	int tr_count = 0;
	set < string_t > added;
	char *p = &trans_part[0];
	std::set < string_t > &word_trans = _translation_map[k_word];
	while ((p = strstr(p, "<dtrn>"))) {
		p += 6;
		char *t = strstr(p, "</dtrn>");
		if (!t)
			break;
		*t = 0;
		//printf("p=%s\n",p);
		string dtrn = p;
		string tr = first_word(cut_tags(p));
		if (!tr.empty()) {
			if (added.count(tr) == 0) {
				++tr_count;
				added.insert(tr);
				word_trans.insert(tr);
				if (word_trans.size() >= _max_trans)
					break;
				//printf("ins: %s %s\n", k_word.c_str(), tr.c_str());
			}
		} else {
			char_t *p = &dtrn[0];
			if ((p = strstr(p, "<kref>"))) {
				p += 6;
				char *t = strstr(p, "</kref>");
				if (!t)
					continue;
				*t = 0;
				if (!*p)
					continue;
				tr = string("\1") + p;
				word_trans.insert(tr);
			}
		}
		p = t + 7;
	}
	//printf("voc: %s\n", k_word.c_str());
}

void Vocabulary::load(const string_t &filename)
{
	FILE *f_voc = fopen(filename.c_str(), "rt");
	if (f_voc == NULL)
		return;
	load(f_voc);
	fclose(f_voc);
}

void Vocabulary::load(FILE *f_voc)
{
	char s[1024 * 4];
	string_t k_word, trans_part;
	int total_words = 0;

	while (!feof(f_voc) && fgets(s, sizeof(s), f_voc) != NULL) {
		if (s[0] == 0)
			continue;
		char *k_pos;
		if ((k_pos = strstr(s, "<k>")) != NULL) {
			//this is the key
			*k_pos = 0;
			add_pair(k_word, trans_part + s);
			char *p = k_pos + 3;
			while (*p && strncmp(p, "</k>", 4) != 0)
				++p;
			*p = 0;
			k_word = &k_pos[3];
			trans_part = "";
			if (settings.verbose) {
				++total_words;
				if (total_words % 10000 == 0) {
					fprintf(stderr, "voc: read %d words\n", total_words);
				}
			}
		} else {
			trans_part += s;
		}
	}
	add_pair(k_word, trans_part);
}
