all: q1 q2

q1: Q1.c Q1.h
	gcc -Wpedantic -std=gnu99 Q1.c -g -o q1

q2: Q2.c Q2.h
	gcc -Wpedantic -std=gnu99 Q2.c -g -o q2

clean:
	rm q1 q2