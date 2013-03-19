// TestBoost.cpp : définit le point d'entrée pour l'application console.
//

// #include "stdafx.h"
#include "aq/ParallelProcess.h"

#include <iostream>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>

typedef std::vector<boost::uint64_t> vint_t;

void function1(boost::uint64_t v, unsigned int threadId)
{
	if ((v % 100000) == 0)
	{
		std::cout << "[" << boost::this_thread::get_id() << "]  "
				  << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ 
					<< ": [v=" << v << ";threadId=" << threadId << "]" << std::endl;
	}

	// consume cpu
	for (unsigned int i = 0; i < 100; i++)
	{
		v = 3.14 * v * v * v * 4 / 3;
	}
}

int main(int argc, char ** argv)
{
	try 
	{
		if (argc < 4)
		{
			std::cerr << argv[0] << " needs 3 arguments: [nb thread] [vector size] [test_vector|test_tabc]" << std::endl; 
			return EXIT_FAILURE;
		}

		boost::uint64_t nbProc = boost::lexical_cast<boost::uint64_t>(argv[1]);
		boost::uint64_t size = boost::lexical_cast<boost::uint64_t>(argv[2]);

		if (strcmp(argv[3], "test_vector") == 0) // Test 1
		{
			std::cout << size << std::endl;
			vint_t vint(size);

			//
			// fill
			{
				std::cout << "fill vector" << std::endl;
				boost::timer::auto_cpu_timer t; // timer from now
				for (boost::uint64_t i = 0; i < size; i++)
				{
					vint[i] = i;
				}
			}

			std::cout << "================" << std::endl;

			//
			// execute
			{
				std::cout << "process vector with " << nbProc << " threads" << std::endl;
				boost::timer::auto_cpu_timer t; // timer from now
				ParallelProcess<boost::uint64_t>(vint, function1, nbProc);
			}
		}
		else if (strcmp(argv[3], "test_tabc") == 0) // Test 2
		{
			std::cout << "================" << std::endl;
			std::cout << "================" << std::endl;
			std::cout << size << std::endl;
			boost::uint64_t * vint = new boost::uint64_t[size];

			//
			// fill
			{
				std::cout << "fill vector" << std::endl;
				boost::timer::auto_cpu_timer t; // timer from now
				for (boost::uint64_t i = 0; i < size; i++)
				{
					vint[i] = i;
				}
			}
			
			std::cout << "================" << std::endl;

			//
			// execute
			{
				std::cout << "process tabc with " << nbProc << " threads" << std::endl;
				boost::timer::auto_cpu_timer t; // timer from now
				ParallelProcess<boost::uint64_t>(vint, size, function1, nbProc);
			}
		}
		else
		{
			std::cerr << "no test available" << std::endl;
			return EXIT_FAILURE;
		}
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

