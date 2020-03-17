CC=gcc
COMPILEROPTIONS = -Wall -Wextra
LIBLINKS = -lmraa -lm

default:
	$(CC) $(COMPILEROPTIONS) $(LIBLINKS) -g -o lab4b lab4b.c

check: default
	chmod a+x smoketest.sh
	./smoketest.sh
	if [ -s failureMsgs.txt ]; then \
		cat failureMsgs.txt; \
		echo "make check result: smoke test FAILED"; \
	else \
		echo "make check result: smoke test PASSED"; \
	fi; \
	rm -f failureMsgs.txt
	
dist:
	tar -cvzf lab4b-305087992.tar.gz lab4b.c Makefile README smoketest.sh

clean:
	rm -f lab4b *.tar.gz *.txt
