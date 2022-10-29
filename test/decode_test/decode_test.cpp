#define DECODER_DIM /*1*/ 2	/*3*/ /*Which dimension of zhw decoder pipeline to test? */

//the following are UBUNTU/LINUX, and MacOS ONLY terminal color codes.
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

#include "zhwconfig.h"
#include "../test_data/bitstream_test_shim.h"

#if defined(VCD)
namespace zhw {
	sc_trace_file *tf;
}
#endif

#define stringify( name ) #name
#define enumstringtuple( name ) "{" << name << "," << stringify(name) << "}"

#define DEFAULT_RATE (fpn_t::bits)
#define DEFAULT_DATA ISABEL
#define DEFAULT_VERBOSE false
static unsigned blocks = DEFAULT_BLOCKS;
static double rate = DEFAULT_RATE;
static unsigned test_data = DEFAULT_DATA;
static bool verbose = DEFAULT_VERBOSE;

static sc_time t0, t1;				//start and end times of test;

double fpga_clks[4] = {0,50e6,20e6,20e6};

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
// Translate between the I/O buffer types and SystemC types.
// The tb_driver is not intended to be synthesizable.
template<typename FP, typename B, int DIM>
struct tb_driver : public sc_module
{
	sc_in<bool> clk;
	sc_in<bool> reset;

	sc_stream_in <FP> m_port;
	sc_stream_out<B> s_port;

	int retval;


	bool tests_started;
	bool testpassed;
	int totalTests;

	sc_time getThroughput()		//In system time. Caller must convert to cycles.
		{return ((t1-t0)/totalTests);}


	bitstream_tcase<fpn_t, enc_t>* ptest_case;


	void ct_send()
	{

		size_t words_per_block = ptest_case->maxbits/B::dbits;
		size_t i = 0;			//offset by maxbits/bits_per_bitstream_word
		size_t nblocks = blocks;//number of encoded blocks in test case
		B streamword;			//for a symetric interface, the encoder bitstream abstraction is used in the decoder
								//However, it is highly desirable to replace this datatype, which is not well suited to task.

		//hold every control signal at 0
		s_port.reset();			//dut<-the bitstream is outputting valid data at this time.
		s_port.valid_w(false);
		wait();
		cout << CYAN << "INFO: " <<YELLOW << "simple_stream() " << RESET << "test @ " << sc_time_stamp() << endl;

		//skip to first block of interest
		for(size_t j =0; j< i; j++)
			for(size_t k =0; k < words_per_block; k++)
				ptest_case->get_next_zfp_stream_word(streamword);

		//output bitstream words as requested by the decoder.
		while(i<(nblocks*words_per_block)+4) //In the 2D case, each block is encoded over 2 ui_t's in tcaseunits.h (128 = minbits = maxbits, and, 1 ui_t = 64 bits)
		{
			if(s_port.ready_r())
			{
				if(!ptest_case->get_next_zfp_stream_word(streamword))	//still data to get?
					streamword.tdata=0x0badbadbadbadbadULL;

				s_port.data_w(streamword);
				s_port.valid_w(true);
			}
			wait();
		}

	}

	//------------------------------- DUT output receive -----------------------------------------
	void ct_recv()
	{

		FP zhwlog[zhw::fpblk_sz(DIM)];		//floating point results (block abstraction)
		FP tstlog[zhw::fpblk_sz(DIM)];
		FP zero = 0;

		//This thread checks behavior of the dut
		int which_test = 0;
		bool tests_started = false;

		std::string which_time;

		testpassed = true;
		totalTests = 0;

		//enable input.
		m_port.ready_w(true);

		//TEST 0
		wait();
		wait();	//stop console messages getting messy

		//check multiple decoded blocks

		cout << CYAN << "INFO" << RESET << " will test " << CYAN << blocks << RESET << blocks << endl;
		for(int h=0; h<blocks; h++)
		{
			if(verbose)
				cout << CYAN "INFO" << RESET<< " Validate "<< DIM << "D block " << dec << CYAN<< h << RESET << " @: " << sc_time_stamp();
			while(!m_port.valid_r())
				wait();					//wait until data is clocked through.

			for (int i=0; i< zhw::fpblk_sz(DIM); )
			{
				if(m_port.valid_r()&& m_port.ready_ref())
				{
					zhwlog[i] = m_port.data_r();

					FP test_float;
					if(!ptest_case->get_next_zfp_float(test_float))
						test_float = 0xbadULL;
					tstlog[i] = test_float;
					if(!(zhwlog[i] == test_float))
						testpassed=false;
					i++;
				}
				wait();

			}
			if(verbose)
			{
				if(testpassed)
					cout << " ... complete: " << GREEN << " PASSED " << RESET << " @ " << sc_time_stamp() << endl;
				else
					cout << " ... complete: " << RED << " FAILED " << RESET << " @ " << sc_time_stamp() << endl;

				if(!testpassed)
				{
					for(int i=0; i< zhw::fpblk_sz(DIM); i++)
					{
						cout << RED << "OURS" << RESET << " (m_port): " << zhwlog[i];
						cout << YELLOW << " GOLD " << RESET <<  " (m_fp_stream): " << tstlog[i] << endl;
					}
					cout << endl;
					which_time = sc_time_stamp().to_string();
cout << RED << "DEBUG" << RESET << "continuing to process..." << endl;
//					break;
				}
			}

			which_test++;

			totalTests++;

			if(!tests_started)
				{tests_started=true; t0= sc_time_stamp();}
		}
		t1 = sc_time_stamp();

		cout << "verbose: " << verbose << endl;

		if(verbose)
		{
			//report status
			cout << endl << CYAN << "INFO" << RESET << ": test decode_bitstream complete " << endl;
			cout << CYAN "STATUS" << RESET << ": test time " << (t1 - t0) << endl;
			cout << CYAN "STATUS" << RESET << ": total tests " << totalTests << endl;
			cout << CYAN "STATUS" << RESET << ": time per test " << (t1 - t0)/(totalTests-1) << endl;
			cout << CYAN "STATUS" << RESET << ": test decode_stream ";
			if(!testpassed)
				cout << RED << "ERROR:" << RESET << " decode_stream control failed at time: " <<which_time << ", test: " << which_test << endl;
			else
				cout << GREEN << "PASSED" <<  RESET << std::dec << ": " << which_test << " tests" << endl;
		}

	}

    tb_driver(sc_module_name name, bitstream_tcase<fpn_t, enc_t>* test_case)
        : sc_module(name),
		  clk("clk"),
		  reset("reset"),
		  retval(1),
		  ptest_case(test_case)
	{
		SC_CTHREAD(ct_send, clk.pos());
			reset_signal_is(reset, RLEVEL);

		SC_CTHREAD(ct_recv, clk.pos());
			reset_signal_is(reset, RLEVEL);
	}

    void process() {}

    SC_HAS_PROCESS(tb_driver);

};

int sc_main(int argc , char *argv[])
{
	/* * * * * * * * * * get arguments * * * * * * * * * */
	int opt;
	bool nok = false;

	while ((opt = getopt(argc, argv, "d:b:r:t:v")) != -1) {
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
			if (test_data < 0 || test_data > CESM_ATM) {
				cerr << " -- error: dataset must be 0 < dataset <= " << CESM_ATM << endl;
				nok = true;
			}
			break;
		case 'v':
			verbose = true;
			break;
		default: /* unknown option */
			nok = true;
		}
	}
	if (nok || optind < argc) {
		cerr << "Usage: decode_unit_test -b<int> -r<fp> -s<int> -d<int> -v" << endl;
		cerr << "  -b  number of blocks, default: " << DEFAULT_BLOCKS << endl;
		cerr << "  -r  rate, default: " << DEFAULT_RATE << endl;
		cerr << "  -v  verbose, default: " << DEFAULT_VERBOSE << endl;
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
	bitstream_tcase<fpn_t, enc_t> test_case(DECODER_DIM,rate, test_data);
	/* * * * * * * * * * SystemC start * * * * * * * * * */
	sc_report_handler::set_actions(SC_ID_IEEE_1666_DEPRECATION_, SC_DO_NOTHING);

	sc_clock clk("clk", 10, SC_NS); // create a 10ns period clock signal
	sc_signal<bool> reset("reset");

	sc_signal<uconfig_t> minbits("minbits");
	sc_signal<uconfig_t> maxbits("maxbits");
	sc_signal<uconfig_t> maxprec("maxprec");
	sc_signal<sconfig_t> minexp("minexp");

	minbits = test_case.minbits;
	maxbits = test_case.maxbits;
	maxprec = test_case.maxprec;
	minexp = test_case.minexp;

	sc_stream<fpn_t> c_dut_fp("c_driver_fp");
	sc_stream<enc_t> c_driver_enc("c_dut_enc");

	pulse<0,2,RLEVEL> u_pulse("u_pulse");
	tb_driver<fpn_t, enc_t, DECODER_DIM> u_tb_driver("u_tb_driver",&test_case);

	//zhw::encode<fpn_t, DIM, enc_t> u_dut("u_dut");
	zhw::decode<fpn_t, DECODER_DIM, enc_t> u_dut("u_dut");

	cout << "{" << endl;
	cout << "\"dims\": "   << DECODER_DIM  << ", " <<  endl;
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
	u_tb_driver.m_port(c_dut_fp);
	u_tb_driver.s_port(c_driver_enc);

	// connect DUT
	u_dut.clk(clk);
	u_dut.reset(reset);
	u_dut.minbits(minbits);
	u_dut.maxbits(maxbits);
	u_dut.maxprec(maxprec);
	u_dut.minexp(minexp);
	//u_dut.s_fp(c_driver_fp);
	//u_dut.m_bits(c_dut_enc);
	u_dut.s_bits(c_driver_enc);
	u_dut.m_stream(c_dut_fp);

	if(!verbose)
		std::cout.setstate(std::ios_base::failbit);	//turn off cout. shut up systemc.
#if defined(VCD)
	using zhw::tf;
	tf = sc_create_vcd_trace_file("FULL_DECODER");
	tf->set_time_unit(1, SC_NS);
	sc_trace(tf, clk, clk.name());
	sc_trace(tf, reset, reset.name());
	sc_trace(tf, minbits, minbits.name());
	sc_trace(tf, maxbits, maxbits.name());
	sc_trace(tf, maxprec, maxprec.name());
	sc_trace(tf, minexp, minexp.name());
	sc_trace(tf, c_driver_enc, c_driver_enc.name());
	sc_trace(tf, c_dut_fp, c_dut_fp.name());
#endif

	cerr << "INFO: Simulating " << endl;

	// start simulation
	sc_start(600+(2400*(int)blocks), SC_NS);
	std::cout.clear();							//turn on cout

	double cycles_per_block = u_tb_driver.getThroughput()/clk.period();
	double bytes_per_block = zhw::fpblk_sz(DECODER_DIM)*(fpn_t::bits/8);
	double fpga_clk = fpga_clks[DECODER_DIM];			//use aproporiate clock.
	cout << "{" << endl;
	cout << "\"Cycles/block\": " << std::fixed << cycles_per_block << ", " << endl;
	cout << "\"Bytes/block\": " << std::fixed << bytes_per_block << ", " << endl;
	cout << "\"FPGA clockrate\": " << std::scientific << fpga_clk << ", " << endl;
	cout << "\"Throughput\": " << std::scientific << (bytes_per_block/cycles_per_block)*fpga_clk  << endl;
	cout << "}" << endl;

	return 0;
}
