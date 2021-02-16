#include <stdio.h>
#include <errno.h>
#include <ncurses.h>
#include <string.h>

WINDOW* init_window(int srow, int scol)
{
	WINDOW* borderwin = newwin(srow-8, scol-4, 6, 2);
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
WINDOW* init_statwin(int correct, int incorrect)
{
	WINDOW* statwin = newwin(5, 40, 0, 0);
	box(statwin, 0, 0);
	mvwprintw(statwin, 1, 1, "Correct characters: %d", correct);
	mvwprintw(statwin, 3, 1, "Mistakes: %d", incorrect);
	refresh();
	wrefresh(statwin);
	return statwin;
}
void update_statwin(WINDOW* statwin, int correct, int incorrect)
{
	mvwprintw(statwin, 1, 1, "Correct characters: %d", correct);
	mvwprintw(statwin, 3, 1, "Mistakes: %d", incorrect);
	refresh();
	wrefresh(statwin);
}
int main(int argc, char **argv)
{
	FILE* txtfile = fopen("texts.txt", "r");
	if(txtfile == NULL)
	{
		perror("Cannot open file texts.txt\n");
		return -ENOENT;
	}
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
	int ymax, xmax;
	getmaxyx(stdscr, ymax, xmax);
	WINDOW* borderwin = init_window(ymax, xmax);
	char text[2048];
	int correct, incorrect;
	correct = incorrect = 0;
	WINDOW* textwin = filetoscreen(txtfile, borderwin, &text[0]);
	WINDOW* statwin = init_statwin(correct, incorrect);
	for(int i = 0; i < (int)strlen(text)-1; ++i)
	{
		wmove(textwin, 0, i);
		wrefresh(textwin);
		char input = (char)getch();
		char current = text[i];
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
			incorrect++;
			wattron(textwin, COLOR_PAIR(3));
			waddch(textwin, (int)current);
			wattroff(textwin, COLOR_PAIR(3));
			wmove(textwin, 0, i+1);
		}
		i = (i<-1) ? -1 : i;
		update_statwin(statwin, correct, incorrect);
	}
	getch();
	endwin();
	return 0;
}
