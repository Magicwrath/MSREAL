ALL: zadatak

zadatak: zadatak.o
	gcc -o zadatak zadatak.o

zadatak.o: zadatak.c
	gcc -c zadatak.c

.PHONY: clean
clean:
	rm -f zadatak zadatak.o
