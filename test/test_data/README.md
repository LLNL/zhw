# ZFP Validation Datasets

This directory contains a set of validation data files built from the [SDRBench](https://sdrbench.github.io/) dataset. Samples of data have been extracted and organized into ZFP block order to create .bod files for the following datasets.
- cesm-atm  `CESM-ATM-4x4096.bod`
- isabel  `ISABEL-4x4x1024.bod`
- nyx  `NYX-4x4x1024.bod`
- qmcpack  `QMCPACK-4x4x1024.bod`
- s3d `S3D-4x4x1024.bod`

## Test Data generation

We have used software zfp to generate ZHW test vectors from SDRBench data.
Our steps can be reproduced with the provided `builddatasets.sh` script. This script applies a zfp compression and decompression step to the datasets over a range of parameters as follows:
1. Compress to yield \<dataset\>-[shape]\_\<dimension\>\_rate\_\<rate\>.zfp for example: 
`zfp -d -3 4 4 1024 -r 64 -i gold_results_data/nyx/NYX-4x4x1024.bod -z gold_results_data/nyx/NYX-4x4x1024_3D_rate_64.zfp` 
which generates
`NYX-4x4096_2D_rate_64.zfp`
2. Decompress to yield \<dataset\>-[shape]\_\<dimension\>\_rate\_\<rate\>\_zfp.bod for example:
`zfp -d -3 4 4 1024 -r 64 -z gold_results_data/nyx/NYX-4x4x1024_3D_rate_64.zfp -o gold_results_data/nyx/NYX-4x4x1024_3D_rate_64_zfp.bod`
which generates
`NYX-4x4x1024_3D_rate_64_zfp.bod`

## Example Test Data Set

An example file listing of test vectors for the CESM dataset is as follows :

 - CESM-ATM-4x4096_2D_rate_1.zfp       
 - CESM-ATM-4x4096_2D_rate_4_zfp.bod
 - CESM-ATM-4x4x1024_3D_rate_3.zfp 
 - CESM-ATM-4x4096_2D_rate_12.zfp     
 - CESM-ATM-4x4096_2D_rate_56.zfp       
 - CESM-ATM-4x4x1024_3D_rate_32.zfp 
 - CESM-ATM-4x4096_2D_rate_12_zfp.bod 
 - CESM-ATM-4x4096_2D_rate_56_zfp.bod   
 - CESM-ATM-4x4x1024_3D_rate_32_zfp.bod 
 - CESM-ATM-4x4096_2D_rate_16.zfp  
 - CESM-ATM-4x4096_2D_rate_6.zfp        
 - CESM-ATM-4x4x1024_3D_rate_3_zfp.bod
 - CESM-ATM-4x4096_2D_rate_16_zfp.bod  
 - CESM-ATM-4x4096_2D_rate_64.zfp   
 - CESM-ATM-4x4x1024_3D_rate_4.zfp 
 - CESM-ATM-4x4096_2D_rate_1_zfp.bod  
 - CESM-ATM-4x4096_2D_rate_64_zfp.bod   
 - CESM-ATM-4x4x1024_3D_rate_40.zfp 
 - CESM-ATM-4x4096_2D_rate_2.zfp      
 - CESM-ATM-4x4096_2D_rate_6_zfp.bod    
 - CESM-ATM-4x4x1024_3D_rate_40_zfp.bod 
 - CESM-ATM-4x4096_2D_rate_24.zfp  
  - CESM-ATM-4x4096_2D_rate_8.zfp        
  - CESM-ATM-4x4x1024_3D_rate_48.zfp 
  - CESM-ATM-4x4096_2D_rate_24_zfp.bod 
   - CESM-ATM-4x4096_2D_rate_8_zfp.bod    
   - CESM-ATM-4x4x1024_3D_rate_48_zfp.bod
   - CESM-ATM-4x4096_2D_rate_2_zfp.bod   
   - CESM-ATM-4x4x1024_3D_rate_1.zfp  
   - CESM-ATM-4x4x1024_3D_rate_4_zfp.bod 
   - CESM-ATM-4x4096_2D_rate_3.zfp    
   - CESM-ATM-4x4x1024_3D_rate_12.zfp     
   - CESM-ATM-4x4x1024_3D_rate_56.zfp 
   - CESM-ATM-4x4096_2D_rate_32.zfp     
   - CESM-ATM-4x4x1024_3D_rate_12_zfp.bod 
   - CESM-ATM-4x4x1024_3D_rate_56_zfp.bod
   - CESM-ATM-4x4096_2D_rate_32_zfp.bod  
   - CESM-ATM-4x4x1024_3D_rate_16.zfp 
   - CESM-ATM-4x4x1024_3D_rate_6.zfp 
   - CESM-ATM-4x4096_2D_rate_3_zfp.bod  
   - CESM-ATM-4x4x1024_3D_rate_16_zfp.bod 
   - CESM-ATM-4x4x1024_3D_rate_64.zfp 
   - CESM-ATM-4x4096_2D_rate_4.zfp      
   - CESM-ATM-4x4x1024_3D_rate_1_zfp.bod  
   - CESM-ATM-4x4x1024_3D_rate_64_zfp.bod 
   - CESM-ATM-4x4096_2D_rate_40.zfp  
   - CESM-ATM-4x4x1024_3D_rate_2.zfp      
   - CESM-ATM-4x4x1024_3D_rate_6_zfp.bod
   - CESM-ATM-4x4096_2D_rate_40_zfp.bod  
   - CESM-ATM-4x4x1024_3D_rate_24.zfp 
   - CESM-ATM-4x4x1024_3D_rate_8.zfp 
   - CESM-ATM-4x4096_2D_rate_48.zfp     
   - CESM-ATM-4x4x1024_3D_rate_24_zfp.bod 
   - CESM-ATM-4x4x1024_3D_rate_8_zfp.bod
   - CESM-ATM-4x4096_2D_rate_48_zfp.bod 
   - CESM-ATM-4x4x1024_3D_rate_2_zfp.bod

