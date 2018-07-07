CC  = gcc
CXX = g++

CFLAGS   = -g -Wall $(INCLUDES)
CXXFLAGS = -g -Wall $(INCLUDES)

.PHONY: default
default: http-server

.PHONY: clean
clean:
	rm -f *.o *~ http-server a.out core

.PHONY: all
all: clean default

.PHONY: valgrind
valgrind: http-server
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./http-server 11112 ~/html localhost 10001
