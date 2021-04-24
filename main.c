#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h> 
#include <ncurses.h>

#include "utils.h"

void statusline(int row, int now, int total, char* title);

int readfile(char arr[MAXSLD][MAXLEN], char *title, const char fname[]) {
	FILE *fp;
	if ((fp = fopen(fname, "r")) == NULL) {
		fputs("Error: file does not exist or is not readable\n", stderr);
		usage();
	}

	char c, line[MAXLEN];
	int i, j;
	i = j = 0;
	while ((c = getc(fp)) != EOF && i < MAXLEN) {
		if (j == 0 && isspace(c))
			continue;

		if (c == '\n') {
			rtrim(line);
			if (!strcmp(line, "--")) {
				strcpy(arr[i+1], arr[i]);
				i++;
			} else if (!strcmp(line, "--newpage"))
				i++;
			else {
				line[j] = c;
				gettitle(line, title);
				strcat(arr[i], line);
			}
			clean(line);
			j = 0;
		} else
			line[j++] = c;
	}

	if (!title)
		fnametotitle(title, fname);

	return i;
}

void format(WINDOW *win, int height, int width, char* text, int lc);

void draw(char *slide, int now, int total, char* title) {
	int row, col, margin = 4;
	getmaxyx(stdscr, row, col);

	/* line width, temp width & line count */
	int lw, tw, lc = 1;
	lw = tw = 0;
	char c, temp[MAXLEN] = "", msg[MAXLEN] = "";

	/* fix formating so that it breaks words when having lines too long for screen */
	while (*slide != '\0' && lc < (row - 2*margin)) {
		temp[tw++] = c = *slide++;
		if (isspace(c)) {
			if (tw + lw <= (col - 2*margin)) {
				strcat(msg, temp);
				if (c == '\n') {
					lc++;
					lw = 0;
				} else
					lw += tw;
			} else {
				lw = tw;
				lc++;
				strcat(strcat(msg, "\n"), temp);
			}
			clean(temp);
			tw = 0;
		}
	}

	WINDOW *win;
	win = newwin(row - 2*margin, col - 2*margin, margin, margin);
	format(win, row - 2*margin, col - 2*margin, msg, lc);
	wrefresh(win);
	delwin(win);

	statusline(row, now+1, total+1, title);
	refresh();
}

int main(int argc, char *argv[]) {
	char pages[MAXSLD][MAXLEN] = {""};
	int slide = 0, total = 0;
	char title[200] = "";

	/* get options */
	if (argc == 1)
		usage();
	for (int i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-v")) {
			printf("scpp 1.0\n");
			return 0;
		} else if (!strcmp(argv[i], "-h"))
			usage();
		else if (!strcmp(argv[i], "-f"))
			slide = -1;
		else if (i + 1 == argc)
			total = readfile( pages, title, (!strcmp(argv[i], "-")) ? "/dev/stdin" : argv[i] );
		else if (!strcmp(argv[i], "-s"))
			slide = atoi(argv[++i]) - 1;
		else
			usage();
	}
	slide = (slide < 0) ? total + slide + 1 : slide;
	slide = (slide < 0) ? 0 : slide;
	slide = (slide > total) ? total : slide;

	initscr();
	cbreak();
	curs_set(0);
	refresh();
	draw(pages[slide], slide, total, title);

	noecho();
	keypad(stdscr, TRUE);
	int key;
	while ( (key = getch()) != 'q' ) {
		switch (key) {
			case 'h':
			case 'p':
			case KEY_LEFT:
				if (slide) {
					slide--;
					draw(pages[slide], slide, total, title);
				}
				break;

			case 'l':
			case 'n':
			case KEY_RIGHT:
			case ' ':
			case '\n':
				if (slide - total) {
					slide++;
					draw(pages[slide], slide, total, title);
				}
				break;

			case 'g':
				slide = 0;
				draw(pages[slide], slide, total, title);
				break;

			case 'G':
				slide = total;
				draw(pages[slide], slide, total, title);
				break;

			case KEY_RESIZE:
				endwin();
				refresh();
				clear();
				draw(pages[slide], slide, total, title);
				break;
		}
	}

	endwin();
	return 0;
}

void statusline(int row, int now, int total, char* title) {
	attron(A_BOLD);
	mvprintw(row-1, 0, "[%d/%d] %s", now, total, title);
	attroff(A_BOLD);
}

void format(WINDOW *win, int height, int width, char* text, int lc) {
	char c;

	int i = 0;
	/* title format */
	int attr, inc;
	/* check for slide title/subtitle */
	for (int j = 0; j < 2; ++j)
		if (text[i] == '#') {
			if (text[++i] == '#')
				attr = A_ITALIC, i++;
			else
				attr = A_BOLD;

			wattron(win, attr);
			do
				waddch(win, text[i]);
			while (text[++i] != '\n');
			wattroff(win, attr);
			waddch(win, text[i++]);
		}

	wmove(win, (height - lc)/2, 0);
	for (; i < strlen(text); ++i) {
		if ((c = text[i]) == '*' || c == '_') {
			if (!isalpha(text[i-1])) {
				if (text[++i] == c) {
					wattron(win, A_BOLD);
				} else {
					wattron(win, A_ITALIC);
					i--;
				}
				continue;
			} else if (!isalpha(text[++i])) {
				if (text[i] == c) {
					wattroff(win, A_BOLD);
				} else {
					wattroff(win, A_ITALIC);
					i--;
				}
				continue;
			}
		}
		if (i == 0 || text[i-1] == '\n') {
			inc = formatcenter(win, i, text, width);
			i  += inc;
			if (inc)
				continue;
		}

		waddch(win, (c == '\\') ? text[++i] : c);
	}
}
