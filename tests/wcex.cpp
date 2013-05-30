#include <cstring>
#include <cstdlib>
#include <string>
#include <set>

#include "utils.hpp"
#include "vocabulary.hpp"
#include "wordform.hpp"
#include "wordreader.hpp"

using namespace std;

int main(int argc, char* argv[])
{
	int total_words = 0;
	map<string_t, int> words_t;
	map<string_t, set<string_t> > word_forms;
	bool full_out = false;
	
	if (argc > 1 && strcmp(argv[1], "-f") == 0) {
		full_out = true;
	}

	Vocabulary voc;
	WordForm wordform;
	#define  kw wordform

	voc.dump_words(kw);
	wordform.dump_words();
	
	WordReader reader(stdin);
        while (!reader.eof()) {
		string_t orig_word = reader.get_word(), word;
		if (orig_word.empty())
			continue;
		word = orig_word;
		//printf("read: %s\n", word.c_str());
		string_t tmp = kw.find(word);
		if (!tmp.empty()) {
			//printf("kw: %s\n", tmp.c_str());
			word = tmp;
		}
		tmp = wordform.get_base(word);
		if (!tmp.empty()) {
			//printf("wf: %s\n", tmp.c_str());
			word = tmp;
		}
		
		++words_t[word];		
		word_forms[word].insert(orig_word);
		++total_words;
		if (total_words % 10000 == 0) {
			fprintf(stderr, "read %d words\n", total_words);
		}
	}
        fprintf(stderr, "read %d words\n", total_words);
	
        multimap<int, string_t> ord_words;
        string_t prev, word, word_list;
        int occur = 0;
        map<string_t, int>::iterator last;
        for (map<string_t, int>::iterator w = words_t.begin();
             w != words_t.end();
             ++w) {
		occur = w->second;
		word = w->first;
		
		ord_words.insert(make_pair(occur, word));
        }

	for (multimap<int, string_t>::reverse_iterator w = ord_words.rbegin();
	     w != ord_words.rend();
	     ++w) {
		word = w->second;
		occur = w->first;
		printf("%s\n", word.c_str());
	}

	if (full_out) {
		freopen("wcex.occur.txt", "w+t", stdout);
		for (multimap<int, string_t>::reverse_iterator w = ord_words.rbegin();
		     w != ord_words.rend();
		     ++w) {
			word = w->second;
			occur = w->first;
			printf("%s\t%d\t%.3lf%%\n",
			       word.c_str(),
			       occur,
			       occur * 100.0 / total_words);
		}


		freopen("wcex.althabet.txt", "w+t", stdout);
		for (map<string_t, int>::iterator w = words_t.begin();
		     w != words_t.end();
		     ++w) {
			word = w->first;
			occur = w->second;
			printf("%s\t%d\t%.3lf%%\n",
			       word.c_str(),
			       occur,
			       occur * 100.0 / total_words);
		}

		freopen("wcex.wordform.txt", "w+t", stdout);
		for (map<string_t, set<string_t> >::iterator i = word_forms.begin();
		     i != word_forms.end();
		     ++i) {
			set<string_t> &s = i->second;
			if (s.size() <= 1)
				continue;
			printf("~%s\n", i->first.c_str());
			for (set<string_t>::iterator j = s.begin();
			     j != s.end();
			     ++j) {
				printf("\t%s\n", j->c_str());
			}
		}
	}
	fclose(stdout);
        return 0;
}
