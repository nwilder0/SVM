#include <vector>
#include <iostream>
#include <fstream>

#include "kernel.h"

namespace vm
{
	Memory::ram_type *LoadExecutable(const std::string &name)
	{
		Memory::ram_type *result = NULL;

		std::ifstream input_stream(name, std::ios::in | std::ios::binary);
		if (!input_stream) {
			std::cerr << "VM: failed to open the program file." << std::endl;
		} else {
			result = new Memory::ram_type();

			input_stream.seekg(0, std::ios::end);
			auto file_size = input_stream.tellg();
			input_stream.seekg(0, std::ios::beg);
			result->resize(static_cast<Memory::ram_size_type>(file_size) / 4);
			input_stream.read(reinterpret_cast<char *>(&(*result)[0]), file_size);

			if (input_stream.bad()) {
				delete result; result = NULL;
				std::cerr << "VM: failed to read the program file." << std::endl;
			}
		}

		return result;
	}
}

int main(int argc, char *argv[])
{
	using namespace vm;

    if (argc > 2) {
        std::string arg(argv[1]);

        Kernel::Scheduler scheduler;
        if (arg == "/scheduler:fcfs") {
            scheduler = Kernel::FirstComeFirstServed;
        } else if (arg == "/scheduler:sf") {
            scheduler = Kernel::ShortestJob;
        } else if (arg == "/scheduler:rr") {
            scheduler = Kernel::RoundRobin;
        } else if (arg == "/scheduler:priority") {
            scheduler = Kernel::Priority;
        } else if (arg == "/scheduler:pqueue") {
			scheduler = Kernel::PQueue;
		} else {
			std::cerr << "VM: Invalid scheduler type" << std::endl;
		}

        std::vector<Memory::ram_type> processes;
		std::vector<int> priorities;

        for (int i = 2; i < argc; ++i) {
			// read in filename and priority
			std::string strFilename(argv[i]);
			int priority = 0;
			
			if(i+1 < argc) {
				std::string strPArg(argv[i+1]);
				if(strPArg.substr(0,3)=="/p:") {
					std::string strPriority(strPArg.substr(3,std::string::npos));
					// convert to int
					try {
						priority = std::stoi(strPriority);
					} catch (int err) {
						std::cerr << "Exe: Invalid priority for " << strFilename << std::endl;
					}
					i++;
				}
			}

			Memory::ram_type *executable = LoadExecutable(strFilename);
			if (executable) {
				processes.push_back(*executable);
				delete executable;
				priorities.push_back(priority);
			}
        }

        Kernel kernel(scheduler, processes, priorities);
    }

    return 0;
}
