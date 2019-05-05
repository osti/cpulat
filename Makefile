all: cpulat

cpulat: cpulat.c
	gcc -o $@ $< -Wall -Wextra -Werror -O2 -g -ggdb

install: cpulat
	install -m 755 cpulat /usr/local/sbin/
	install -m 644 cpulat.conf /etc/
	install -m 755 init.d/cpulat /etc/init.d/

