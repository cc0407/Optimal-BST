all: q1

q1: Q1.c Q1.h
	gcc -Wpedantic -std=gnu99 Q1.c -g -o q1

clean:
	rm q1
