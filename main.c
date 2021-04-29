#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h> 
#include <ncurses.h>
#include <locale.h>

#include "utils.h"

#define MIN(a,b)  (a < b) ? a : b
#define MAX(a,b)  (a > b) ? a : b

void statusline(int row, int now, int total, char* title);

int readfile(char arr[MAXSLD][MAXLEN], char *title, int slidegrp[], const char fname[]) {
	FILE *fp;
	if ((fp = fopen(fname, "r")) == NULL) {
		fputs("Error: file does not exist or is not readable\n", stderr);
		usage();
	}

	char c, line[MAXLEN];
	int i, j, k;
	i = j = k = 0;
	slidegrp[0] = 0;
	while ((c = getc(fp)) != EOF && i < MAXLEN) {
		if (c == '\n') {
			rtrim(line);
			if (!strcmp(line, "--")) {
				strcpy(arr[i+1], arr[i]);
				slidegrp[++i] = k;
			} else if (!strcmp(line, "--newpage"))
				k = ++i, slidegrp[i] = k;
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

int incSlide(int slide, const int slidegrp[], const int total) {
	for (int i = slide; i < total+1; ++i) {
		if (slidegrp[i] != slidegrp[slide])
			return i;
	}
	return slide;
}

int decSlide(int slide, const int slidegrp[], const int total) {
	int value = 0;
	for (int i = slide; i >= 0; --i) {
		if (slidegrp[i] != slidegrp[slide] && !value)
			value = slidegrp[i];
		else if (slidegrp[i] < value)
			return i+1;
	}
	return 0;
}

int main(int argc, char *argv[]) {
	char pages[MAXSLD][MAXLEN] = {""};
	int slide = 0, total = 0, slidegrp[MAXSLD] = {};
	char title[200] = "";

	/* get options */
	if (argc == 1)
		usage();
	for (int i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-v")) {
			printf("scpp 1.2\n");
			return 0;
		} else if (!strcmp(argv[i], "-h"))
			usage();
		else if (!strcmp(argv[i], "-f"))
			slide = -1;
		else if (i + 1 == argc)
			total = readfile( pages, title, slidegrp, (!strcmp(argv[i], "-")) ? "/dev/stdin" : argv[i] );
		else if (!strcmp(argv[i], "-s"))
			slide = atoi(argv[++i]) - 1;
		else
			usage();
	}
	/* check for slide overflow or negatives */
	slide = (slide < 0) ? total + slide + 1 : slide;
	slide = (slide < 0) ? 0 : slide;
	slide = (slide > total) ? total : slide;

	setlocale(LC_ALL, ""); // allow utf8 characters
	initscr();
	cbreak();
	curs_set(0); // hide cursor
	refresh();
	draw(pages[slide], slide, total, title);

	noecho(); // don't display pressed characters
	keypad(stdscr, TRUE); // include extra keys (in particular, left, right and backspace)
	int i, key, num = 0;
	while ( (key = getch()) != 'q' && key != 'Q' ) {
		/* number based commands */
		if (isdigit(key))
			do
				num = 10*num + (key - '0');
			while ( isdigit(key = getch()) );

		switch (key) {
			case 'h':
			case 'p':
			case KEY_LEFT:
			case KEY_BACKSPACE:
				slide -= MAX(num, 1); // is num not null?
				slide  = MAX(0, slide); // slide overflow
				draw(pages[slide], slide, total, title);
				break;

			case 'l':
			case 'n':
			case KEY_RIGHT:
			case ' ':
			case '\n':
				slide += MAX(num, 1);
				slide  = MIN(slide, total); // slide overflow
				draw(pages[slide], slide, total, title);
				break;

			case 'H':
				i = 0;
				do {
					if (slidegrp[slide])
						slide = decSlide(slide, slidegrp, total);
					else
						slide = 0;
				} while ((++i) < num);
				draw(pages[slide], slide, total, title);
				break;

			case 'L':
				i = 0;
				do
					slide = incSlide(slide, slidegrp, total);
				while ((++i) < num);
				draw(pages[slide], slide, total, title);
				break;

			case 'g':
				num--; // user will press slide 0 as 1
				slide = MAX(num, 0);
				slide = MIN(slide, total);
				draw(pages[slide], slide, total, title);
				break;

			case 'G':
				if (num) {
					num--;
					slide = num;
					slide = MIN(slide, total);
				} else
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

		num = 0; // reset number
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
	bool it = false, bf = false;
	/* check for slide title/subtitle */
	for (int j = 0; j < 2; ++j) {
		if (text[i] == '#') {
			if (text[++i] == '#')
				attr = A_ITALIC, i++;
			else
				attr = A_BOLD;

			wattron(win, attr);
			do
				wprintw(win, "%c", text[i]);
			while (text[++i] != '\n');
			wattroff(win, attr);
			waddch(win, text[i++]);
			lc--;
		} else
			break;
	}

	wmove(win, (height - lc)/2, 0);
	for (; i < strlen(text); ++i) {
		if ((c = text[i]) == '*' || c == '_') {
			if (text[++i] == c) {
				if (bf)
					wattroff(win, A_BOLD);
				else
					wattron(win, A_BOLD);
				bf = !bf;
			} else {
				if (it)
					wattroff(win, A_ITALIC);
				else
					wattron(win, A_ITALIC);
				it = !it, i--;
			}
			continue;
		}
		if (i == 0 || text[i-1] == '\n') {
			inc = formatcenter(win, i, text, width);
			i  += inc;
			if (inc)
				continue;
		}

		/* We use wprintw instead of waddch because of utf8 displaying errors */
		/* waddch(win, (c == '\\') ? text[++i] : c); */
		wprintw(win, "%c", (c == '\\') ? text[++i] : c);
	}
	wattrset(win, A_NORMAL);
}
