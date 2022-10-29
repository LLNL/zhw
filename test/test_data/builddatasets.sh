#!/bin/bash

for rate in 1 2 3 4 6 8 12 16 24 32 40 48 56 64 
do
	for dataset in  cesm-atm #isabel nyx qmcpack nyx #cesm-atm
	do
		upperstr=$(echo $dataset | tr '[:lower:]' '[:upper:]')
		#2D dimensions
		cmd="zfp -d -2 4 4096 -r $rate -i gold_results_data/$dataset/$upperstr-4x4096.bod -z gold_results_data/$dataset/$upperstr-4x4096_2D_rate_$rate.zfp"
		echo $cmd
		$cmd > /dev/null 2> /dev/null
		cmd="zfp -d -2 4 4096 -r $rate -z gold_results_data/$dataset/$upperstr-4x4096_2D_rate_$rate.zfp -o gold_results_data/$dataset/$upperstr-4x4096_2D_rate_"$rate"_zfp.bod"
		echo $cmd
		$cmd > /dev/null 2> /dev/null
		#3D dimensions
		cmd="zfp -d -3 4 4 1024 -r $rate -i gold_results_data/$dataset/$upperstr-4x4096.bod -z gold_results_data/$dataset/$upperstr-4x4x1024_3D_rate_$rate.zfp"
		echo $cmd
		$cmd > /dev/null 2> /dev/null
		cmd="zfp -d -3 4 4 1024 -r $rate -z gold_results_data/$dataset/$upperstr-4x4x1024_3D_rate_$rate.zfp -o gold_results_data/$dataset/$upperstr-4x4x1024_3D_rate_"$rate"_zfp.bod"
		echo $cmd
		$cmd > /dev/null 2> /dev/null
	done
done

#cesm has different data arrangement, so different name, so a whole seperate command line...

for rate in 1 2 3 4 6 8 12 16 24 32 40 48 56 64 
do
	for dataset in  s3d isabel nyx qmcpack nyx #cesm-atm
	do
		upperstr=$(echo $dataset | tr '[:lower:]' '[:upper:]')
		#2D dimensions
		cmd="zfp -d -2 4 4096 -r $rate -i gold_results_data/$dataset/$upperstr-4x4x1024.bod -z gold_results_data/$dataset/$upperstr-4x4096_2D_rate_$rate.zfp"
		echo $cmd
		$cmd > /dev/null 2> /dev/null
		cmd="zfp -d -2 4 4096 -r $rate -z gold_results_data/$dataset/$upperstr-4x4096_2D_rate_$rate.zfp -o gold_results_data/$dataset/$upperstr-4x4096_2D_rate_"$rate"_zfp.bod"
		echo $cmd
		$cmd > /dev/null 2> /dev/null
		#3D dimensions
		cmd="zfp -d -3 4 4 1024 -r $rate -i gold_results_data/$dataset/$upperstr-4x4x1024.bod -z gold_results_data/$dataset/$upperstr-4x4x1024_3D_rate_$rate.zfp"
		echo $cmd
		$cmd > /dev/null 2> /dev/null
		cmd="zfp -d -3 4 4 1024 -r $rate -z gold_results_data/$dataset/$upperstr-4x4x1024_3D_rate_$rate.zfp -o gold_results_data/$dataset/$upperstr-4x4x1024_3D_rate_"$rate"_zfp.bod"
		echo $cmd
		$cmd > /dev/null 2> /dev/null
	done
done
