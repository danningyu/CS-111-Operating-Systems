CC=gcc
COMPILEROPTIONS = -Wall -Wextra

default: ext2_fs.h
	$(CC) $(COMPILEROPTIONS) -g -o lab3a lab3a.c

dist:
	tar -czvf lab3a-305087992.tar.gz Makefile README lab3a.c ext2_fs.h

clean:
	rm -f lab3a *.tar.gz