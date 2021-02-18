#include "ctyping.h"

pthread_mutex_t mutex;
pthread_cond_t condition;

void* timerThread(void* arguments)
{
	struct timerargs *args = arguments;
	bool* stop = args->stop;
	clock_t start_t = args->start_t;
	WINDOW* timewin = args->timewin;
	clock_t now_t;
	while(!*stop)
	{
		pthread_mutex_lock(&mutex);
		now_t = clock();
		mvwprintw(timewin, 2, 1, "Time elapsed: %f", (double)10*(now_t - start_t) / CLOCKS_PER_SEC);
		refresh();
		wrefresh(timewin);
		refresh();
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&condition);
		usleep(1000);
	}
	return NULL;
}
void* inputThread(void* arguments)
{
	struct inputargs *args = arguments;
	char* text = args->text;
	WINDOW* textwin = args->textwin;
	WINDOW* statwin = args->statwin;
	int correct, incorrect, mistake;
	correct = incorrect = mistake = 0;
	for(int i = 0; i < (int)strlen(text)-1; ++i)
	{
		char input = (char)wgetch(textwin);
		char current = text[i];
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&condition, &mutex);
		if(input == current) // correct character
		{
			correct++;
			wattron(textwin, COLOR_PAIR(2));
			waddch(textwin, (int)input);
			wattroff(textwin, COLOR_PAIR(2));
			wmove(textwin, 0, i+1);
		}
		else if(input == (char)127) // backspace
		{
			i--;
			if(i >= 0)
			{
				wmove(textwin, 0, i);
				if((winch(textwin) & A_COLOR) == COLOR_PAIR(2))
					correct--;
				else
					incorrect--;
				wattron(textwin, COLOR_PAIR(1));
				waddch(textwin, (int)text[i]);
				wattroff(textwin, COLOR_PAIR(1));
			}
			i--;
		}
		else // wrong character
		{
			mistake++;
			incorrect++;
			wattron(textwin, COLOR_PAIR(3));
			if(text[i] == ' ')
			{
				wattron(textwin, A_REVERSE);
			}
			waddch(textwin, (int)current);
			wattroff(textwin, COLOR_PAIR(3));
			if(text[i] == ' ')
			{
				wattroff(textwin, A_REVERSE);
			}
			wmove(textwin, 0, i+1);
		}
		i = (i<-1) ? -1 : i;
		update_statwin(statwin, correct, incorrect, mistake);
		wmove(textwin, 0, i+1);
		wrefresh(textwin);
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}
WINDOW* init_window(int srow, int scol)
{
	WINDOW* borderwin = newwin(srow-8, scol-4, 6, 2);
	box(borderwin, 0, 0);
	refresh();
	wrefresh(borderwin);
	return borderwin;
}
WINDOW* filetoscreen(FILE* f, WINDOW* win, char* text)
{
	int row, col;
	getmaxyx(win, row, col);
	WINDOW* textwin = newwin(row-5, col-2, 7, 3);
	refresh();
	wrefresh(textwin);
	fgets(text, 2048, f);
	mvwprintw(textwin, 0, 0, "%s", text);
	wmove(textwin, 0, 0);
	refresh();
	wrefresh(textwin);
	return textwin;
}
WINDOW* init_timerwin()
{
	WINDOW* timerwin = newwin(5, 30, 0, 41);
	box(timerwin, 0, 0);
	mvwprintw(timerwin, 2, 1, "Time elapsed: %f", 0.0);
	refresh();
	wrefresh(timerwin);
	return timerwin;
}
void update_statwin(WINDOW* statwin, int correct, int incorrect, int mistake)
{
	mvwprintw(statwin, 1, 1, "Correct characters: %d", correct);
	mvwprintw(statwin, 2, 1, "Incorrect characters: %d", incorrect);
	mvwprintw(statwin, 3, 1, "Mistakes: %d", mistake);
	refresh();
	wrefresh(statwin);
}
WINDOW* init_statwin(int correct, int incorrect, int mistake)
{
	WINDOW* statwin = newwin(5, 40, 0, 0);
	box(statwin, 0, 0);
	update_statwin(statwin, correct, incorrect, mistake);
	refresh();
	wrefresh(statwin);
	return statwin;
}
FILE* open_textfile(int argc, char** argv)
{
	char* filename;
	char* dfilename[] = { "texts.txt", NULL };
	if(argc < 2)
	{
		filename = *dfilename;
	}
	else
	{
		filename = argv[argc-1];
	}
	return fopen(filename, "r");
}
int main(int argc, char **argv)
{
	FILE* txtfile = open_textfile(argc, argv);
	if(txtfile == NULL)
	{
		perror("Cannot open input file: ");
		return -ENOENT;
	}
	initscr(); // Initialize screen
	if(!has_colors())
	{
		perror("Cannot detect color support");
		return -EPROTONOSUPPORT; // FIXME: Possibly not the correct errno
	}
	start_color();
	noecho();
	cbreak();
	curs_set(0); // set cursor to be invisible
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_RED, COLOR_BLACK);
	int ymax, xmax;
	getmaxyx(stdscr, ymax, xmax);
	WINDOW* borderwin = init_window(ymax, xmax);
	char text[2048];
	int correct, incorrect, mistake;
	correct = incorrect = mistake = 0;
	WINDOW* textwin = filetoscreen(txtfile, borderwin, &text[0]);
	WINDOW* statwin = init_statwin(correct, incorrect, mistake);
	WINDOW* timewin = init_timerwin();
	clock_t start_t;
	bool stop;
	start_t = clock();
	pthread_t threads[2];
	// args for input-thread
	struct inputargs ipargs;
	ipargs.textwin = textwin;
	ipargs.statwin = statwin;
	ipargs.text = &text[0];
	// args for timer thread
	struct timerargs targs;
	targs.start_t = start_t;
	targs.stop = &stop;
	targs.timewin = timewin;
	// start threads
	pthread_create(&threads[0], NULL, inputThread, (void*)&ipargs);
	pthread_create(&threads[1], NULL, timerThread, (void*)&targs);
	// wait for input thread to finish
	pthread_join(threads[0], NULL);
	stop = true; // tell timer thread to quit
	pthread_join(threads[1], NULL);
	getch();
	endwin();
	return 0;
}
