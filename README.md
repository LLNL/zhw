# ZHW - ZFP Hardware Implementation
ZFP is a floating-point compression format gaining traction in high-performance computing applications. A software implementation has demonstrated the ability to reduce data movement across communication channels and to reduce the footprint of floating-point arrays in memory. Nevertheless, any benefit in performance is limited to the spare compute cycles available before reaching bandwidth limitations. A hardware implementation of ZFP has the potential to substantially improve HPC application performance.

The hardware implementation of ZFP is sourced in SystemC to facilitate its evaluation in various architectures. The encode pipeline consists of several modules. Uncompressed blocks of floating-point numbers in IEEE format flow into the pipeline in a stream and a compressed bitstream flows out from the pipeline. The decode pipeline is the inverse of the encode pipeline. A modular design enables number formats other than IEEE (such as posits) to be considered with minor adaptations. Best performance will be realized from the hardware ZFP unit when batches of blocks are processed at a time. The implementation supports 1-D, 2-D and 3-D blocks of floating-point numbers. C++ template parameters are used to specify the bit width of floating-point numbers and the array dimension of the encoder.


### Getting Started
- Build and install the SystemC library **with c++ 11** support
- Clone or copy the zhw repository to your system
- Set your build toolchain to clang-12 or higher

#### Building Using CMake
- Create an out of source build directory and navigate to it (e.g. mkdir ../zhw_build && cd ../zhw_build)
- Invoke cmake specifying the zhw project directory as a build target (e.g. cmake ../zhw)
- Type "make" to build baseline encoder and decoder 2D test programs with .vcd output generation enabled

SystemC will be autodetected provided it was integrated into your CMake cache during install (default behavior).

Environment variables:

- LD_LIBRARY_DIR: Must contain a path to the systemc dynamic library
- PATH: Must contain a path to the install root of the systemc library

A 'make' only build environment is provided but is deprecated. navigate to the testbench directories and type `make help` for information.

#### Testing

Test bench programs have been created with several test cases from the [SDRbench](https://sdrbench.github.io/) data set. ZHW configuration can be modified in these test benches via #define settings, or command line parameters. see test/encode_test/encode_test.cpp or test/decode_test/decode_test.cpp for compiler parameters, or, run `decode_test -h` / `encode_test -h` to review command line parameters. We also provide the parameter sweeping scripts `sweepencoder.sh` and `sweepdecoder.sh` which can be used to validate a matrix of ZHW parameters on SDRBench sample data sets. 


### References
1. Michael Barrow, Zhuanhao Wu, Scott Lloyd, Maya Gokhale, Hiren Patel, and Peter Lindstrom, "ZHW: A Numerical CODEC for Big Data Scientific Computation", IEEE International Conference on Field Programmable Technology, December 2022.
	- note: SystemC clang (synthesizable) compilations of the ZHW [encoder](https://github.com/anikau31/systemc-clang/tree/fpt-2022/examples/llnl-examples/zfpsynth/zfp3) and [decoder](https://github.com/anikau31/systemc-clang/tree/fpt-2022/examples/llnl-examples/zfpsynth/zfp7) pertaining to this manuscript are also available.
	
2. Peter Lindstrom, "Fixed-Rate Compressed Floating-Point Arrays," IEEE Transactions on Visualization and Computer Graphics, 20(12):2674-2683, December 2014. [doi:10.1109/TVCG.2014.2346458](http://doi.org/10.1109/TVCG.2014.2346458).

3. ZFP on GitHub, [http://github.com/LLNL/zfp](http://github.com/LLNL/zfp)

4. SDRBench on GitHub, [https://sdrbench.github.io/]

### Dependencies
The open source SystemC library is needed for simulation of ZHW.
[http://workspace.accellera.org/downloads/standards/systemc](http://workspace.accellera.org/downloads/standards/systemc)

### Contributing
Bug fixes, new features, applications, or hardware modules can be submitted as [pull requests](http://help.github.com/articles/using-pull-requests/).
All new contributions must be open source and not contain any proprietary code.

### License
ZHW is distributed under the terms of the BSD 3-Clause license.
See LICENSE and NOTICE for details.
SPDX-License-Identifier: BSD-3-Clause

LLNL-CODE-811758
