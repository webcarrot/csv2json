# Simple Makefile
INSTALL = install
INSTALL_PROGRAM = $(INSTALL) -m 755
INSTALL_DIR = $(INSTALL) -d
UNINSTALL_PROGRAM = rm
DESTDIR = /
BINDIR = /usr/bin/
CFLAGS = -c -Wall

.PHONY: all install uninstall clean

all:
	cc $(CFLAGS) ./csv2json.c -o ./csv2json;

install:
	$(INSTALL_DIR) $(DESTDIR)$(BINDIR);
	$(INSTALL_PROGRAM) csv2json $(DESTDIR)$(BINDIR)/;

uninstall:
	$(UNINSTALL_PROGRAM) $(DESTDIR)$(BINDIR)/csv2json;

clean:
	rm -f ./csv2json;