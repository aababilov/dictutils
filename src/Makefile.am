DEFS = -DPKGDATADIR=\"$(pkgdatadir)\"

bin_PROGRAMS = dictext dicinvert

dictext_SOURCES =   \
	utils.hpp       vocabulary.hpp \
	wordform.cpp  wordreader.cpp \
	dictext.cpp  utils.cpp       vocabulary.cpp \
 	wordform.hpp  wordreader.hpp

dicinvert_SOURCES =   \
	utils.hpp       vocabulary.hpp \	
	dicinvert.cpp  utils.cpp       vocabulary.cpp \
 	wordform.hpp

clean-local:
	$(RM) *~*