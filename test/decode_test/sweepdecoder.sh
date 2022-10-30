#!/bin/bash

echo "[ [{\"decoder\":1024} ]" > decoder_results.txt
for rate in 1 2 3 4 6 8 12 16 24 32 40 48 56 64 
do
	echo ",[ [{\"rate\":$rate}]" 1>> decoder_results.txt
	for dataset in 0 1 2 3 4
	do
		echo ",[" 1>> decoder_results.txt
		./decode_test -d $dataset -r $rate -b 1024 >> decoder_results.txt 2> /dev/null
		echo "]" 1>> decoder_results.txt
	done
	echo "]" 1>> decoder_results.txt
done
echo "]" >> decoder_results.txt
