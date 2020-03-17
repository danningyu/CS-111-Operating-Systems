#!/bin/bash

# Smoke test for CS 111 Lab 4B
# Danning Yu, 305087992

echo "Test 1: Check valid commands"

./lab4b <<-EOF
SCALE=F
SCALE=C
OFF
EOF
if [ $? -ne 0 ]; then
	echo "Test 1 FAILED" > failureMsgs.txt
fi

echo "Test 2: Check invalid commands"
./lab4b <<-EOF
SCALE=C
ABCDEF
START
START
STOP
OFF
EOF
if [ $? -ne 0 ]; then
	echo "Test 2 FAILED" >> failureMsgs.txt
fi

echo "Test 3: Check invalid args"
./lab4b --badargument
if [ $? -eq 0 ]; then
	echo "Test 3 FAILED" >> failureMsgs.txt
fi

echo "Test 4: Check logfile creation, logging of commands"
./lab4b --log=file.txt <<-EOF
SCALE=C
STOP
START
SCALE=F
PERIOD=2
LOG logtexthere
OFF
EOF
if [ ! -s file.txt ]; then
	echo "Test 4A FAILED" >> failureMsgs.txt
fi

grep "PERIOD" file.txt > /dev/null
if [ $? -ne 0 ]; then
	echo "Test 4B FAILED" >> failureMsgs.txt
fi

grep "SCALE" file.txt > /dev/null
if [ $? -ne 0 ]; then
	echo "Test 4C FAILED" >> failureMsgs.txt
fi

grep "LOG logtexthere" file.txt > /dev/null
if [ $? -ne 0 ]; then
	echo "Test 4D FAILED" >> failureMsgs.txt
fi

grep "SHUTDOWN" file.txt > /dev/null
if [ $? -ne 0 ]; then
	echo "Test 4E FAILED" >> failureMsgs.txt
fi

egrep '[0-9][0-9]:[0-9][0-9]:[0-9][0-9] [0-9]+\.[0-9]\>' file.txt > /dev/null
if [ $? -ne 0 ]; then
	echo "Test 4F FAILED" >> failureMsgs.txt
fi

rm -f file.txt
