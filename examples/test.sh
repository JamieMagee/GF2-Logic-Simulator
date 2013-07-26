#!/bin/bash
echo "" > test.txt
for f in *.gf2
do
	echo "processing $f..."
	echo "processing $f..." >> test.txt
	echo -e "r 50\r\nq\r\n" | ../src/logsim $f >> test.txt
	echo -e '\n' >> test.txt
done
