#C for object files, E for executables, A for all, L for lib flags
EFLAGS=-Wall -pedantic
AFLAGS=-std=c99 -Wall -pedantic
NAMEFLAG=-o ass8

ass8: assignment8.o
	cc $(NAMEFLAG) $(EFLAGS) assignment8.o

assignment8.o: assignment8.c
	cc -c $(AFLAGS) assignment8.c

all: 
	cc $(NAMEFLAG) assignment8.c

clean:
	rm -f assignment8.o && rm -f ass8

run: ass8
	./ass8