CC=gcc
COMPILEROPTIONS = -Wall -Wextra
SEGFAULTERRNO = 139

default:
	$(CC) $(COMPILEROPTIONS) -g -o lab0 lab0.c

check: default test1A test1B test2 test3 test4 test5 test6 test7 test8 test9 test10 test11 test12
	if [ -s checkoutput.txt ]; then \
		cat checkoutput.txt; \
		echo -e "\nmake check result: smoke test FAILED"; \
		rm -f checkoutput.txt; \
	else \
		echo -e "\nmake check result: smoke test PASSED"; \
	fi; \
	rm -f checkdebug.txt

dist:
	tar -cvzf lab0-305087992.tar.gz lab0.c Makefile backtrace.png breakpoint.png README

clean:
	rm -f lab0 *.tar.gz *.txt

.PHONY: default check dist clean

test1A:
	# test input and output
	echo "Test 1 text" > test1A.txt; \
	echo "1A: Checking input/output" >> checkdebug.txt; \
	./lab0 --input=test1A.txt --output=test1B.txt
	cmp test1A.txt test1B.txt; \
	if [ $$? -ne 0 ]; then \
	 	echo "1A: Copying not performed successfully" >> checkoutput.txt; \
	fi; \
	rm -f test1A.txt test1B.txt

test1B:
	# test input and output with other form of argument passing
	echo "Test 1 text" > test1A.txt; \
	echo "1B: Checking input/output" >> checkdebug.txt; \
	./lab0 --input test1A.txt --output test1B.txt
	cmp test1A.txt test1B.txt; \
	if [ $$? -ne 0 ]; then \
	 	echo "1B: Copying not performed successfully" >> checkoutput.txt; \
	fi; \
	rm -f test1A.txt test1B.txt

test2:
	# test segfault
	echo "2: Check --segfault option" >> checkdebug.txt; \
	./lab0 --segfault; \
	if [ $$? -ne $(SEGFAULTERRNO) ]; then \
		echo "2: Error: segfault not performed properly" >> checkoutput.txt; \
	fi

test3:
	# test segfault and catch
	echo "3: Checking --segfault and --catch options" >> checkdebug.txt; \
	./lab0 --segfault --catch; \
	if [ $$? -ne 4 ]; then \
		echo "3: Error: segfault not caught properly or wrong exit code" >> checkoutput.txt; \
	fi

test4:
	#--input with no filename
	echo "4: --input with no filename" >> checkdebug.txt; \
	./lab0 --input; \
	if [ $$? -ne 1 ]; then \
		echo "4: incorrect --input behavior" >> checkoutput.txt; \
	fi

test5:
	# --input with bad filename
	echo "5: --input with bad filename" >> checkdebug.txt; \
	./lab0 --input=nofilehere1357924680; \
	if [ $$? -ne 2 ]; then\
		echo "5: failed to recognize nonexistent file" >> checkoutput.txt; \
	fi
	
test6:
	# same as test 4 but with --output
	echo "6: --output with no filename" >> checkdebug.txt; \
	./lab0 --output; \
	if [ $$? -ne 1 ]; then \
		echo "6: incorrect --output behavior" >> checkoutput.txt; \
	fi

test7:
	# test bad argument
	echo "7: Checking when bad parameters passed in" >> checkdebug.txt; \
	./lab0 --badparameter; \
	if [ $$? -ne 1 ]; then \
		echo "7: bad parameter not recognized" >> checkoutput.txt; \
	fi

test8:
	# pass in read permission denied file to read from
	echo "8: pass in read permission denied file to read from" >> checkdebug.txt; \
	touch noreadfile.txt; \
	chmod a-r noreadfile.txt; \
	./lab0 --input=noreadfile.txt; \
	if [ $$? -ne 2 ]; then \
		echo "8: incorrect behavior for unreadable file as input" >> checkoutput.txt; \
	fi; \
	rm -f noreadfile.txt

test9:
	# write to write permission denied file
	echo "9: writing to unwritable file" >> checkdebug.txt; \
	touch nowritefile.txt; \
	chmod a-w nowritefile.txt; \
	./lab0 --input=Makefile --output=nowritefile.txt; \
	if [ $$? -ne 3 ]; then \
		echo "9: incorrect behavior for unwritable file as output" >> checkoutput.txt; \
	fi; \
	rm -f nowritefile.txt

test10:
	# file to stdout
	echo "10: file to stdout" >> checkdebug.txt; \
	echo "test10 text" > test10.txt; \
	./lab0 --input=test10.txt > output.txt; \
	if [ $$? -ne 0 ]; then \
		echo "10: incorrect exit code, should be 0" >> checkoutput.txt; \
	fi; \
	cmp test10.txt output.txt; \
	if [ $$? -ne 0 ]; then \
		echo "10: failed copying from file to stdout"; \
	fi; \
	rm -f test10.txt output.txt

test11:
	# stdin to file
	echo "11: stdin to file" >> checkdebug.txt; \
	echo "test11 text" | ./lab0 --output=output.txt; \
	if [ $$? -ne 0 ]; then \
		echo "11: incorrect exit code, should be 0" >> checkoutput.txt; \
	fi; \
	echo "test11 text" > test11.txt; \
	cmp test11.txt output.txt; \
	if [ $$? -ne 0 ]; then \
		echo "11: failed copying from stdin to file"; \
	fi; \
	rm -f test11.txt output.txt

test12:
	# stdin to stdout
	echo "12: stdin to stdout" >> checkdebug.txt; \
	echo "test12 text" > test12.txt; \
	echo "test12 text" | ./lab0 > output.txt; \
	if [ $$? -ne 0 ]; then \
		echo "12: incorrect exit code, should be 0" >> checkoutput.txt; \
	fi; \
	cmp test12.txt output.txt; \
	if [ $$? -ne 0 ]; then \
		echo "12: failed copying from stdin to stdout"; \
	fi; \
	rm -f test12.txt output.txt
#for input file to output file, see test 1(a) and 1(b)
