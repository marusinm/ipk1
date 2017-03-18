CC=g++
# CC=gcc
RM=rm -vf
CPPFLAGS=-Wall -pedantic -Wextra
SRCFILES=ftrest.cpp ftrestd.cpp
OBJFILES= $(patsubst %.cpp, %.o, $(SRCFILES))
PROGFILES= $(patsubst %.cpp, %, $(SRCFILES))

.PHONY: all clean

all: $(PROGFILES)
clean:
	$(RM) $(OBJFILES) $(PROGFILES)
