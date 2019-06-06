CC=gcc

src=$(wildcard *.c)
obj=$(src:.c=.o)

CFLAGS= -Wall -O2

cache: $(obj)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(obj) cache