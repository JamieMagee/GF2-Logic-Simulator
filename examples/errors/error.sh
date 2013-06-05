#!/bin/bash
echo "" > error.txt
for f in *.gf2
do
	echo "processing $f..."
	echo "processing $f..." >> error.txt
	echo -e "r 50\r\nq\r\n" | ../../src/logsim $f >> error.txt
	echo -e '\n' >> error.txt
done
