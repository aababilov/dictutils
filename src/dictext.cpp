#include <config.h>

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <string>
#include <set>

#include "utils.hpp"
#include "vocabulary.hpp"
#include "wordform.hpp"
#include "wordreader.hpp"

using namespace std;

int total_words = 0;

map<string_t, set<string_t> > word_forms;
set<string_t> not_found_words, found_words;
map<string_t, int> alph_words;

void print_wordform()
{
	freopen((settings.in_file_name + ".wordform.txt").c_str(), "w+t", stdout);
	for (map<string_t, set<string_t> >::iterator i = word_forms.begin(); i
			!= word_forms.end(); ++i) {
		set < string_t > &s = i->second;
		printf("~%s\n", i->first.c_str());
		for (set<string_t>::iterator j = s.begin(); j != s.end(); ++j) {
			printf("\t%s\n", j->c_str());
		}
	}
}

void print_notfound()
{
	freopen((settings.in_file_name + ".notfound.txt").c_str(), "w+t", stdout);
	printf("Not found (%d):\n", not_found_words.size());
	for (set<string>::iterator w = not_found_words.begin(); w
			!= not_found_words.end(); ++w) {
		printf("%s\n", w->c_str());
	}
}

void print_occur(const string_t &word, int occur)
{
	printf("%s\t%d\t%.3lf%%\n", word.c_str(), occur,
			occur * 100.0 / total_words);
}

void do_trans_wc()
{
	Vocabulary voc;
	WordForm wordform;

	voc.load(settings.voc_file_name);
	wordform.load(settings.wf_file_name);
	voc.dump_words(wordform);

	bool by_alth = false;
	wordreader_t reader(stdin);
	while (!reader.eof()) {
		string cap_orig_word = reader.get_word();
		if (cap_orig_word.empty())
			continue;
		string orig_word = tolower(cap_orig_word), word = wordform.get_base(
				orig_word);
		if (word.empty()) {
			word = orig_word;
		} else {
			if (word != orig_word)
				word_forms[word].insert(orig_word);
		}

		if (settings.work_mode == MODE_TRANS) {
			if (found_words.count(word) == 0) {
				if (voc.contains(word)) {
					found_words.insert(word);
					if (!by_alth) {
						printf("%s\t%s\n", word.c_str(),
								voc.get_trans(word).c_str());
					}
				} else {
					not_found_words.insert(cap_orig_word);
				}
			}
		} else if (settings.work_mode == MODE_WC) {
			++alph_words[word];
		}
		++total_words;
		if (settings.verbose) {
			if (total_words % 1000 == 0) {
				fprintf(stderr, "read %d words\n", total_words);
			}
		}
	}
	if (settings.verbose) {
		fprintf(stderr, "total: read %d words\n", total_words);
	}

	if (settings.work_mode == MODE_TRANS) {
		if (by_alth) {
			for (set<string>::iterator w = found_words.begin(); w
					!= found_words.end(); ++w) {
				printf("%s\t%s\n", w->c_str(), voc.get_trans(*w).c_str());
			}
		}

		if (settings.full_out) {
			print_notfound();
			print_wordform();
		}
	} else if (settings.work_mode == MODE_WC) {
		multimap<int, string_t> ord_words;
		string_t word;
		int occur;
		map<string_t, int>::iterator last;
		for (map<string_t, int>::iterator w = alph_words.begin(); w
				!= alph_words.end(); ++w) {
			ord_words.insert(make_pair(w->second, w->first));
		}

		for (multimap<int, string_t>::reverse_iterator w = ord_words.rbegin(); w
				!= ord_words.rend(); ++w) {
			occur = w->first;
			printf("%s\n", w->second.c_str());
		}

		if (settings.full_out) {
			freopen((settings.in_file_name + ".occur.txt").c_str(), "w+t",
					stdout);
			for (multimap<int, string_t>::reverse_iterator w =
					ord_words.rbegin(); w != ord_words.rend(); ++w) {
				word = w->second;
				occur = w->first;
				printf("%s\t%d\t%.3lf%%\n", word.c_str(), occur,
						occur * 100.0 / total_words);
			}

			freopen((settings.in_file_name + ".alphabet.txt").c_str(), "w+t",
					stdout);
			for (map<string_t, int>::iterator w = alph_words.begin(); w
					!= alph_words.end(); ++w) {
				word = w->first;
				occur = w->second;
				printf("%s\t%d\t%.3lf%%\n", word.c_str(), occur,
						occur * 100.0 / total_words);
			}

			print_wordform();
		}
	}
}

void do_highlight()
{
	WordForm wordform;
	set < string_t > hstrs;
	string h_start = "<b>", h_end = "</b>";
	wordform.load(settings.wf_file_name);

	wordreader_t reader(fopen(settings.hl_file_name.c_str(), "rt"));

	while (!reader.eof()) {
		string orig_word = tolower(reader.get_word());
		if (orig_word.empty())
			continue;
		hstrs.insert(orig_word);
		pair < WordForm::by_base_map_t::iterator, WordForm::by_base_map_t::iterator
				> forms = wordform.get_by_base().equal_range(orig_word);
		for (WordForm::by_base_map_t::iterator i = forms.first; i
				!= forms.second; ++i) {
			hstrs.insert(i->second);
		}
	}

	char *s = NULL;
	size_t s_n = 0;
	while (!feof(stdin)) {
		if (getline(&s, &s_n, stdin) == -1) {
			break;
		}

		char *p = &s[0];
		while (*p && !like_letter(*p)) {
			putchar(*p);
			++p;
		}
		while (*p) {
			string word;
			while (*p && like_letter(*p)) {
				word += *p;
				++p;
			}
			if (hstrs.count(tolower(word))) {
				printf("%s%s%s", h_start.c_str(), word.c_str(), h_end.c_str());
			} else {
				printf("%s", word.c_str());
			}
			while (*p && !like_letter(*p)) {
				putchar(*p);
				++p;
			}
		}
	}
}

int main(int argc, char* argv[])
{
	settings.parse_opts(argc, argv);
	switch (settings.work_mode) {
	case MODE_TRANS:
	case MODE_WC:
		do_trans_wc();
		break;
	case MODE_HIGHLIGHT:
		do_highlight();
	}

	return 0;
}

