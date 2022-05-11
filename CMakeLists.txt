cmake_minimum_required(VERSION 3.12)
project(zhw)
set(LOCAL_INCLUDE_DIR "$ENV{HOME}/local/include") #assumes systemc is installed in local directory
set(LOCAL_LIB_DIR "$ENV{HOME}/local/lib")	#so the zhw makefile(s) can find systemc
include_directories(${LOCAL_INCLUDE_DIR}) 	#so the eclipse project can findsystemC 

include(ExternalProject)

#set compiler to clang
SET (CMAKE_C_COMPILER             "/usr/bin/clang")
SET (CMAKE_CXX_COMPILER             "/usr/bin/clang++")

#Enable verbose make output
set(CMAKE_VERBOSE_MAKEFILE ON)

#set debug flags if needed, make sure systemC include directory is passed to make.
string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)
if (uppercase_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
        message("C++ debug mode")
	set(OPT_ARGS "-L${LOCAL_LIB_DIR} -g -O1 -I${LOCAL_INCLUDE_DIR}")
endif ()

#=========================== DECODER TARGET =============
#This is for the original test target
#Add the build target, which is a makefile project in the 'test' sub directory
ExternalProject_Add(decode_unit_test
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test/decode_test #run build command in this directory
  CONFIGURE_COMMAND ""
  BUILD_COMMAND make clean && make OPT=${OPT_ARGS} D=VCD #Build 'test' program  with waveform output option
  INSTALL_COMMAND ""
)
#make it clear what exaclty is going to be built
message("Building zhw \"decode_test\" program (decoder test), with '.vcd' waveform output generation setting")

#=========================== ENCODER TARGET ============
ExternalProject_Add(zhw_test
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test/encode_test #run build command in this directory
  CONFIGURE_COMMAND ""
  BUILD_COMMAND make clean && make OPT=${OPT_ARGS} D=VCD #Build 'test' program  with waveform output option
  INSTALL_COMMAND ""
)
message("Building zhw \"encode_test\" program (encoder test) , with '.vcd' waveform output generation setting")


