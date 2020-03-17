# Lab 2B Makefile

CC=gcc
COMPILEROPTIONS = -Wall -Wextra
PTHREADLINK = -pthread
LAB2LISTPICS = lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png
LAB2LISTFILES = SortedList.h SortedList.c lab2_list.c lab2b_list.csv lab2_list.gp
GPERFTOOLSDIR = /usr/local/cs/gperftools-2.7/

default:
	$(CC) $(COMPILEROPTIONS) $(PTHREADLINK) -g -o lab2_list lab2_list.c SortedList.h SortedList.c

tests: default
	# test run times, graphs 1 and 2
	- ./lab2_list --iterations=1000 --threads=1 --sync=m > lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=2 --sync=m >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=8 --sync=m >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=12 --sync=m >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=16 --sync=m >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=24 --sync=m >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=1 --sync=s >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=2 --sync=s >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=8 --sync=s >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=12 --sync=s >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=16 --sync=s >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=24 --sync=s >> lab2b_list.csv

	# tests how many threads and iterations until failure with 4 lists, graph 3
	- ./lab2_list --threads=1 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv

	# tests threads and iterations with mutex with 4 lists, graph 3
	- ./lab2_list --threads=1 --iterations=10 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=10 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=10 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=10 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=10 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=20 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=20 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=20 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=20 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=20 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=40 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=40 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=40 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=40 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=40 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=80 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=80 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=80 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=80 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=80 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	
	# tests threads and iterations with spin lock
	- ./lab2_list --threads=1 --iterations=10 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=10 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=10 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=10 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=10 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=20 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=20 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=20 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=20 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=20 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=40 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=40 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=40 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=40 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=40 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=80 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=80 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=80 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=80 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=80 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	
	# test run times, graph 4
	- ./lab2_list --iterations=1000 --threads=1 --sync=m --lists=4 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=2 --sync=m --lists=4 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=4 --sync=m --lists=4 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=8 --sync=m --lists=4 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=12 --sync=m --lists=4 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=1 --sync=s --lists=4 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=2 --sync=s --lists=4 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=4 --sync=s --lists=4 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=8 --sync=s --lists=4 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=12 --sync=s --lists=4 >> lab2b_list.csv

	- ./lab2_list --iterations=1000 --threads=1 --sync=m --lists=8 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=2 --sync=m --lists=8 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=4 --sync=m --lists=8 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=8 --sync=m --lists=8 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=12 --sync=m --lists=8 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=1 --sync=s --lists=8 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=2 --sync=s --lists=8 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=4 --sync=s --lists=8 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=8 --sync=s --lists=8 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=12 --sync=s --lists=8 >> lab2b_list.csv

	- ./lab2_list --iterations=1000 --threads=1 --sync=m --lists=12 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=2 --sync=m --lists=12 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=4 --sync=m --lists=12 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=8 --sync=m --lists=12 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=12 --sync=m --lists=12 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=1 --sync=s --lists=12 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=2 --sync=s --lists=12 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=4 --sync=s --lists=12 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=8 --sync=s --lists=12 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=12 --sync=s --lists=12 >> lab2b_list.csv

	- ./lab2_list --iterations=1000 --threads=1 --sync=m --lists=16 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=2 --sync=m --lists=16 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=4 --sync=m --lists=16 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=8 --sync=m --lists=16 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=12 --sync=m --lists=16 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=1 --sync=s --lists=16 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=2 --sync=s --lists=16 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=4 --sync=s --lists=16 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=8 --sync=s --lists=16 >> lab2b_list.csv
	- ./lab2_list --iterations=1000 --threads=12 --sync=s --lists=16 >> lab2b_list.csv

graphs:
	gnuplot lab2_list.gp

profile:
	$(CC) $(COMPILEROPTIONS) $(PTHREADLINK) -L$(GPERFTOOLSDIR) -lprofiler -g -o lab2_list_prof lab2_list.c SortedList.h SortedList.c
	LD_PRELOAD=/usr/local/cs/gperftools-2.7/lib/libprofiler.so.0	
	CPUPROFILE=profile2.out ./lab2_list_prof --threads=12 --iterations=1000 --sync=s
	pprof --text --lines lab2_list_prof profile2.out > profile.out
	rm -f lab2_list lab2_list_prof profile2.out

dist: tests graphs
	rm -f lab2_list
	tar -czvf lab2b-305087992.tar.gz Makefile README profile.out $(LAB2LISTFILES) $(LAB2LISTPICS)

clean:
	rm -f *.tar.gz lab2_list

.PHONY: all build lab2_list tests graphs profile dist clean
