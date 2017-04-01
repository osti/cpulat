all: cpulat

cpulat: cpulat.c
	gcc -o $@ $< -Wall -Wextra -Werror -O2 -g -ggdb

