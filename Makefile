CC=gcc
DEPS = HTTP_Server.h
exec = cervur
sources = $(wildcard src/*.c)
objects = $(sources:.c=.o)
CFLAGS=-Wextra -Wall -lm  -I./include -fomit-frame-pointer -march=native

#
.PHONY: build
build: $(objects)
	$(CC) $(objects) $(CFLAGS) -o $(exec) 
.PHONY: debug

debug:
	$(CC) $(oobjects) $(CFLAGS) $(DEBUG_FLAGS) -o $(exec)

%.o: %.c %.h
	$(CC)   $(CFLAGS) -c  $< -o $@


clean:
	-rm src/*.o
