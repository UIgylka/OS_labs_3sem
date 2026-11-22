CC = gcc
CFLAGS = -Wall -Wextra -O2

all: libimpl1.so libimpl2.so prog1 prog2

libimpl1.so: libimpl1.c contracts.h
	$(CC) $(CFLAGS) -shared -fPIC -o libimpl1.so libimpl1.c -lm

libimpl2.so: libimpl2.c contracts.h
	$(CC) $(CFLAGS) -shared -fPIC -o libimpl2.so libimpl2.c -lm

prog1: prog1.c contracts.h libimpl1.so
	$(CC) $(CFLAGS) -o prog1 prog1.c -L. -limpl1 -lm

prog2: prog2.c contracts.h
	$(CC) $(CFLAGS) -o prog2 prog2.c -ldl -lm

clean:
	rm -f *.so prog1 prog2

.PHONY: all clean