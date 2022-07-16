#define DECODER_DIM /*1*/ 2		/*Which dimension of zhw decoder pipeline to test? todo: support 3D*/

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

#include "decode_tb.h"

#if defined(VCD)
namespace zhw {
	sc_trace_file *tf;
}
#endif

/*======================================================================*\
| 			wire up a zhw decoder test bench		 |
\*======================================================================*/
int sc_main(int argc , char *argv[])
{
	/* * * * * * * * * * SystemC start * * * * * * * * * */
	sc_report_handler::set_actions(SC_ID_IEEE_1666_DEPRECATION_, SC_DO_NOTHING);

	sc_clock clk("clk", 10, SC_NS); // create a 10ns period clock signal
	sc_signal<bool> reset("reset");
	pulse<0,2,RLEVEL> u_pulse("u_pulse");

	// connect reset
	u_pulse.clk(clk);
	u_pulse.sig(reset);

#if defined(VCD)
	using zhw::tf;
	tf = sc_create_vcd_trace_file(std::string("FULL_"+std::to_string(DECODER_DIM)+"D_DECODER").c_str());
	tf->set_time_unit(1, SC_NS);
	sc_trace(tf, clk, clk.name());
	sc_trace(tf, reset, reset.name());
#endif

	cout << CYAN << "INFO: " << RESET "Test decode" << endl;
	decode_full_tb<fpn_t,enc_t,DECODER_DIM> decode_stream_tb(clk,reset,"DUT_decode_full");

	cout << CYAN << "INFO" << RESET << ": Simulating " << endl;

	sc_start(55000, SC_NS);

	cout << CYAN "INFO" << RESET << ": Simulation done " << endl;
	return 0;
}

