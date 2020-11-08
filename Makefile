all: build_all link_all run

build_all:
	g++ -c main.cpp lexer.cpp parser.cpp

link_all:
	g++ main.o lexer.o parser.o -o main

run:
	./main $(prog)
