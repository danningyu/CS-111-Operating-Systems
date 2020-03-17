CC=gcc
COMPILEROPTIONS = -Wall -Wextra
PTHREADLINK = -pthread
LAB2ADDPICS = lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png
LAB2LISTPICS = lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png
LAB2LISTFILES = SortedList.h SortedList.c lab2_list.c lab2_list.csv lab2_list.gp
LAB2ADDFILES = lab2_add.c lab2_add.csv lab2_add.gp

all: build

build: lab2_add lab2_list

lab2_add:
	$(CC) $(COMPILEROPTIONS) $(PTHREADLINK) -g -o lab2_add lab2_add.c

# lab2_list: SortedList.o lab2_list.o
# 	$(CC) $(COMPILEROPTIONS) $(PTHREADLINK) -g -o lab2_list lab2_list.o SortedList.o

# lab2_list.o: lab2_list.c
# 	$(CC) -c lab2_list.c

# SortedList.o: SortedList.h SortedList.c
# 	$(CC) -c SortedList.c

lab2_list:
	$(CC) $(COMPILEROPTIONS) $(PTHREADLINK) -g -o lab2_list lab2_list.c SortedList.h SortedList.c

tests: build
	#no yielding or protection - graphs 1 and 3
	- ./lab2_add --iterations=100 --threads=1 > lab2_add.csv
	- ./lab2_add --iterations=1000 --threads=1 >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=1 >> lab2_add.csv
	- ./lab2_add --iterations=100000 --threads=1 >> lab2_add.csv
	- ./lab2_add --iterations=1000000 --threads=1 >> lab2_add.csv

	- ./lab2_add --iterations=100 --threads=2 >> lab2_add.csv
	- ./lab2_add --iterations=1000 --threads=2 >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=2 >> lab2_add.csv
	- ./lab2_add --iterations=100000 --threads=2 >> lab2_add.csv
	- ./lab2_add --iterations=1000000 --threads=2 >> lab2_add.csv
	- ./lab2_add --iterations=100 --threads=4 >> lab2_add.csv
	- ./lab2_add --iterations=1000 --threads=4 >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=4 >> lab2_add.csv
	- ./lab2_add --iterations=100000 --threads=4 >> lab2_add.csv
	- ./lab2_add --iterations=1000000 --threads=4 >> lab2_add.csv
	- ./lab2_add --iterations=100 --threads=8 >> lab2_add.csv
	- ./lab2_add --iterations=1000 --threads=8 >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=8 >> lab2_add.csv
	- ./lab2_add --iterations=100000 --threads=8 >> lab2_add.csv
	- ./lab2_add --iterations=1000000 --threads=8 >> lab2_add.csv
	- ./lab2_add --iterations=100 --threads=12 >> lab2_add.csv
	- ./lab2_add --iterations=1000 --threads=12 >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=12 >> lab2_add.csv
	- ./lab2_add --iterations=100000 --threads=12 >> lab2_add.csv
	- ./lab2_add --iterations=1000000 --threads=12 >> lab2_add.csv

	- ./lab2_add --iterations=100 --threads=1 --yield >> lab2_add.csv
	- ./lab2_add --iterations=1000 --threads=1 --yield >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=1 --yield >> lab2_add.csv
	- ./lab2_add --iterations=100000 --threads=1 --yield >> lab2_add.csv
	- ./lab2_add --iterations=1000000 --threads=1 --yield >> lab2_add.csv

	# - ./lab2_add --iterations=100 --threads=2 --yield >> lab2_add.csv
	# - ./lab2_add --iterations=1000 --threads=2 --yield >> lab2_add.csv
	# - ./lab2_add --iterations=10000 --threads=2 --yield >> lab2_add.csv
	# - ./lab2_add --iterations=100000 --threads=2 --yield >> lab2_add.csv
	- ./lab2_add --iterations=1000000 --threads=2 --yield >> lab2_add.csv
	- ./lab2_add --iterations=100 --threads=4 --yield >> lab2_add.csv
	- ./lab2_add --iterations=1000 --threads=4 --yield >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=4 --yield >> lab2_add.csv
	- ./lab2_add --iterations=100000 --threads=4 --yield >> lab2_add.csv
	- ./lab2_add --iterations=1000000 --threads=4 --yield >> lab2_add.csv
	# - ./lab2_add --iterations=100 --threads=8 --yield >> lab2_add.csv
	# - ./lab2_add --iterations=1000 --threads=8 --yield >> lab2_add.csv
	# - ./lab2_add --iterations=10000 --threads=8 --yield >> lab2_add.csv
	# - ./lab2_add --iterations=100000 --threads=8 --yield >> lab2_add.csv
	- ./lab2_add --iterations=1000000 --threads=8 --yield >> lab2_add.csv
	- ./lab2_add --iterations=100 --threads=12 --yield >> lab2_add.csv
	- ./lab2_add --iterations=1000 --threads=12 --yield >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=12 --yield >> lab2_add.csv
	- ./lab2_add --iterations=100000 --threads=12 --yield >> lab2_add.csv
	- ./lab2_add --iterations=1000000 --threads=12 --yield >> lab2_add.csv

	#yielding - graph 2
	- ./lab2_add --iterations=10 --threads=2 --yield >> lab2_add.csv
	- ./lab2_add --iterations=20 --threads=2 --yield >> lab2_add.csv
	- ./lab2_add --iterations=40 --threads=2 --yield >> lab2_add.csv
	- ./lab2_add --iterations=80 --threads=2 --yield >> lab2_add.csv
	- ./lab2_add --iterations=100 --threads=2 --yield >> lab2_add.csv
	- ./lab2_add --iterations=1000 --threads=2 --yield >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=2 --yield >> lab2_add.csv
	- ./lab2_add --iterations=100000 --threads=2 --yield >> lab2_add.csv
	- ./lab2_add --iterations=10 --threads=4 --yield >> lab2_add.csv
	- ./lab2_add --iterations=20 --threads=4 --yield >> lab2_add.csv 
	- ./lab2_add --iterations=40 --threads=4 --yield >> lab2_add.csv
	- ./lab2_add --iterations=80 --threads=4 --yield >> lab2_add.csv
	- ./lab2_add --iterations=100 --threads=4 --yield >> lab2_add.csv
	- ./lab2_add --iterations=1000 --threads=4 --yield >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=4 --yield >> lab2_add.csv
	- ./lab2_add --iterations=100000 --threads=4 --yield >> lab2_add.csv
	- ./lab2_add --iterations=10 --threads=8 --yield >> lab2_add.csv
	- ./lab2_add --iterations=20 --threads=8 --yield >> lab2_add.csv
	- ./lab2_add --iterations=40 --threads=8 --yield >> lab2_add.csv
	- ./lab2_add --iterations=80 --threads=8 --yield >> lab2_add.csv
	- ./lab2_add --iterations=100 --threads=8 --yield >> lab2_add.csv
	- ./lab2_add --iterations=1000 --threads=8 --yield >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=8 --yield >> lab2_add.csv
	- ./lab2_add --iterations=100000 --threads=8 --yield >> lab2_add.csv
	- ./lab2_add --iterations=10 --threads=12 --yield >> lab2_add.csv
	- ./lab2_add --iterations=20 --threads=12 --yield >> lab2_add.csv
	- ./lab2_add --iterations=40 --threads=12 --yield >> lab2_add.csv
	- ./lab2_add --iterations=80 --threads=12 --yield >> lab2_add.csv
	- ./lab2_add --iterations=100 --threads=12 --yield >> lab2_add.csv
	- ./lab2_add --iterations=1000 --threads=12 --yield >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=12 --yield >> lab2_add.csv
	- ./lab2_add --iterations=100000 --threads=12 --yield >> lab2_add.csv

	#mutex, spinlock, CAS - graph 4
	- ./lab2_add --iterations=10000 --threads=1 --sync=m --yield >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=2 --sync=m --yield >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=4 --sync=m --yield >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=8 --sync=m --yield >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=12 --sync=m --yield >> lab2_add.csv
	- ./lab2_add --iterations=1000 --threads=1 --sync=s --yield >> lab2_add.csv
	- ./lab2_add --iterations=1000 --threads=2 --sync=s --yield >> lab2_add.csv
	- ./lab2_add --iterations=1000 --threads=4 --sync=s --yield >> lab2_add.csv
	- ./lab2_add --iterations=1000 --threads=8 --sync=s --yield >> lab2_add.csv
	- ./lab2_add --iterations=1000 --threads=12 --sync=s --yield >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=1 --sync=c --yield >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=2 --sync=c --yield >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=4 --sync=c --yield >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=8 --sync=c --yield >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=12 --sync=c --yield >> lab2_add.csv

	#amortized time with protection - graph 5
	# ./lab2_add --iterations=10000 --threads=1 >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=2 >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=4 >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=8 >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=12 >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=1 --sync=m >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=2 --sync=m >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=4 --sync=m >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=8 --sync=m >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=12 --sync=m >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=1 --sync=s >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=2 --sync=s >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=4 --sync=s >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=8 --sync=s >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=12 --sync=s >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=1 --sync=c >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=2 --sync=c >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=4 --sync=c >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=8 --sync=c >> lab2_add.csv
	- ./lab2_add --iterations=10000 --threads=12 --sync=c >> lab2_add.csv

	# lab2_list: single thread, increasing iterations - graph 1
	- ./lab2_list --iterations=10 > lab2_list.csv
	- ./lab2_list --iterations=100 >> lab2_list.csv
	- ./lab2_list --iterations=1000 >> lab2_list.csv
	- ./lab2_list --iterations=10000 >> lab2_list.csv
	- ./lab2_list --iterations=20000 >> lab2_list.csv

	- ./lab2_list --iterations=1 --threads=2 >> lab2_list.csv
	- ./lab2_list --iterations=10 --threads=2 >> lab2_list.csv
	- ./lab2_list --iterations=100 --threads=2 >> lab2_list.csv
	- ./lab2_list --iterations=1000 --threads=2 >> lab2_list.csv
	- ./lab2_list --iterations=1 --threads=4 >> lab2_list.csv
	- ./lab2_list --iterations=10 --threads=4 >> lab2_list.csv
	- ./lab2_list --iterations=100 --threads=4 >> lab2_list.csv
	- ./lab2_list --iterations=1000 --threads=4 >> lab2_list.csv
	- ./lab2_list --iterations=1 --threads=8 >> lab2_list.csv
	- ./lab2_list --iterations=10 --threads=8 >> lab2_list.csv
	- ./lab2_list --iterations=100 --threads=8 >> lab2_list.csv
	- ./lab2_list --iterations=1000 --threads=8 >> lab2_list.csv
	- ./lab2_list --iterations=1 --threads=12 >> lab2_list.csv
	- ./lab2_list --iterations=10 --threads=12 >> lab2_list.csv
	- ./lab2_list --iterations=100 --threads=12 >> lab2_list.csv
	- ./lab2_list --iterations=1000 --threads=12 >> lab2_list.csv

	# test yielding for inserts - graph 2
	- ./lab2_list --iterations=1 --threads=2 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=2 --threads=2 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=4 --threads=2 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=8 --threads=2 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=16 --threads=2 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=2 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=1 --threads=4 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=2 --threads=4 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=4 --threads=4 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=8 --threads=4 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=16 --threads=4 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=4 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=1 --threads=8 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=2 --threads=8 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=4 --threads=8 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=8 --threads=8 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=16 --threads=8 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=8 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=1 --threads=12 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=2 --threads=12 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=4 --threads=12 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=8 --threads=12 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=16 --threads=12 --yield=i >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=12 --yield=i >> lab2_list.csv

	# test yielding for deletes - graph 2
	- ./lab2_list --iterations=1 --threads=2 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=2 --threads=2 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=4 --threads=2 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=8 --threads=2 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=16 --threads=2 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=2 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=1 --threads=4 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=2 --threads=4 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=4 --threads=4 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=8 --threads=4 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=16 --threads=4 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=4 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=1 --threads=8 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=2 --threads=8 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=4 --threads=8 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=8 --threads=8 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=16 --threads=8 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=8 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=1 --threads=12 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=2 --threads=12 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=4 --threads=12 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=8 --threads=12 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=16 --threads=12 --yield=d >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=12 --yield=d >> lab2_list.csv

	# test yielding for both insert and lookup - graph 2
	- ./lab2_list --iterations=1 --threads=2 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=2 --threads=2 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=4 --threads=2 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=8 --threads=2 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=16 --threads=2 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=2 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=1 --threads=4 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=2 --threads=4 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=4 --threads=4 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=8 --threads=4 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=16 --threads=4 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=4 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=1 --threads=8 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=2 --threads=8 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=4 --threads=8 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=8 --threads=8 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=16 --threads=8 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=8 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=1 --threads=12 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=2 --threads=12 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=4 --threads=12 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=8 --threads=12 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=16 --threads=12 --yield=il >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=12 --yield=il >> lab2_list.csv

	# test yielding for both delete and lookup - graph 2
	- ./lab2_list --iterations=1 --threads=2 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=2 --threads=2 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=4 --threads=2 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=8 --threads=2 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=16 --threads=2 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=2 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=1 --threads=4 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=2 --threads=4 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=4 --threads=4 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=8 --threads=4 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=16 --threads=4 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=4 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=1 --threads=8 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=2 --threads=8 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=4 --threads=8 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=8 --threads=8 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=16 --threads=8 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=8 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=1 --threads=12 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=2 --threads=12 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=4 --threads=12 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=8 --threads=12 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=16 --threads=12 --yield=dl >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=12 --yield=dl >> lab2_list.csv

	# test mutex and spin lock options - graph 3
	- ./lab2_list --iterations=32 --threads=12 --yield=i --sync=m >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=12 --yield=d --sync=m >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=12 --yield=il --sync=m >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=12 --yield=dl --sync=m >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=12 --yield=i --sync=s >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=12 --yield=d --sync=s >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=12 --yield=il --sync=s >> lab2_list.csv
	- ./lab2_list --iterations=32 --threads=12 --yield=dl --sync=s >> lab2_list.csv

	# test sync option run times
	- ./lab2_list --iterations=1000 --threads=1 --sync=m >> lab2_list.csv
	- ./lab2_list --iterations=1000 --threads=2 --sync=m >> lab2_list.csv
	- ./lab2_list --iterations=1000 --threads=4 --sync=m >> lab2_list.csv
	- ./lab2_list --iterations=1000 --threads=8 --sync=m >> lab2_list.csv
	- ./lab2_list --iterations=1000 --threads=12 --sync=m >> lab2_list.csv
	- ./lab2_list --iterations=1000 --threads=16 --sync=m >> lab2_list.csv
	- ./lab2_list --iterations=1000 --threads=24 --sync=m >> lab2_list.csv
	- ./lab2_list --iterations=1000 --threads=1 --sync=s >> lab2_list.csv
	- ./lab2_list --iterations=1000 --threads=2 --sync=s >> lab2_list.csv
	- ./lab2_list --iterations=1000 --threads=4 --sync=s >> lab2_list.csv
	- ./lab2_list --iterations=1000 --threads=8 --sync=s >> lab2_list.csv
	- ./lab2_list --iterations=1000 --threads=12 --sync=s >> lab2_list.csv
	- ./lab2_list --iterations=1000 --threads=16 --sync=s >> lab2_list.csv
	- ./lab2_list --iterations=1000 --threads=24 --sync=s >> lab2_list.csv

graphs:
	gnuplot lab2_add.gp
	gnuplot lab2_list.gp

dist: tests graphs
	rm -f lab2_add lab2_list
	tar -czvf lab2a-305087992.tar.gz Makefile README $(LAB2ADDFILES) $(LAB2ADDPICS) $(LAB2LISTFILES)  $(LAB2LISTPICS)

clean:
	rm -f *.tar.gz lab2_add lab2_list

.PHONY: all build lab2_add lab2_list tests graphs dist clean
