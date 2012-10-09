# Simple Makefile
INSTALL = install
INSTALL_PROGRAM = $(INSTALL) -m 755
INSTALL_DIR = $(INSTALL) -d
UNINSTALL_PROGRAM = rm
DESTDIR = /usr/
bindir = bin
CFLAGS=-c -Wall

.PHONY: all install uninstall clean

all:
	cc $(CFLAGS) ./csv2json.c -o ./csv2json;

install:
	$(INSTALL_DIR) $(DESTDIR)$(bindir);
	$(INSTALL_PROGRAM) csv2json $(DESTDIR)$(bindir)/;

uninstall:
	$(UNINSTALL_PROGRAM) $(DESTDIR)$(bindir)/csv2json;

clean:
	rm -f ./csv2json;
