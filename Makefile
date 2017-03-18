CC=g++
RM=rm -vf
CPPFLAGS=-std=c++11 -Wall -pedantic -Wextra

.PHONY: all clean 

all: client server

client: ftrest.cpp
	${CC} ${CPPFLAGS} ftrest.cpp -o ftrest

server: ftrestd.cpp
	${CC} ${CPPFLAGS} ftrestd.cpp -o ftrestd

clean:
	${RM} *.o  
