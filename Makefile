# Makefile

CC=gcc
srcdir=./src/
bindir=./bin/
docdir=./doc/
savedir=./save/
PROG=main
FLAGS=-Wall
CXXFLAGS= $(FLAGS)
LDFLAGS=-lm
SRC=$(wildcard $(srcdir)*.c)
HEAD=$(wildcard $(srcdir)*.h)
OBJ=$(patsubst $(srcdir)%.c,$(bindir)%.o,$(SRC))
CURRENT_FOLDER=$(basename $(pwd))
LANGUAGE = English
PROJECT_NAME = 
CSS = doxygen-awesome.css

# Compilation
all : $(PROG)

$(PROG) : $(OBJ)
	$(CC) $^ -o $(bindir)$@ $(LDFLAGS) $(CXXFLAGS) 

$(bindir)%.o : $(srcdir)%.c $(HEAD)
	$(CC) $(CXXFLAGS) -c $< -o $@


# to use this function even if you have the same filename
.PHONY : clean
.PHONY : all_clean
.PHONY : clean_doc
.PHONY : clean_save
.PHONY : clean_exe
.PHONY : save
.PHONY : restore
.PHONY : run
.PHONY : Doxyfile
.PHONY : doc

# clean
all_clean : clean_doc clean clean_save clean_exe

clean :
	rm -f $(bindir)*.o

clean_exe :
	rm -f $(bindir)$(PROG)

clean_save :
	rm -f $(savedir)*

clean_doc :
	rm -rf $(docdir)*

# save & restore
save :
	cp -r $(srcdir)* $(savedir)

restore :
	cp -r $(savedir)* $(srcdir)

# run
run :
	$(bindir)$(PROG)

# documentation

all_doc : Doxyfile doc

Doxyfile :
	doxygen -g
	( echo 'PROJECT_NAME=$(PROJECT_NAME)' ; echo 'HTML_EXTRA_STYLESHEET=$(CSS)' ; echo 'OUTPUT8LANGUAGE=$(LANGUAGE)' ; echo 'OUTPUT_DIRECTORY=$(docdir)' ; echo 'INPUT=$(srcdir)') >> Doxyfile

doc :
	doxygen Doxyfile


# initialization
init :
	mkdir src
	mkdir save
	mkdir doc
	mkdir bin