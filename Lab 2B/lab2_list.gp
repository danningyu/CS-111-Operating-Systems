#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#	lab2_list-1.png ... cost per operation vs threads and iterations
#	lab2_list-2.png ... threads and iterations that run (un-protected) w/o failure
#	lab2_list-3.png ... threads and iterations that run (protected) w/o failure
#	lab2_list-4.png ... cost per operation vs number of threads
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

# how many threads/iterations we can run without failure (w/o yielding)
### GRAPH 1 ###
set title "List-1: Throughput vs threads with mutex and spin-lock protection"
set xlabel "Number of Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (operations/second)"
set logscale y 10
set output 'lab2b_1.png'

plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'mutex synchronized list' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'spin-lock synchronized list' with linespoints lc rgb 'green'

### GRAPH 4 ###
### For some reason graph 4 after 3 breaks so I'm putting it here instead ###
set title "List-4: Throughput with sublist partitioning, mutex synchronization"
set xlabel "Number of Threads"
set logscale x 2
set xrange [0.75:30]
set ylabel "Throughput (operations/second)"
set logscale y
set yrange [:50000000]
set output 'lab2b_4.png'
set key right top
plot \
	 "< grep -E \"list-none-m,[0-9],1000,1,|list-none-m,12,1000,1,\" lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '1 sublist' with linespoints lc rgb 'blue', \
	 "< grep -e 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '4 sublists' with linespoints lc rgb 'green', \
	 "< grep -e 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '8 sublists' with linespoints lc rgb 'red', \
	 "< grep -e 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '16 sublists' with linespoints lc rgb 'violet'

### GRAPH 5 ###
set title "List-5: Throughput with sublist partitioning, spin-lock synchronization"
set xlabel "Number of Threads"
set logscale x 2
set xrange [0.75:30]

set ylabel "Throughput (operations/second)"
set logscale y
set yrange [:50000000]
set output 'lab2b_5.png'
set key right top
plot \
	"< grep -E \"list-none-s,[0-9],1000,1,|list-none-s,12,1000,1,\" lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '1 sublist' with linespoints lc rgb 'blue', \
	 "< grep -e 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '4 sublists' with linespoints lc rgb 'green', \
	 "< grep -e 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '8 sublists' with linespoints lc rgb 'red', \
	 "< grep -e 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '16 sublists' with linespoints lc rgb 'violet'

### GRAPH 2 ###
set title "List-2: Mutexes: time per operation and lock acquisition times"
set xlabel "Number of Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Time (ns)"
set logscale y 10
set output 'lab2b_2.png'
# note that unsuccessful runs should have produced no output
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'Average time per operation' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
	title 'Mutex lock acquisition time' with linespoints lc rgb 'green'

### GRAPH 3 ###
set title "List-3: Unprotected and protected successful runs, id yields, 4 sublists"
set xlabel "Number of Threads"
set logscale x 2
set xrange [0.75:20]
set ylabel "Successful Iterations"
set yrange [:500]
set logscale y 10
set output 'lab2b_3.png'
# note that unsuccessful runs should have produced no output
plot \
     "< grep list-id-none lab2b_list.csv" using ($2):($3) \
	title 'w/o protection' with points lc rgb 'green', \
     "< grep list-id-m lab2b_list.csv" using ($2):($3) \
	title 'mutex protection' with points lc rgb 'red', \
     "< grep list-id-s lab2b_list.csv" using ($2):($3) \
	title 'spin-lock protection' with points lc rgb 'blue'