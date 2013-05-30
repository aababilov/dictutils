#ifndef WORDREADER_HPP
#define WORDREADER_HPP

#include <string>
#include <cstdlib>
#include <cstdio>

class wordreader_t {
public:
	wordreader_t(std::FILE *f);
	~wordreader_t();

	std::string get_word();
	void open(std::FILE *f);
	bool eof() const { return m_eof; }
private:
	std::FILE *f_in;
	char *s, *p;
	size_t s_n;
	bool m_eof;
};

#endif
