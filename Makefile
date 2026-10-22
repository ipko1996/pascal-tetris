CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra

tetris: tetris.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f tetris

.PHONY: clean
