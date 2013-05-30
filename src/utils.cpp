#include <config.h>

#include "utils.hpp"

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include <unistd.h>
#include <getopt.h>

using namespace std;

settings_t settings;

settings_t::settings_t()
{
	verbose = false;
	full_out = false;
	work_mode = MODE_TRANS;
}

void usage()
{
	printf("usage: " PACKAGE_NAME " [ OPTIONS ] [ FILE ] \n"
			"\t-m | --mode=[wc,translate,highlight]\t mode\n"
			"\t-w | --wordform=FILE    \t wordform\n"
			"\t-c | --vocabulary=FILE  \t vocabulary\n"
			"\t-i | --highlight=FILE  \t highlight\n"
			"\t-f | --full-out     \t full output\n"
			"\t-a | --alphabet     \t by alphabet\n"
			"\t-v | --verbose     \t verbose\n"
			"\t-V | --version     \t version\n"
			"\t-h | --help        \t help\n");
	exit(0);
}

void settings_t::parse_opts(int argc, char *argv[])
{
	struct option long_options[] = { { "mode", required_argument, NULL, 'm' },
			{ "full-out", no_argument, NULL, 'f' }, { "alphabet", no_argument,
					NULL, 'a' }, { "wordform", required_argument, NULL, 'w' },
			{ "vocabulary", required_argument, NULL, 'c' }, { "highlight",
					required_argument, NULL, 'i' }, { "verbose", no_argument,
					NULL, 'v' }, { "version", no_argument, NULL, 'V' }, {
					"help", no_argument, NULL, 'h' }, { 0, 0, NULL, 0 } };
	char c;
	while ((c = getopt_long(argc, argv, "m:w:c:i:favVh", long_options, NULL))
			!= -1) {
		switch (c) {
		case 'm':
			work_mode = optarg[0] == 'w' ? MODE_WC
					: optarg[0] == 't' ? MODE_TRANS : MODE_HIGHLIGHT;
			break;
		case 'w':
			wf_file_name = optarg;
			break;
		case 'c':
			voc_file_name = optarg;
			break;
		case 'i':
			hl_file_name = optarg;
			work_mode = MODE_HIGHLIGHT;
			break;
		case 'f':
			full_out = true;
			break;
		case 'v':
			verbose = true;
			break;
		case 'V':
			printf(PACKAGE_STRING "\n");
			exit(0);
		case 'h':
			usage();
		}
	}
	if (optind < argc) {
		freopen(argv[optind], "rt", stdin);
		in_file_name = argv[optind];
	} else {
		in_file_name = "stdin";
	}
	if (wf_file_name.empty()) {
		wf_file_name = locate_file("wordform.txt");
	}
	if (voc_file_name.empty()) {
		voc_file_name = locate_file("voc.txt");
	}
	if (hl_file_name.empty()) {
		hl_file_name = locate_file("highlight.txt");
	}
}

string_t settings_t::locate_file(const string_t &name)
{
	const string_t dirs[] = { ".", PKGDATADIR };
	for (int i = 0; i < 3; ++i) {
		string path;
		if (access((path = dirs[i] + "/" + name).c_str(), R_OK) == 0) {
			return path;
		}
	}
	return "";
}

bool like_letter(char_t c)
{
	if (c & 0x80)
		return true;
	if (isdigit(c))
		return false;
	if (isalpha(c))
		return true;
	if (strchr("\"'", c) != NULL)
		return false;
	if (isspace(c) || ispunct(c))
		return false;
	return true;
}

bool isvowel(char_t c)
{
	if (strchr("oaueiy", c) != NULL)
		return true;
	return false;
}

string_t tolower(const string_t &s)
{
	string_t ret;
	ret.resize(s.length());
	for (size_t i = 0; i < s.length(); ++i)
		ret[i] = tolower(s[i]);
	return ret;
}
const int max_dist = 5;

int word_distance(const string_t &a, const string_t &b)
{
	static EditDistance ed;

	return ed.CalEditDistance(a.c_str(), b.c_str(), max_dist);
}

bool looks_same(const string_t &a, const string_t &b)
{
	size_t min_l = min(a.length(), b.length());
	if (min_l > 2 && strncmp(a.c_str(), b.c_str(), min_l - 1) == 0) {
		if (a.length() == b.length() && isvowel(a[a.length() - 1]) && isvowel(
				b[b.length() - 1])) {
			return true;
		}
	}
	size_t d = word_distance(a, b);
	//printf("dist: %d, l %d\n", d, min_l);
	return (d == 1 && min_l > 4) || (d == 2 && min_l > 8) || d <= min_l / 5;
}

#ifndef HAVE_STRTOK_R
extern "C" char *
strtok_r(char *s, const char *delim, char **save_ptr)
{
	char *token;

	if (s == NULL)
		s = *save_ptr;

	/* Scan leading delimiters.  */
	s += strspn(s, delim);
	if (*s == '\0') {
		*save_ptr = s;
		return NULL;
	}

	/* Find the end of the token.  */
	token = s;
	s = strpbrk(token, delim);
	if (s == NULL)
		/* This token finishes the string.  */
		*save_ptr = __rawmemchr(token, '\0');
	else {
		/* Terminate the token and make *SAVE_PTR point past it.  */
		*s = '\0';
		*save_ptr = s + 1;
	}
	return token;
}
#endif /*defined(HAVE_STRTOK_R)*/

/*
 writer : Opera Wang
 E-Mail : wangvisual AT sohu DOT com
 License: GPL
 */

/* filename: distance.cc */
/*
 http://www.merriampark.com/ld.htm
 What is Levenshtein Distance?

 Levenshtein distance (LD) is a measure of the similarity between two strings,
 which we will refer to as the source string (s) and the target string(t).
 The distance is the number of deletions, insertions, or substitutions required
 to transform s into t. For example,

 * If s is "test" and t is "test", then LD(s,t) = 0, because no transformations are needed.
 The strings are already identical.
 * If s is "test" and t is "tent", then LD(s,t) = 1, because one substitution
 (change "s" to "n") is sufficient to transform s into t.

 The greater the Levenshtein distance, the more different the strings are.

 Levenshtein distance is named after the Russian scientist Vladimir Levenshtein,
 who devised the algorithm in 1965. If you can't spell or pronounce Levenshtein,
 the metric is also sometimes called edit distance.

 The Levenshtein distance algorithm has been used in:

 * Spell checking
 * Speech recognition
 * DNA analysis
 * Plagiarism detection
 */

#define OPTIMIZE_ED
/*
 Cover transposition, in addition to deletion,
 insertion and substitution. This step is taken from:
 Berghel, Hal ; Roach, David : "An Extension of Ukkonen's
 Enhanced Dynamic Programming ASM Algorithm"
 (http://www.acm.org/~hlb/publications/asm/asm.html)
 */
#define COVER_TRANSPOSITION

/****************************************/
/*Implementation of Levenshtein distance*/
/****************************************/

EditDistance::EditDistance()
{
	currentelements = 2500; // It's enough for most conditions :-)
	d = (int*) malloc(sizeof(int) * currentelements);
}

EditDistance::~EditDistance()
{
	//    printf("size:%d\n",currentelements);
	if (d)
		free( d);
}

#ifdef OPTIMIZE_ED
int EditDistance::CalEditDistance(const char_t *s, const char_t *t,
		const int limit)
/*Compute levenshtein distance between s and t, this is using QUICK algorithm*/
{
	int n = 0, m = 0, iLenDif, k, i, j, cost;
	// Remove leftmost matching portion of strings
	while (*s && (*s == *t)) {
		s++;
		t++;
	}

	while (s[n]) {
		n++;
	}
	while (t[m]) {
		m++;
	}

	// Remove rightmost matching portion of strings by decrement n and m.
	while (n && m && (*(s + n - 1) == *(t + m - 1))) {
		n--;
		m--;
	}
	if (m == 0 || n == 0 || d == (int*) 0)
		return (m + n);
	if (m < n) {
		const char_t * temp = s;
		int itemp = n;
		s = t;
		t = temp;
		n = m;
		m = itemp;
	}
	iLenDif = m - n;
	if (iLenDif >= limit)
		return iLenDif;
	// step 1
	n++;
	m++;
	//    d=(int*)malloc(sizeof(int)*m*n);
	if (m * n > currentelements) {
		currentelements = m * n * 2; // double the request
		d = (int*) realloc(d, sizeof(int) * currentelements);
		if ((int*) 0 == d)
			return (m + n);
	}
	// step 2, init matrix
	for (k = 0; k < n; k++)
		d[k] = k;
	for (k = 1; k < m; k++)
		d[k * n] = k;
	// step 3
	for (i = 1; i < n; i++) {
		// first calculate column, d(i,j)
		for (j = 1; j < iLenDif + i; j++) {
			cost = s[i - 1] == t[j - 1] ? 0 : 1;
			d[j * n + i] = minimum(d[(j - 1) * n + i] + 1,
					d[j * n + i - 1] + 1, d[(j - 1) * n + i - 1] + cost);
#ifdef COVER_TRANSPOSITION
			if (i >= 2 && j >= 2
					&& (d[j * n + i] - d[(j - 2) * n + i - 2] == 2)
					&& (s[i - 2] == t[j - 1]) && (s[i - 1] == t[j - 2]))
				d[j * n + i]--;
#endif
		}
		// second calculate row, d(k,j)
		// now j==iLenDif+i;
		for (k = 1; k <= i; k++) {
			cost = s[k - 1] == t[j - 1] ? 0 : 1;
			d[j * n + k] = minimum(d[(j - 1) * n + k] + 1,
					d[j * n + k - 1] + 1, d[(j - 1) * n + k - 1] + cost);
#ifdef COVER_TRANSPOSITION
			if (k >= 2 && j >= 2
					&& (d[j * n + k] - d[(j - 2) * n + k - 2] == 2)
					&& (s[k - 2] == t[j - 1]) && (s[k - 1] == t[j - 2]))
				d[j * n + k]--;
#endif
		}
		// test if d(i,j) limit gets equal or exceed
		if (d[j * n + i] >= limit) {
			return d[j * n + i];
		}
	}
	// d(n-1,m-1)
	return d[n * m - 1];
}
#else
int EditDistance::CalEditDistance(const char *s,const char *t,const int limit)
{
	//Step 1
	int k,i,j,n,m,cost;
	n=strlen(s);
	m=strlen(t);
	if( n!=0 && m!=0 && d!=(int*)0 )
	{
		m++;n++;
		if ( m*n > currentelements )
		{
			currentelements = m*n*2;
			d = (int*)realloc(d,sizeof(int)*currentelements);
			if ( (int*)0 == d )
			return (m+n);
		}
		//Step 2	
		for(k=0;k<n;k++)
		d[k]=k;
		for(k=0;k<m;k++)
		d[k*n]=k;
		//Step 3 and 4	
		for(i=1;i<n;i++)
		for(j=1;j<m;j++)
		{
			//Step 5
			if(s[i-1]==t[j-1])
			cost=0;
			else
			cost=1;
			//Step 6
			d[j*n+i]=minimum(d[(j-1)*n+i]+1,d[j*n+i-1]+1,d[(j-1)*n+i-1]+cost);
#ifdef COVER_TRANSPOSITION
			if ( i>=2 && j>=2 && (d[j*n+i]-d[(j-2)*n+i-2]==2)
					&& (s[i-2]==t[j-1]) && (s[i-1]==t[j-2]) )
			d[j*n+i]--;
#endif        
		}
		return d[n*m-1];
	}
	else
	return (n+m);
}
#endif
