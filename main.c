#include <sys/ioctl.h>
#include <stdio.h>
#include <errno.h>
#include <ncurses.h>
#include <string.h>

WINDOW* init_window(int srow, int scol)
{
	WINDOW* borderwin = newwin(srow-4, scol-4, 2, 2);
	box(borderwin, 0, 0);
	refresh();
	wrefresh(borderwin);
	const char* title[] = { "C-typing", NULL};
	mvprintw(0,(int)scol/2-(sizeof title/2),*title);
	return borderwin;
}
WINDOW* filetoscreen(FILE* f, WINDOW* win, char* text)
{
	int row, col;
	getmaxyx(win, row, col);
	WINDOW* textwin = newwin(row-2, col-2, 3, 3);
	refresh();
	wrefresh(textwin);
	fgets(text, 2048, f);
	mvwprintw(textwin, 0, 0, "%s", text);
	wmove(textwin, 0, 0);
	refresh();
	wrefresh(textwin);
	return textwin;
}

int main(int argc, char **argv)
{
	FILE* txtfile = fopen("texts.txt", "r");
	if(txtfile == NULL)
	{
		perror("Cannot open file texts.txt\n");
		return -ENOENT;
	}
	struct winsize w;
	ioctl(0, TIOCGWINSZ, &w);
	initscr(); // Initialize screen
	if(!has_colors())
	{
		perror("Cannot detect color support\n");
		return -EPROTONOSUPPORT; // Possibly not the correct errno
	}
	start_color();
	noecho();
	cbreak();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_RED, COLOR_BLACK);
	WINDOW* borderwin = init_window(w.ws_row, w.ws_col);
	char text[2048];
	WINDOW* textwin = filetoscreen(txtfile, borderwin, &text[0]);
	for(int i = 0; i < (int)strlen(text); ++i)
	{
		wmove(textwin, 0, i);
		refresh();
		wrefresh(textwin);
		char input = (char)getch();
		char current = text[i];
		if(input == current) // correct character
		{
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
				wattron(textwin, COLOR_PAIR(1));
				waddch(textwin, (int)text[i]);
				wattroff(textwin, COLOR_PAIR(1));
			}
			i--;
		}
		else // wrong character
		{
			wattron(textwin, COLOR_PAIR(3));
			waddch(textwin, (int)current);
			wattroff(textwin, COLOR_PAIR(3));
			wmove(textwin, 0, i+1);
		}
		i = (i<-1) ? -1 : i;
	}
	getch();
	endwin();
	return 0;
}
