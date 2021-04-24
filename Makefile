PREFIX ?= /usr
MANDIR ?= $(PREFIX)/share/man
DOCDIR ?= $(PREFIX)/share/doc/scpp

CC     = gcc
CFLAGS = -lncurses

.PHONY: install

all:
	@echo Run \'make install\' or just \'make\' to install scpp.

scpp: main.c
	@echo Compiling program...
	$(CC) $(CFLAGS) -o scpp main.c

install: scpp
	@echo Installing...
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@mkdir -p $(DESTDIR)$(MANDIR)/man1
	@mkdir -p $(DESTDIR)$(DOCDIR)
	@cp -p scpp $(DESTDIR)$(PREFIX)/bin/scpp
	@cp -p scpp.1 $(DESTDIR)$(MANDIR)/man1
	@cp -p README.md $(DESTDIR)$(DOCDIR)

uninstall:
	@echo uninstalling...
	@rm -rf $(DESTDIR)$(PREFIX)/bin/scpp
	@rm -rf $(DESTDIR)$(MANDIR)/man1/scpp.1
	@rm -rf $(DESTDIR)$(DOCDIR)
