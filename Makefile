all: ctyping.c
	clang -Wall -lncurses -lpthread -o ctyping ctyping.c

clean:
	rm -f ctyping
