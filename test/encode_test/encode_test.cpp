
#include <cmath> // floor
#include <cstdlib> // exit, atoi, atof
#include <unistd.h> // getopt, optarg, optind
#include <limits> // numeric_limits

#include "pulse.h"
#include "zhw_encode.h"

#ifndef DATAW
#define DATAW 64   // bitstream data width
#endif

#ifndef PREC
#define PREC 64 // floating-point number bit width
#endif

// #if PREC ==     16 // half
// #define EXPOW    5 // exponent bit width
// #define FRACW   10 // fraction bit width
// #define USE_HALF
#if PREC ==     32 // single
#define EXPOW    8 // exponent bit width
#define FRACW   23 // fraction bit width
#define USE_FLOAT
#elif PREC ==   64 // double
#define EXPOW   11 // exponent bit width
#define FRACW   52 // fraction bit width
#define USE_DOUBLE
#elif PREC ==   80 // ldouble
#define EXPOW   15 // exponent bit width
#define FRACW   64 // fraction bit width
#define USE_LDOUBLE
#elif PREC ==  128 // quad
#define EXPOW   15 // exponent bit width
#define FRACW  112 // fraction bit width
#define USE_QUAD
#else
#error "unknown precision"
#define EXPOW    1 // exponent bit width
#define FRACW    1 // exponent bit width
#endif
#include "real.h"

#ifndef _DIM
#define _DIM /*1*/ 2 /*3*/
#endif

#define BLOCK_LEN zhw::fpblk_sz(_DIM) /* values per block */

typedef fp_t<EXPOW,FRACW> fpn_t;  // floating-point number type
typedef bits_t<DATAW> enc_t;      // encoded bitstream type
typedef zhw::sconfig_t sconfig_t; // signed configuration parameter type
typedef zhw::uconfig_t uconfig_t; // unsigned configuration parameter type

#include "../test_data/bitstream_test_shim.h"
#define stringify( name ) #name
#define enumstringtuple( name ) "{" << name << "," << stringify(name) << "}"

#define DEFAULT_BLOCKS 1024/*1 use block size of the .bod datasets*/
#define DEFAULT_RATE (fpn_t::bits)
#define DEFAULT_DATA ISABEL
#define DEFAULT_VERBOSE false
#define DEFAULT_GENERATE false
static unsigned blocks = DEFAULT_BLOCKS;
static unsigned test_data = DEFAULT_DATA;
static double rate = DEFAULT_RATE;
static bool verbose = DEFAULT_VERBOSE;
static bool generate = DEFAULT_GENERATE;

static sc_time t0, t1;

double fpga_clks[4] = {0,100e6,50e6,20e6}; //FPGA clock rate for X,1D,2D,3D pipelines

// Process Prefix
// --------------
// mc: method combinatorial logic
// ms: method sequential logic
// ct: clocked thread
// th: thread

// Port & Channel Prefix
// ---------------------
// m: port master
// s: port slave
// c: channel
// u: module instance (unit)

// Send/Recv data on a stream.
template<typename I, typename O>
struct tb_driver : public sc_module
{
	sc_in<bool> clk;
	sc_in<bool> reset;

	sc_stream_in <I> s_port;
	sc_stream_out<O> m_port;

	int retval;

	bool tests_started;
	int totalTests;

	FILE* zfptr;

	sc_time getThroughput()		//In system time. Caller must convert to cycles.
		{return ((t1-t0)/totalTests);}

	~tb_driver(){if(zfptr != NULL)fclose(zfptr);}

	bitstream_tcase<fpn_t, enc_t>* test_case;

	void ct_send()
	{
		O flit;

		m_port.reset();
		wait();
		for (unsigned j = 0; j < blocks; j++) {
			for (unsigned i = 0; i < BLOCK_LEN; i++) {

				test_case->get_next_data_set_float(flit);
				m_port.write(flit); // write has call to wait();
			}
		}
	}

	void ct_recv()
	{
		//generate zfp data
		std::string bitstreamfnm;
		std::string zpath;


		if(generate)
		{
			bitstreamfnm = std::string("../test_data/zhw_results_data/") + zfp_files[test_data] + zfp_dim_str[_DIM-1] + std::string("_ZHW_") + std::to_string(_DIM) + std::string("D_rate_") + std::to_string(int(rate))+ std::string(".zfp");
			zpath =  MyPaths::mergePaths(MyPaths::getExecutableDir(),bitstreamfnm);

			zfptr = fopen(zpath.c_str(),"wb");
			M_Assert(zfptr != NULL, zpath.c_str());
		}

		I flit;
		I validationflit;

		s_port.reset();
		wait();
		for (unsigned j = 0; j < blocks; j++) {
			for (unsigned i = 0; ; i++)  {
				// wait();
				flit = s_port.read(); 						// get DUT flit (read has call to wait());
				if(verbose)
				{
					if(!test_case->get_next_zfp_stream_word(validationflit))	// get validation flit from .zfp file to compare with.
						validationflit.tdata = 0xbad;
					cout << "tb_driver::ct_recv ts: " << sc_time_stamp() << ", flit: 0x" << hex << flit.tdata << "ULL, validation: 0x" << validationflit.tdata << "ULL" << endl;
				}

				//write the zhw stream if required

				if(generate)
				{
					uint64_t dummy = flit.tdata.to_uint64();
					fwrite(&dummy,sizeof(uint64_t),1,zfptr);
				}

				if (flit.tlast)
				{
					if(!tests_started)
						{tests_started=true; t0= sc_time_stamp();}
					totalTests++;
					break;
				}
			}
		}
		t1 = sc_time_stamp();
	}

    tb_driver(sc_module_name name, bitstream_tcase<fpn_t, enc_t>* test_case)
        : sc_module(name),
		  clk("clk"),
		  reset("reset"),
		  retval(1),
		  test_case(test_case)
	{
		SC_CTHREAD(ct_send, clk.pos());
			reset_signal_is(reset, RLEVEL);

		SC_CTHREAD(ct_recv, clk.pos());
			reset_signal_is(reset, RLEVEL);
	}

    void process() {}

    SC_HAS_PROCESS(tb_driver);
};

#if defined(VCD)
namespace zhw {
	sc_trace_file *tf;
}
#endif

int sc_main(int argc , char *argv[])
{
	/* * * * * * * * * * get arguments * * * * * * * * * */
	int opt;
	bool nok = false;

	while ((opt = getopt(argc, argv, "d:b:r:t:vg")) != -1) {
		switch (opt) {
		case 'b':
			blocks = atoi(optarg);
			if (blocks < 1) {
				cerr << " -- error: number of blocks must be 1 or greater" << endl;
				nok = true;
			}
			break;
		case 'r':
			rate = atof(optarg);
			if (rate <= 0 || rate > fpn_t::bits) {
				cerr << " -- error: range must be 0 < rate <= " << fpn_t::bits << endl;
				nok = true;
			}
			break;
		case 'd':
			test_data = atoi(optarg);
			if (test_data < 0 || test_data > S3D) {
				cerr << " -- error: dataset be 0 <= dataset < " << CESM_ATM << endl;
				nok = true;
			}
			break;
		case 'v':
			verbose = true;
			break;
		case 'g':
			generate = true;
			break;
		default: /* unknown option */
			nok = true;
		}
	}
	if (nok || optind < argc) {
		cerr << "Usage: test -b<int> -r<fp> -t<int>" << endl;
		cerr << "  -b  number of blocks, default: " << DEFAULT_BLOCKS << endl;
		cerr << "  -r  rate, default: " << DEFAULT_RATE << endl;
		cerr << "  -d  data set, ["
									<< enumstringtuple(ISABEL) << ","
									<< enumstringtuple(NYX) << ","
									<< enumstringtuple(QMCPACK) << ","
									<< enumstringtuple(S3D) << ","
									<< enumstringtuple(CESM_ATM) << ","
									<< "]" << endl;
		cerr << "\t default: "<< DEFAULT_DATA << endl;
		cerr << endl;
		return EXIT_FAILURE;
	}

	/* * * * * * * * * * setup * * * * * * * * * */
	static_assert(sizeof(real_t)*8 == fpn_t::bits, "real_t must match fpn_t size");


	/* * * * * * * * * * SystemC start * * * * * * * * * */
	sc_report_handler::set_actions(SC_ID_IEEE_1666_DEPRECATION_, SC_DO_NOTHING);

	sc_clock clk("clk", 10, SC_NS); // create a 10ns period clock signal
	sc_signal<bool> reset("reset");

	sc_signal<uconfig_t> minbits("minbits");
	sc_signal<uconfig_t> maxbits("maxbits");
	sc_signal<uconfig_t> maxprec("maxprec");
	sc_signal<sconfig_t> minexp("minexp");

	bitstream_tcase<fpn_t, enc_t> test_case(_DIM,int(rate), test_data);

	minbits = test_case.minbits;
	maxbits = test_case.maxbits;
	maxprec = test_case.maxprec;
	minexp = test_case.minexp;

	sc_stream<fpn_t> c_driver_fp("c_driver_fp");
	sc_stream<enc_t> c_dut_enc("c_dut_enc");

	pulse<0,2,RLEVEL> u_pulse("u_pulse");

	tb_driver<enc_t, fpn_t> u_tb_driver("u_tb_driver",&test_case);

	zhw::encode<fpn_t, _DIM, enc_t> u_dut("u_dut");

	cout << "{" << endl;
	cout << "\"dims\": "   << _DIM  << ", " <<  endl;
	cout << "\"dataset\": " << "\"" << DataSetsStr[test_data] << "\"," << endl;
	cout << "\"rate\": "   << rate   << ", " << endl;
	cout << "\"blocks\": " << blocks << ", "<< endl;

	cout << "\"minbits\": " << minbits.get_new_value() << ", " << endl;
	cout << "\"maxbits\": " << maxbits.get_new_value() << ", " << endl;
	cout << "\"maxprec\": " << maxprec.get_new_value() << ", " << endl;
	cout << "\"minexp\": "  << minexp.get_new_value()  << endl;
	cout << "}," << endl;

	// connect reset
	u_pulse.clk(clk);
	u_pulse.sig(reset);

	// connect driver
	u_tb_driver.clk(clk);
	u_tb_driver.reset(reset);
	u_tb_driver.m_port(c_driver_fp);
	u_tb_driver.s_port(c_dut_enc);

	// connect DUT
	u_dut.clk(clk);
	u_dut.reset(reset);
	u_dut.minbits(minbits);
	u_dut.maxbits(maxbits);
	u_dut.maxprec(maxprec);
	u_dut.minexp(minexp);
	u_dut.s_fp(c_driver_fp);
	u_dut.m_bits(c_dut_enc);


#if defined(VCD)
	if(!verbose)
		std::cout.setstate(std::ios_base::failbit);	//turn off cout. shut up systemc.
	using zhw::tf;
	tf = sc_create_vcd_trace_file("test");
	tf->set_time_unit(1, SC_NS);
	sc_trace(tf, clk, clk.name());
	sc_trace(tf, reset, reset.name());
	sc_trace(tf, minbits, minbits.name());
	sc_trace(tf, maxbits, maxbits.name());
	sc_trace(tf, maxprec, maxprec.name());
	sc_trace(tf, minexp, minexp.name());
	sc_trace(tf, c_driver_fp, c_driver_fp.name());
	sc_trace(tf, c_dut_enc, c_dut_enc.name());
	if(!verbose)
		std::cout.clear();							//turn on cout
#endif


	cerr << "INFO: Simulating " << endl;

	// start simulation 
	// sc_start(600+(900*(int)blocks), SC_NS);
	sc_start(600+(2400*(int)blocks), SC_NS);


	double cycles_per_block = u_tb_driver.getThroughput()/clk.period();
	double bytes_per_block = zhw::fpblk_sz(_DIM)*(fpn_t::bits/8);
	double fpga_clk = fpga_clks[_DIM];
	cout << "{" << endl;
	cout << "\"Cycles/block\": " << std::fixed << cycles_per_block << ", " << endl;
	cout << "\"Bytes/block\": " << std::fixed << bytes_per_block << ", " << endl;
	cout << "\"FPGA clockrate\": " << std::scientific << fpga_clk << ", " << endl;
	cout << "\"Throughput\": " << std::scientific << (bytes_per_block/cycles_per_block)*fpga_clk  << endl;
	cout << "}" << endl;
	return 0;
}

/* TODO:
 *
   - Support various IEEE FP precisions half, single, double, quad
 * Support other ZFP compression modes besides fixed-rate?
 */
