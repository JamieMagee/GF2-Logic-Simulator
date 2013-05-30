#!/bin/bash
for f in *.gf2
do
	echo "processing $f..."
	echo "processing $f..." >> errors.txt
	../../src/logsim $f >> errors.txt
	echo -e '\n' >> errors.txt
done
