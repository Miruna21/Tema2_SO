build: libso_stdio.so

libso_stdio.so: tema_linux.o
	gcc -Wall -shared tema_linux.o -o libso_stdio.so

tema_linux.o: tema_linux.c
	gcc -Wall -fPIC -o tema_linux.o -c tema_linux.c

.PHONY: clean

clean:
	rm -f *.o libso_stdio.so
