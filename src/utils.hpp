#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

typedef char char_t;
typedef std::string string_t;

bool like_letter(char_t c);
bool isvowel(char_t c);
bool looks_same(const string_t &a, const string_t &b);
int word_distance(const string_t &a, const string_t &b);
string_t tolower(const string_t &s);

#ifndef HAVE_STRTOK_R
extern "C" char *
strtok_r(char *s, const char *delim, char **save_ptr);
#endif

enum work_mode_t {
	MODE_WC, MODE_TRANS, MODE_HIGHLIGHT
};

class settings_t {
public:
	settings_t();
	void parse_opts(int argc, char *argv[]);
	std::string locate_file(const std::string &name);

	bool verbose, full_out;
	std::string wf_file_name, voc_file_name, in_file_name, hl_file_name;
	work_mode_t work_mode;
};

extern settings_t settings;

class EditDistance {
private:
	int *d;
	int currentelements;
	/*Gets the minimum of three values */
	inline int minimum(const int a, const int b, const int c)
	{
		int min = a;
		if (b < min)
			min = b;
		if (c < min)
			min = c;
		return min;
	}
	;
public:
	EditDistance();
	~EditDistance();
	int CalEditDistance(const char_t *s, const char_t *t, const int limit);
};

#define FOR_EACH(type, i, cont) for (type::iterator i = (cont).begin(); i != (cont).end(); ++i)

#endif
