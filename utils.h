#define MAXSLD 1000
#define MAXLEN 1000

#ifndef A_ITALIC
#define A_ITALIC A_BLINK
	// Force ncurses to use individual enter_attr_mode strings:
	set_attributes = NULL;
	enter_blink_mode = enter_italics_mode;
#endif

static void usage(void) {
	fputs("usage: scpp [-hv] [-f] [-s SLIDE] FILE\n", stderr);
	exit(1);
}

void clean(char *string) {
	while (*string != '\0') {
		*(string++) = '\0';
	}
}

void rtrim(char s[]) {
	int n;
	for (n = strlen(s); n >= 0; n--)
		if (!isspace(s[n]))
			break;
	s[n+1] = '\0';
}

void replaceCommand(char *line, const char command[], const char replacement[]) {
	int n = strlen(command);

	char *temp;
	temp = malloc((MAXLEN+1) * sizeof(char));
	if (temp == NULL) {
		fputs("Error: Could not allocate memory for operation\n", stderr);
		return;
	}

	int j = 0;
	if (!strncmp(line, command, n)) {
		for (int i = n; i < strlen(line); i++) {
			/* remove beginning spaces */
			if ( !j && isspace(*(line+i)) )
				continue;
			*(temp+j) = *(line+i);
			j++;
		}
		*(temp+j) = '\0';
		strcpy(line, replacement);
		strcat(line, temp);
	}
	free(temp);
}

void gettitle(char *line, char *title) {
	if (!strncmp(line, "--title:", 8)) {
		int j = 0;
		for (int i = 8; i < strlen(line); ++i) {
			if (j == 0 && isspace(line[i]))
				continue;
			*(title+j) = line[i];
			j++;
		}
		*(title+j) = '\0';
		strcpy(line, "--bcenter:");
		strcat(line, title);
	}
	replaceCommand(line, "--subtitle:", "--icenter:");
	replaceCommand(line, "--author:",   "--center:");
	replaceCommand(line, "--date:",     "--center:");
}

void fnametotitle(char *title, const char fname[]) {
	int n, point = strlen(fname);
	for (n = strlen(fname) - 1; n >= 0; n--) {
		/* detect last apareance of '/' in filename (going backwards) */
		if (fname[n] == '/')
			break;
		/* detect first point in filename (going backwards) */
		else if (fname[n] == '.' && point != strlen(fname))
			point = n;
	}

	/* copy substring to title */
	for (int i = n+1; i < point; ++i) 
		*title++ = fname[i];
}

int formatcenter(WINDOW *win, int i, char* text, int width) {
	int start;
	char c, *command, *line;
	command = (char*) malloc(21 * sizeof(char));
	line    = (char*) malloc((MAXLEN+1) * sizeof(char));
	if (command == NULL || line == NULL) {
		fputs("Error: Not enoght memory for this operation\n", stderr);
		return 0;
	}

	start = i;
	if (text[i+1] == text[i] && text[i] == '-') {
		int j;
		i += 2;
		for (j = 0; j < 20; ++j) {
			if ((c = text[i++]) == ':' || c == '\n')
				break;
			*(command+j) = c;
		}
		*(command+j) = '\0';

		j = 0;
		int x, y, attr;
		if (!strcmp(command, "bcenter"))
			attr = A_BOLD;
		else if (!strcmp(command, "icenter"))
			attr = A_ITALIC;
		else if (!strcmp(command, "center"))
			attr = A_NORMAL;
		else
			return 0;
		free(command);

		if (attr)
			wattron(win, attr);
		while ((c = text[i]) != '\n' && c != EOF)
			*(line+j) = c, j++, i++;
		*(line+j) = '\0';
		getyx(win, y, x);
		mvwprintw(win, y, (width - j)/2, "%s\n", line);
		wattrset(win, A_NORMAL);
		free(line);
	}
	return i - start;
}
