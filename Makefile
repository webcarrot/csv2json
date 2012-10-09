all:
	cc ./csv2json.c -o ./csv2json;

.PHONY: install

install:
	cp csv2json /usr/bin;

.PHONY: clean

clean:
	rm -f ./csv2json
