#!/bin/bash
echo "" > error.txt
for f in *.gf2
do
	echo "processing $f..."
	echo "processing $f..." >> error.txt
	echo ../../src/logsim $f >> error.txt
	echo -e '\n' >> error.txt
done
