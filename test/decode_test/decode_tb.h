#ifndef DECODE_FULL_TB
#define DECODE_FULL_TB

#include "zhwconfig.h"
#include "tcasestream_2D.h"
#include "tcasestream_2D_rate64.h"

//test cases available with compile switches.
#define DECODE_FULL_TB_MULTI_BLOCK_TEST
//#define DECODE_FULL_TB_EXECUTION_STARVATION_TEST
//#define DECODE_FULL_TB_DOWNSTREAM_STALL_TEST
//#define DECODE_FULL_TB_ZERO_BLOCK_TEST

#define DECODE_FULL_TB_START_BLOCK 0 /*which block to start decoding and validating in the stream?*/
#define DECODE_FULL_TB_END_BLOCK 255	  /*which block to end decoding and validating in the stream?*/

/*======================================================================*\
|	TEST CLASS: 			decode_full								 	 |
\*======================================================================*/
//NOTE: the 'B' is maintained purely for symetric IO.
//B is treated as a POD array when instantiating the decoder
template<typename FP, typename B, int DIM>
class decode_full_tb
{
	typedef typename FP::si_t si_t;
	typedef typename FP::ui_t ui_t;

	//TB Drive signals
	sc_signal<bool> &_clk;
	sc_signal<bool> &_reset;

	//DUT
	zhw::decode<FP, DIM, B> _dut;

	//Signals for DUT plumbing

	//(control path)
	sc_signal <uconfig_t> _c_s_maxbits;						//number of bits per block
	sc_signal <uconfig_t> _c_s_maxprec;						//precision of an int in a block (in bits)
	sc_signal <uconfig_t> _c_s_minbits;
	sc_signal <sconfig_t> _c_s_minexp; 						// assume biased

	//(data path)
	sc_stream<B> _c_s_bits;									//Bitstream for dut containing blocks with header and data

	sc_stream<FP> _c_m_stream;								//the output of the DUT is fp_t (opposite of encoder)
	
	//=================		DUT TEST BENCH DRIVER DEFINITION ======================
	SC_MODULE(decode_full_tb_driver)
	{

		//TB status
		public: bool testpassed;
		public: int start_block, end_block;

		//drive signals
		sc_in<bool> clk;
		sc_in<bool> reset;

		//test cases (see tcaseunits.h)
		tcase_decode_full<FP, B, DIM, SEQUENTIAL_BLOCKS_256> test0;	//Get test case[0] data

		//---------- DUT IO PORTS AND CONTROL SIGNALS ------------------

		//DUT input signals
		sc_out <uconfig_t> 	s_maxbits;	//tb writes_to->dut.s_maxbits
		sc_out <uconfig_t> 	s_maxprec;	//etc...
		sc_out <uconfig_t> 	s_minbits;
		sc_out <sconfig_t> 	s_minexp;
		sc_stream_out<B>  	s_bits;
		//DUT output signals
		sc_stream_in<FP> m_stream;	//tb reads_from<-dut.m_stream

		//Misc members (various test cases)

		//------------------------------- DUT input send -----------------------------------------

		//fixed rate case

		//No stress on module, complete decode
		void simple_stream_send()
		{
			size_t i = start_block*2;//block in stream to start from (assumes 2 words per block)
			size_t nblocks = end_block-start_block;	//number of encoded blocks in test case
			B streamword;			//for a symetric interface, the encoder bitstream abstraction is used in the decoder
									//However, it is highly desirable to replace this datatype, which is not well suited to task.

			//set values for used configuration ports
			s_maxprec.write(test0.maxprec);
			s_minbits.write(test0.minbits);
			s_maxbits.write(test0.maxbits);
			s_minexp.write(test0.minexp);

			//hold every control signal at 0
			s_bits.reset();			//dut<-the bitstream is outputting valid data at this time.
			s_bits.valid_w(false);
			wait();
			cout << CYAN << "INFO: " <<YELLOW << "simple_stream() " << RESET << "test @ " << sc_time_stamp() << endl;

			//output bitstream words as requested by the decoder.
			while(i<(nblocks*test0.words_per_block)+30) //each block is encoded over 2 ui_t's in tcaseunits.h (128 = minbits = maxbits, and, 1 ui_t = 64 bits)
			{
				if(s_bits.ready_r())
				{
					streamword.tdata=test0.s_stream[i++];
					s_bits.data_w(streamword);
					s_bits.valid_w(true);
				}
				wait();
			}

			//cout <<RED << "DEBUG: COUT DISABLED!!!" << RESET << "@: " << sc_time_stamp() <<endl; cout.rdbuf(NULL);
		}


		//Execution Starvation: Delay in FIFO loading causing stall
		void single_block_execution_starvation_send()
		{

			cout <<RED << "DEBUG: NOT IMPLEMENTED!! COUT DISABLED!!!" << RESET << "@: " << sc_time_stamp() <<endl; cout.rdbuf(NULL);

		}

		//Backpressure: Downstream module does not request data.
		void single_block_backpressure_send()
		{
			cout <<RED << "DEBUG: NOT IMPLEMENTED!! COUT DISABLED!!!" << RESET << "@: " << sc_time_stamp() <<endl; cout.rdbuf(NULL);
		}

		//Nought / zero block decode. 64 zero blocks followed by test case 0.
		void nought_block_test_send()
		{
			size_t i = 0;//block in stream to start from (assumes 2 words per block)
			size_t nblocks = 1;//15;	//number of encoded blocks in test case
			B streamword;			//for a symetric interface, the encoder bitstream abstraction is used in the decoder
									//However, it is highly desirable to replace this datatype, which is not well suited to task.

			//set values for used configuration ports
			s_maxprec.write(FP::bits);
			s_minbits.write(128);
			s_maxbits.write(128);
			s_minexp.write(-1027);

			//hold every control signal at 0
			s_bits.reset();			//dut<-the bitstream is outputting valid data at this time.
			s_bits.valid_w(false);
			wait();
			cout << CYAN << "INFO: " <<YELLOW << "simple_stream() " << RESET << "test @ " << sc_time_stamp() << endl;

			//output bitstream words as requested by the decoder.
			while(i<(nblocks*2)+30) //each block is encoded over 2 ui_t's in tcaseunits.h (128 = minbits = maxbits, and, 1 ui_t = 64 bits)
			{
				if(s_bits.ready_r())
				{
					if(!(i&1))
						streamword.tdata=(~ui_t(0) & ~ui_t(0)<<1);//zero bit clear (a zero block) otherwise all f's.
					else
						streamword.tdata=(~ui_t(0));
					s_bits.data_w(streamword);
					s_bits.valid_w(true);
				}
				wait();
			}

		}

		//------------------------ DUT IO LOGIC -----------------------------
		void ct_send()	//Thread to drive -="/*decode_stream*/ dut INPUT"=- and control signals
		{
			//which test to run?
			#if defined(DECODE_FULL_TB_MULTI_BLOCK_TEST)
			simple_stream_send();
			#elif defined(DECODE_FULL_TB_EXECUTION_STARVATION_TEST)
			single_block_execution_starvation_send();
			#elif defined(DECODE_FULL_TB_DOWNSTREAM_STALL_TEST)
			single_block_backpressure_send();
			#elif defined(DECODE_FULL_TB_ZERO_BLOCK_TEST)
			nought_block_test_send();
			#endif

		}

		//------------------------------- DUT output receive -----------------------------------------

		//fixed rate case

		//No stress on module, complete decode
		void simple_stream_recv()	//Thread to read -="/*decode_stream*/ dut OUTPUT"=- and status signals
		{
			FP blklog[zhw::fpblk_sz(DIM)];		//floating point results (block abstraction)
			FP zero = 0;

			//This thread checks behavior of the dut
			int which_test = 0;
			std::string which_time = "N/A";

			testpassed = true;

			//enable input.
			 m_stream.ready_w(true);

			//TEST 0
			wait();
			wait();	//stop console messages getting messy

			//check multiple decoded blocks
			int blocks = end_block;
			cout << CYAN << "INFO" << RESET << " will test " << CYAN << end_block-start_block << RESET << " consecutive block decodes from block: " << CYAN << start_block  << RESET " to " << CYAN << end_block << RESET << endl;
			for(int h=start_block; h<blocks; h++)
			{
				cout << CYAN "INFO" << RESET<< " Validate "<< DIM << "D block " << dec << CYAN<< h << RESET << " decoding (13 plane test block) @: " << sc_time_stamp();
				while(!m_stream.valid_r())
					wait();					//wait until data is clocked through.

				for (int i=0; i< zhw::fpblk_sz(DIM); )
				{
					if(m_stream.valid_r()&& m_stream.ready_ref())
					{
						blklog[i] = m_stream.data_r();
						if(!(blklog[i] == FP(test0.m_fp_stream[i+((zhw::fpblk_sz(DIM)*h))])))
							testpassed=false;
						i++;
					}
					wait();

				}
				if(testpassed)
					cout << " ... complete: " << GREEN << " PASSED " << RESET << " @ " << sc_time_stamp() << endl;
				else
					cout << " ... complete: " << RED << " FAILED " << RESET << " @ " << sc_time_stamp() << endl;

				if(!testpassed)
				{
					for(int i=0; i< zhw::fpblk_sz(DIM); i++)cout << RED << "OURS" << RESET << " (m_stream): " << blklog[i] << YELLOW << " GOLD " << RESET <<  " (m_fp_stream): " << test0.m_fp_stream[i+((zhw::fpblk_sz(DIM)*h))] << endl;
					cout << endl;
					which_time = sc_time_stamp().to_string();
					break;
				}
				else
					which_test++;
			}

			//report status
			cout << endl << CYAN << "INFO" << RESET << ": test decode_stream complete" << endl;
			cout << CYAN "STATUS" << RESET << ": test decode_stream ";
			if(!testpassed)
				cout << RED << "ERROR:" << RESET << " decode_stream control failed at time: " <<which_time << "ns, test: " << which_test << endl;
			else
				cout << GREEN << "PASSED" <<  RESET << std::dec << ": " << which_test << " tests" << endl;

		}

		//zero block decode (multiple)
		//this test sends all 1b's except for every maxbit/minbit. This will result in n 0 blocks if decode stream correctly skips pad bits.
		void nought_block_test_recv()	//Thread to read -="/*decode_stream*/ dut OUTPUT"=- and status signals
		{
			FP blklog[zhw::fpblk_sz(DIM)];		//floating point results (block abstraction)
			FP zero = 0;

			int which_test = 0;
			std::string which_time = "N/A";
			testpassed = true;

			//enable input.
			 m_stream.ready_w(true);
			wait();
			wait();	//stop console messages getting messy

			//check multiple decoded blocks
			cout << CYAN << "INFO" << RESET << " will test " << CYAN << 15 << RESET << " consecutive zero block decodes from block: " << CYAN << 0  << RESET " to " << CYAN << 15 << RESET << endl;
			for(int h=0; h<15; h++)
			{
				cout << CYAN "INFO" << RESET<< " Validate "<< DIM << "D block " << dec << CYAN<< h << RESET << " decoding (13 plane test block) @: " << sc_time_stamp();
				while(!m_stream.valid_r())
					wait();					//wait until data is clocked through.

				for (int i=0; i< zhw::fpblk_sz(DIM); )
				{
					if(m_stream.valid_r()&& m_stream.ready_ref())
					{
						blklog[i] = m_stream.data_r();
						if(!(blklog[i] == zero))
							testpassed=false;
						i++;
					}
					wait();

				}
				if(testpassed)
					cout << " ... complete: " << GREEN << " PASSED " << RESET << " @ " << sc_time_stamp() << endl;
				else
					cout << " ... complete: " << RED << " FAILED " << RESET << " @ " << sc_time_stamp() << endl;

				if(!testpassed)
				{
					for(int i=0; i< zhw::fpblk_sz(DIM); i++)cout << RED << "OURS" << RESET << " (m_stream): " << blklog[i] << YELLOW << " GOLD " << RESET <<  " (m_fp_stream): " << test0.m_fp_stream[i+((zhw::fpblk_sz(DIM)*h))] << endl;
					cout << endl;
					which_time = sc_time_stamp().to_string();
					break;
				}
				else
					which_test++;
			}

			//report status
			cout << endl << CYAN << "INFO" << RESET << ": test decode_stream complete" << endl;
			cout << CYAN "STATUS" << RESET << ": test decode_stream ";
			if(!testpassed)
				cout << RED << "ERROR:" << RESET << " decode_stream control failed at time: " <<which_time << "ns, test: " << which_test << endl;
			else
				cout << GREEN << "PASSED" <<  RESET << std::dec << ": " << which_test << " tests" << endl;

		}

		void ct_recv()	//Thread to drive -="/*decode_stream*/ dut INPUT"=- and control signals
		{
			//which test to run?
			#if defined(DECODE_FULL_TB_MULTI_BLOCK_TEST)
			simple_stream_recv();
			#elif defined(DECODE_FULL_TB_EXECUTION_STARVATION_TEST)
			single_block_execution_starvation_recv();
			#elif defined(DECODE_FULL_TB_DOWNSTREAM_STALL_TEST)
			single_block_backpressure_recv();
			#elif defined(DECODE_FULL_TB_ZERO_BLOCK_TEST)
			nought_block_test_recv();
			#endif

		}

		//pipe out all signals of interest
#if defined(VCD)
		void start_of_simulation()
		{
			//check drive is working
			sc_trace(zhw::tf, clk, (std::string(name())+".clk").c_str());
			sc_trace(zhw::tf, reset, (std::string(name())+".reset").c_str());
			//DUT input signals
			sc_trace(zhw::tf, s_maxbits, (std::string(name())+".s_maxbits").c_str());
			sc_trace(zhw::tf, s_maxprec, (std::string(name())+".s_maxprec").c_str());
			sc_trace(zhw::tf, s_minbits, (std::string(name())+".s_minbits").c_str());
			sc_trace(zhw::tf, s_minexp, (std::string(name())+".s_minexp").c_str());
			sc_trace(zhw::tf, s_bits, (std::string(name())+".s_bits").c_str());
			//DUT output signals
			sc_trace(zhw::tf, m_stream, (std::string(name())+".m_stream").c_str());
		}
#endif

		//plumb levels of clk and reset to c thread processes defined above...
		SC_CTOR(decode_full_tb_driver)
		{
			SC_CTHREAD(ct_send, clk.pos());
				reset_signal_is(reset, RLEVEL);

			SC_CTHREAD(ct_recv, clk.pos());
				reset_signal_is(reset, RLEVEL);

		}

	};

	decode_full_tb_driver _tb;	//class instance of testbench.

	//=================		DUT PLUMBING FUNCTION		=====================
	//class instantiation.
	public: decode_full_tb(sc_signal<bool> &clk,
						sc_signal<bool> &reset,
						const char* dut_name		//required that dut module is initialized with a given name...
												):
												_clk(clk),		//initialize all systemc instances immidiately (required)
												_reset(reset),
												_dut(dut_name),
												_tb("decode_full_tb_driver")
	{

		//----------------- connect tb to clock, reset, and dut ------
		_dut.clk(_clk);
		_dut.reset(_reset);

		_tb.clk(_clk);
		_tb.reset(_reset);

		//plumb together dut input to test bench
		//configuration
		_dut.maxbits(_c_s_maxbits);
		_tb.s_maxbits(_c_s_maxbits);
		_dut.maxprec(_c_s_maxprec);
		_tb.s_maxprec(_c_s_maxprec);
		_dut.minbits(_c_s_minbits);
		_tb.s_minbits(_c_s_minbits);
		_dut.minexp(_c_s_minexp);
		_tb.s_minexp(_c_s_minexp);

		//data
		_dut.s_bits(_c_s_bits);
		_tb.s_bits(_c_s_bits);

		//plumb together dut output to test bench
		//data
		_tb.m_stream(_c_m_stream);
		_dut.m_stream(_c_m_stream);

		//initialize test result flag to false.
		_tb.testpassed = false;
		//starting block in stream to decode
		_tb.start_block = DECODE_FULL_TB_START_BLOCK;
		_tb.end_block = DECODE_FULL_TB_END_BLOCK;
	}

	bool testPassed(){return _tb.testpassed;}	//interrogate result of test.

	~decode_full_tb(){}

};

#endif //DECODE_FULL_TB
