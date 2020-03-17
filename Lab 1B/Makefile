# Danning Yu
# danningyu@ucla.edu
# 305087992

CC = gcc
COMPILEROPTIONS = -Wall -Wextra
ZLIBLINK = -lz

default:
	$(CC) $(COMPILEROPTIONS) $(ZLIBLINK) -g -o lab1b-server lab1b-server.c
	$(CC) $(COMPILEROPTIONS) $(ZLIBLINK) -g -o lab1b-client lab1b-client.c

server:
	$(CC) $(COMPILEROPTIONS) $(ZLIBLINK) -g -o lab1b-server lab1b-server.c

client:
	$(CC) $(COMPILEROPTIONS) $(ZLIBLINK) -g -o lab1b-client lab1b-client.c

dist:
	tar -czvf lab1b-305087992.tar.gz lab1b-client.c lab1b-server.c Makefile README

clean:
	rm -f lab1b-server lab1b-client *.tar.gz *.txt

