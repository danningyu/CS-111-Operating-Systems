CC=gcc
COMPILEROPTIONS = -Wall -Wextra
LIBLINKS = -lmraa -lm
SSLLINKS = -lssl -lcrypto

default: lab4c_tcp_target lab4c_tls_target

lab4c_tcp_target:
	$(CC) $(COMPILEROPTIONS) $(LIBLINKS) -g -o lab4c_tcp lab4c_tcp.c

lab4c_tls_target:
	$(CC) $(COMPILEROPTIONS) $(LIBLINKS) $(SSLLINKS) -g -o lab4c_tls lab4c_tls.c
	
dist:
	tar -cvzf lab4c-305087992.tar.gz lab4c_tcp.c lab4c_tls.c Makefile README

clean:
	rm -f lab4c_tcp lab4c_tls *.tar.gz
