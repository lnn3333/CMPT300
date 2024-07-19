CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror

all: build

build: UnixLs
	gcc $(CFLAGS) UnixLs.c -o UnixLs

valgrind: build
	valgrind --leak-check=full ./UnixLs

clean: 
	rm -f UnixLs