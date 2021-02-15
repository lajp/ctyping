all: main.c
	gcc -Wall -lncurses -o ctyping main.c

clean:
	rm -f ctyping
