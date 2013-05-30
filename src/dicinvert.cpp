#include <config.h>

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <string>
#include <set>

#include <arpa/inet.h>

#include "utils.hpp"
#include "vocabulary.hpp"
#include "wordform.hpp"
#include "wordreader.hpp"

using namespace std;

void print_set(FILE *f_inv, Vocabulary &voc, Translations &trans,
		Translations &occured_words)
{
	int trans_no = 0;
	FOR_EACH (Translations, j, trans) {
		string_t translation = *j;
		if (occured_words.count(translation) > 0) {
			continue;
		}
		occured_words.insert(translation);
		if (trans_no > 0) {
			fprintf(f_inv, ", ");
		}
		fprintf(f_inv, "%s", translation.c_str());
		occured_words.insert(translation);
		TranslationMap::iterator ref = voc.translation_map().find(
				'\1' + translation);
		if (ref != voc.translation_map().end()) {
			fprintf(f_inv, ", ");
			print_set(f_inv, voc, ref->second, occured_words);
		}
		++trans_no;
	}
}

string_t trim(const string_t &s)
{
	const char *p = &s[0];
	while (isspace(*p)) {
		++p;
	}
	if (*p != '\0') {
		const char *pe = &s[s.length() - 1];
		while (pe > p && isspace(*pe)) {
			--pe;
		}
		return string_t(p, pe + 1);
	}
	return string_t();
}

int main(int argc, char **argv)
{
	Vocabulary voc;
	FILE *f_input = argc > 1 ? fopen(argv[1], "rt") : stdin;
	voc.load(f_input);
	Vocabulary voc_inverted;
	FOR_EACH (TranslationMap, i, voc.translation_map()) {
		Translations &trans = i->second;
		string_t orig_word = trim(i->first);
		//printf("word %s\n", orig_word.c_str());
		FOR_EACH (Translations, j, trans) {
			//printf("\t%s\n", j->c_str());
			voc_inverted.translation_map()[trim(j->c_str())].insert(orig_word);
		}
	}
	fclose(f_input);

	FILE *f_inv = argc > 2 ? fopen(argv[2], "wb") : stdout;
	FILE *f_idx;
	if (argc > 2) {
		f_idx = fopen((string(argv[2]) + ".idx").c_str(), "wb");
	} else {
		f_idx = fopen("dictionary.idx", "wb");
	}
	int total_article = 0;
	FOR_EACH (TranslationMap, i, voc_inverted.translation_map()) {
		Translations &trans = i->second;
		string_t orig_word = i->first;
		if (orig_word[0] == '\1') {
			continue;
		}
		fprintf(f_inv, "<k>%s</k>\n", orig_word.c_str());
		long start_article = ftell(f_inv);
		fprintf(f_inv, "<dtrn>");
		Translations occured_words;
		print_set(f_inv, voc_inverted, trans, occured_words);
		fprintf(f_inv, "</dtrn>");
		long end_article = ftell(f_inv);
		uint32_t tmpuint32 = htonl(start_article);
		fwrite(orig_word.c_str(), 1, orig_word.length() + 1, f_idx);
		fwrite(&tmpuint32, 4, 1, f_idx);
		tmpuint32 = htonl(end_article - start_article);
		fwrite(&tmpuint32, 4, 1, f_idx);
		++total_article;
	}
	fclose(f_inv);
	fclose(f_idx);
	fprintf(stderr, "total: %d\n", total_article);
	return 0;
}
