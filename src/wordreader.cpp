#include "wordreader.hpp"
#include "utils.hpp"

using namespace std;

wordreader_t::wordreader_t(std::FILE *f)
{
	open(f);
}

void wordreader_t::open(std::FILE *f)
{
	f_in = f;
	s_n = 1024 * 4;
	s = new char_t[s_n];	
	p = 0;
	m_eof = false;
}

wordreader_t::~wordreader_t()
{
	fclose(f_in);
	delete [] s;
}

string wordreader_t::get_word()
{
	while (p == 0 || *p == 0 || !like_letter(*p)) {
		do {
			if (feof(f_in) ||
			    fgets(s, s_n, f_in) == NULL) {
				m_eof = true;
				return "";
			}
			//printf("got lin `%s'", s);
		} while (s[0] == 0);
                p = &s[0];
		while (*p && !like_letter(*p)) {
			++p;
		}
	}
	
	char *t = p;
	while (*t && like_letter(*t)) {
		++t;
	}
	*t = 0;
	string ret = p;
	p = t + 1;
	while (*p && !like_letter(*p)) {
		++p;
	}
	return ret;
	
}

