/*=============================================*\
|  			bitstream_test_shim.h				|
|	zhw test cases instantiated using			|
|	files created by the zfp library			|
\*=============================================*/

//convert bitstream file into testcase input

#ifndef DECODE_UNIT_TEST_BITSTREAM_TEST_SHIM_H_
#define DECODE_UNIT_TEST_BITSTREAM_TEST_SHIM_H_

#include "zhw.h"
#include <string>

//
//			UTILITY
//
#ifndef DECODE_UNIT_TEST_FLOAT_TEST_SHIM_H_

#if defined(_WIN32)
    #include <windows.h>
    #include <Shlwapi.h>
    #include <io.h>

    #define access _access_s
#endif

#ifdef __APPLE__
    #include <libgen.h>
    #include <limits.h>
    #include <mach-o/dyld.h>
    #include <unistd.h>
#endif

#ifdef __linux__
    #include <limits.h>
    #include <libgen.h>
    #include <unistd.h>

    #if defined(__sun)
        #define PROC_SELF_EXE "/proc/self/path/a.out"
    #else
        #define PROC_SELF_EXE "/proc/self/exe"
    #endif

#endif

namespace MyPaths {

#if defined(_WIN32)

std::string getExecutablePath() {
   char rawPathName[MAX_PATH];
   GetModuleFileNameA(NULL, rawPathName, MAX_PATH);
   return std::string(rawPathName);
}

std::string getExecutableDir() {
    std::string executablePath = getExecutablePath();
    char* exePath = new char[executablePath.length() + 1];
    strcpy_s(exePath, executablePath.length() + 1, executablePath.c_str());
    PathRemoveFileSpecA(exePath);
    std::string directory = std::string(exePath);
    delete[] exePath;
    return directory;
}

std::string mergePaths(std::string pathA, std::string pathB) {
  char combined[MAX_PATH];
  PathCombineA(combined, pathA.c_str(), pathB.c_str());
  std::string mergedPath(combined);
  return mergedPath;
}

#endif

#ifdef __linux__

std::string getExecutablePath() {
   char rawPathName[PATH_MAX];
   realpath(PROC_SELF_EXE, rawPathName);
   return  std::string(rawPathName);
}

std::string getExecutableDir() {
    std::string executablePath = getExecutablePath();
    executablePath.erase(executablePath.rfind('/'));
    return executablePath;
}

std::string mergePaths(std::string pathA, std::string pathB) {
  return pathA+"/"+pathB;
}

#endif

#ifdef __APPLE__
    std::string getExecutablePath() {
        char rawPathName[PATH_MAX];
        char realPathName[PATH_MAX];
        uint32_t rawPathSize = (uint32_t)sizeof(rawPathName);

        if(!_NSGetExecutablePath(rawPathName, &rawPathSize)) {
            realpath(rawPathName, realPathName);
        }
        return  std::string(realPathName);
    }

    std::string getExecutableDir() {
        std::string executablePath = getExecutablePath();
        char *executablePathStr = new char[executablePath.length() + 1];
        strcpy(executablePathStr, executablePath.c_str());
        char* executableDir = dirname(executablePathStr);
        delete [] executablePathStr;
        return std::string(executableDir);
    }

    std::string mergePaths(std::string pathA, std::string pathB) {
        return pathA+"/"+pathB;
    }
#endif


bool checkIfFileExists (const std::string& filePath)
{
   return access( filePath.c_str(), 0 ) == 0;
}


}

void M_Assert(bool expr, const char* msg)
{
    if (!expr)
    {
        std::cerr << "Assert failed:\t" << msg << std::endl;
        abort();
    }
}


#endif //DECODE_UNIT_TEST_FLOAT_TEST_SHIM_H_

enum DataSets
{
	ISABEL,
	NYX,
	QMCPACK,
	S3D,
	CESM_ATM
};

const char* DataSetsStr[]
{
	"ISABEL",
	"NYX",
	"QMCPACK",
	"S3D",
	"CESM-ATM"
};

const char* dataset_float_files[] =
{
		"isabel/ISABEL-4x4x1024.bod",
		"nyx/NYX-4x4x1024.bod",
		"qmcpack/QMCPACK-4x4x1024.bod",
		"s3d/S3D-4x4x1024.bod",
		"cesm-atm/CESM-ATM-4x4096.bod"
};

const char* zfp_files[] =
{
		"isabel/ISABEL-",
		"nyx/NYX-",
		"qmcpack/QMCPACK-",
		"s3d/S3D-",
		"cesm-atm/CESM-ATM-"
};

const char* zfp_dim_str[] =
{
		"4096",		//1D
		"4x4096",	//2D
		"4x4x1024"	//3D
};


//IO class for various zfp test cases
template<typename FP, typename B>
struct bitstream_tcase
{
	//typdefs
	typedef typename FP::ui_t ui_t;

	//members
	FILE* _datasetfloatdata;
	FILE* _floatdata;
	FILE* _bitstream;

	zhw::sconfig_t minexp;			//minimum exponent (per stream) used to compute per block maxprec
	zhw::uconfig_t minbits;			//minimum bits per block. Used in register file draining corner case(s) for decoding multiple zero blocks in one bitstream register
	zhw::uconfig_t maxbits;			//maximum bits per block. Used in register file draining corner case(s) for decoding trailing planes in one bitstream register
	zhw::uconfig_t maxprec;			//maximum precision (per stream) not to be confused with per-block maxprec, which is computed.


	int _dim, _rate, _dataset;

	//dummy
	bitstream_tcase(){}

	//initialize file pointers for test and validation vectors
	bitstream_tcase(int dim, int rate, int dataset):
		minexp(1-fp_t<EXPOW,FRACW>::ebias-fp_t<EXPOW,FRACW>::fbits), minbits(zhw::fpblk_sz(dim)*rate), maxbits(zhw::fpblk_sz(dim)*rate), maxprec(fp_t<EXPOW,FRACW>::bits),
		_dim(dim), _rate(rate), _dataset(dataset)
	{
		std::string SDR_benchfnm = std::string("../test_data/gold_results_data/") + dataset_float_files[dataset];
		std::string zfp_floatfnm = std::string("../test_data/gold_results_data/") + zfp_files[dataset] + zfp_dim_str[dim-1] + std::string("_") + std::to_string(dim) + std::string("D_rate_") + std::to_string(rate)+ std::string("_zfp.bod");
		std::string zfp_bitstreamfnm = std::string("../test_data/gold_results_data/") + zfp_files[dataset] + zfp_dim_str[dim-1] + std::string("_") + std::to_string(dim) + std::string("D_rate_") + std::to_string(rate)+ std::string(".zfp");

		std::string dpath =  MyPaths::mergePaths(MyPaths::getExecutableDir(),SDR_benchfnm);
		std::string fpath =  MyPaths::mergePaths(MyPaths::getExecutableDir(),zfp_floatfnm);
		std::string zpath =  MyPaths::mergePaths(MyPaths::getExecutableDir(),zfp_bitstreamfnm);
		//print expected path if data not found
		_datasetfloatdata = fopen(dpath.c_str(),"rb");
		M_Assert(_datasetfloatdata != NULL, dpath.c_str());
		_floatdata = fopen(fpath.c_str(),"rb");
		M_Assert(_floatdata != NULL, fpath.c_str());
		_bitstream = fopen(zpath.c_str(),"rb");
		M_Assert(_bitstream != NULL, zpath.c_str());
	}

	//cleanup
	~bitstream_tcase()
	{
		if(_datasetfloatdata)
			fclose(_datasetfloatdata);
		if(_floatdata)
			fclose(_floatdata);
		if(_bitstream)
			fclose(_bitstream);
	}

	//methods

	//Get the next float from validation file. Return false if EOF
	bool get_next_data_set_float(FP& next_float)
	{
		uint64_t IOfp;
		bool success =  fread(&IOfp,sizeof(uint64_t),1,_datasetfloatdata);	//Read bytes
		ui_t fp(IOfp);												//casting to FP is tricky...
		next_float = fp;											//but this works ok.
		return success;
	}						//attempt to get the next float for test case. Return false if end of floats reached

	//Get the next float from validation file. Return false if EOF
	bool get_next_zfp_float(FP& next_float)
	{
		uint64_t IOfp;
		bool success =  fread(&IOfp,sizeof(uint64_t),1,_floatdata);
		ui_t fp(IOfp);
		next_float = fp;
		return success;
	}

	//Get the next flit from bitstream file. Return false if EOF
	bool get_next_zfp_stream_word(B& next_word)
	{
		uint64_t IObs;
		bool success =  fread(&IObs,sizeof(IObs),1,_bitstream);	//Read bytes
		next_word.tdata = IObs;
		return success;
	}

};


#endif /* DECODE_UNIT_TEST_BITSTREAM_TEST_SHIM_H_ */
