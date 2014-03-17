#include "kernel.h"

#include <iostream>
#include <algorithm>

namespace vm
{
    Kernel::Kernel(Scheduler scheduler, std::vector<Memory::ram_type> executables_paths)
        : machine(),
		  processes(),
		  priorities(),
		  scheduler(scheduler),
          _last_issued_process_id(0),
          _last_ram_position(0),
          _current_process_index(0),
          _cycles_passed_after_preemption(0)
    {
        std::for_each(executables_paths.begin(), executables_paths.end(), [&](Memory::ram_type &executable) {
            CreateProcess(executable);
        });

        if (scheduler == FirstComeFirstServed || scheduler == ShortestJob) {
            machine.pic.isr_0 = [&]() {
				// ToDo: Process the timer interrupt for the FCFS or Shortest Job scheduler
				// nothing to do
			};

            machine.pic.isr_3 = [&]() {
                // ToDo: Process the first software interrupt for the FCFS or Shortest Job scheduler
				// Unload the current process
            };
        } else if (scheduler == RoundRobin) {
            machine.pic.isr_0 = [&]() {
				// ToDo: Process the timer interrupt for the Round Robin scheduler
				if(processes.size() > 1) {
					processes[_current_process_index].chgState(Process::States::Ready);
					_current_process_index++;
					if(_current_process_index >= processes.size()) {
						_current_process_index=0;
					}
					if(processes.size()) {
						processes[_current_process_index].chgState(Process::States::Running);
					}
				}
			};

            machine.pic.isr_3 = [&]() {
                // ToDo: Process the first software interrupt for the Round Robin scheduler
				// Unload the current process
				switch(machine.cpu.registers.a) {
				
				case 2:
					char c = machine.cpu.registers.b;
					std::cout << c;
				}

            };
        } else if (scheduler == Priority) {
            machine.pic.isr_0 = [&]() {
				// ToDo: Process the timer interrupt for the Priority Queue scheduler

			};

            machine.pic.isr_3 = [&]() {
                // ToDo: Process the first software interrupt for the Priority Queue scheduler
				// Unload the current process
            };
        }

		// ToDo

		// ---

        machine.Start();
    }

    Kernel::~Kernel() {}

    void Kernel::CreateProcess(Memory::ram_type &executable)
	{
        std::copy(executable.begin(),
				  executable.end(),
				  machine.memory.ram.begin() + _last_ram_position);

        Process process(_last_issued_process_id++,
					    _last_ram_position,
						_last_ram_position + executable.size());

        _last_ram_position += executable.size();

        // ToDo: add the new process to an appropriate data structure
		if ((scheduler == FirstComeFirstServed)||(scheduler == RoundRobin)) {
			processes.push_back(process);
		} else if (scheduler == ShortestJob) {

			process_list_type::iterator itr = processes.begin();
			
			while((itr != processes.end()) || (process.sequential_instruction_count >= (*itr).sequential_instruction_count)) {
				processes.insert(itr,process);
			}

		} else if (scheduler == Priority) {

			process_list_type::iterator itr = processes.begin();
			
			while((itr != processes.end()) || (process.priority <= (*itr).priority)) {
				processes.insert(itr,process);
			}

		}


		// ToDo: process the data structure

    }

	void Kernel::ExitProcess() 
	{

	}
}
