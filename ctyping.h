#include <stdio.h>
#include <errno.h>
#include <ncurses.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

void* timerThread(void* arguments);
void* inputThread(void* arguments);

FILE* open_textfile(int argc, char** argv);

WINDOW* init_window(int srow, int scol);
WINDOW* filetoscreen(FILE* f, WINDOW* win, char* text);
WINDOW* init_statwin(int correct, int incorrect, int mistake);
WINDOW* init_timerwin();
void update_statwin(WINDOW* statwin, int correct, int incorrect, int mistake);

struct inputargs {
	WINDOW* textwin;
	WINDOW* statwin;
	char* text;
};
struct timerargs {
	WINDOW* timewin;
	clock_t start_t;
	bool* stop;
};
