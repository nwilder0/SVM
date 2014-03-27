#include "kernel.h"

#include <iostream>
#include <algorithm>

namespace vm
{
    Kernel::Kernel(Scheduler scheduler, std::vector<Memory::ram_type> executables_paths, std::vector<int> priorities)
        : machine(),
		  processes(),
		  priorities(),
		  scheduler(scheduler),
          _last_issued_process_id(0),
          _last_ram_position(0),
          _current_process_index(0),
          _cycles_passed_after_preemption(0),
		  _current_queue(_REALTIME_QUEUE)
    {
		for(int i=0; i<_PRIORITY_QUEUE_COUNT; i++) {
			pqueues[i] = process_list_type();
		}
		std::vector<int>::iterator it = priorities.begin();
        std::for_each(executables_paths.begin(), executables_paths.end(), [&](Memory::ram_type &executable) {
            CreateProcess(executable, *it);
			it++;
        });

        if (scheduler == FirstComeFirstServed || scheduler == ShortestJob) {
            machine.pic.isr_0 = [&]() {
				// Process the timer interrupt for the FCFS or Shortest Job scheduler
				// nothing to do - this interrupt shouldn't occur but if it does then ignore it
			};

        } else if (scheduler == RoundRobin) {
            machine.pic.isr_0 = [&]() {
				// ToDo: Process the timer interrupt for the Round Robin scheduler
				
				if(++_cycles_passed_after_preemption >= _MAX_CYCLES_BEFORE_PREEMPTION) {
					if(processes.size() > 1) {
						std::cout << "RR: Preempt: save process " << _current_process_index << std::endl;
						SaveProcess(&processes[_current_process_index]);
						_current_process_index++;
						if(_current_process_index >= processes.size()) {
							_current_process_index=0;
						}
						_cycles_passed_after_preemption = 0;
						std::cout << "RR: preempt: load process " << _current_process_index << std::endl;
						LoadProcess(&processes[_current_process_index]);
					}
				}
			};

        } else if (scheduler == Priority) {
            machine.pic.isr_0 = [&]() {
				// ToDo: Process the timer interrupt for the Priority Queue scheduler
				if(++_cycles_passed_after_preemption >= (_MAX_CYCLES_BEFORE_PREEMPTION+(processes[_current_process_index].priority*_MAX_CYCLES_BEFORE_PREEMPTION))) {
					
					if(processes[_current_process_index].priority) processes[_current_process_index].priority--;

					if(processes.size() > 1) {
						std::cout << "Priority: Preempt: save process " << _current_process_index << " with priority " << processes[_current_process_index].priority << std::endl;
						SaveProcess(&processes[_current_process_index]);
						_current_process_index++;
						if(_current_process_index >= processes.size()) {
							_current_process_index=0;
						}
						std::cout << "Priority: Preempt: load process " << _current_process_index << " with priority " << processes[_current_process_index].priority << std::endl;
						LoadProcess(&processes[_current_process_index]);
					}
				}

			};

        } else if (scheduler == PQueue) {
            machine.pic.isr_0 = [&]() {
				// ToDo: Process the timer interrupt for the Priority Queue scheduler
				if(++_cycles_passed_after_preemption >= _MAX_CYCLES_BEFORE_PREEMPTION) {
					if(pqueues[_current_queue][_current_process_index].priority) 
					{
						pqueues[_current_queue][_current_process_index].priority--;
						if(_current_queue != (pqueues[_current_queue][_current_process_index].priority / 20)) {
							pqueues[pqueues[_current_queue][_current_process_index].priority / 20].push_back(pqueues[_current_queue][_current_process_index]);
							pqueues[_current_queue].erase(pqueues[_current_queue].begin() + _current_process_index);
							_current_queue = _current_queue--;
							_current_process_index = pqueues[_current_queue].size()-1;
						}
					}

					int newQueue=-1, newIndex=-1;
					for(int i=_REALTIME_QUEUE; i>=0 && (newIndex == -1); i--) {
						if(pqueues[i].size()) {
							if(i==_current_queue) {
								if(_switch_count) {
									if(_current_process_index+1 < pqueues[_current_queue].size()) {
										newIndex = _current_process_index+1;
									} else {
										newIndex = 0;
									}
									newQueue = _current_queue;
									if(i!=_REALTIME_QUEUE) _switch_count--;
								}
							} else {
								if(i==_REALTIME_QUEUE) {
									newIndex = 0;
									newQueue = _REALTIME_QUEUE;
								} else if((i<_current_queue)||(!pqueues[_current_queue].size())) {									
									newQueue = i;
									newIndex = _last_process_index[i]+1;
									if(newIndex >= pqueues[i].size()) {
										newIndex = 0;
									}
								}
							}

						}
					}
					if(newIndex != -1) {
						_last_process_index[_current_queue] = _current_process_index;
						//std::cout << "PQueue: _current_queue = " << _current_queue << " _current_process_index = " << _current_process_index << " pqueues = " << sizeof(pqueues) / sizeof(process_list_type) << " pqueues[current] = " << pqueues[_current_queue].size();
						std::cout << "PQueue: Preempt: save process " << _current_process_index << " with priority " << pqueues[_current_queue][_current_process_index].priority << std::endl;
						SaveProcess(&pqueues[_current_queue][_current_process_index]);

						if(_current_queue != newQueue) _switch_count = newQueue;
						
						_current_queue = newQueue;
						_current_process_index = newIndex;
						std::cout << "PQueue: Preempt: load process " << _current_process_index << " with priority " << pqueues[_current_queue][_current_process_index].priority << std::endl;
							LoadProcess(&pqueues[_current_queue][_current_process_index]);
					}
					
				}

			};			
		}

		machine.pic.isr_3 = [&]() {
			// ToDo: Process the first software interrupt for the FCFS or Shortest Job scheduler
			Process *p;
			if (scheduler == PQueue) {
				p = &pqueues[_current_queue][_current_process_index];
			} else {
				p = &processes[_current_process_index];
			}
			switch(machine.cpu.registers.a) {
			case 0:
				ExitProcess(p);
			case 2:
				char c = machine.cpu.registers.b;
				std::cout << c;
			}

		};


		// ToDo

		// ---

        machine.Start();
    }

    Kernel::~Kernel() {}

    void Kernel::CreateProcess(Memory::ram_type &executable, int priority)
	{
        std::copy(executable.begin(),
				  executable.end(),
				  machine.memory.ram.begin() + _last_ram_position);

        Process process(_last_issued_process_id++,
					    _last_ram_position,
						_last_ram_position + executable.size());

        _last_ram_position += executable.size();

		if(priority > _MAX_PROCESS_PRIORITY) priority = _MAX_PROCESS_PRIORITY;
	
		process.priority = priority;

        // ToDo: add the new process to an appropriate data structure
		if ((scheduler == FirstComeFirstServed)||(scheduler == RoundRobin)) {
			processes.push_back(process);
		} else if (scheduler == ShortestJob) {

			process_list_type::iterator itr = processes.begin();
			
			while((itr != processes.end()) || (process.sequential_instruction_count >= (*itr).sequential_instruction_count)) {
				processes.insert(itr,process);
			}

		} else if (scheduler == Priority) {

			//process_list_type::iterator itr = processes.begin();
			int size = processes.size();
			int i;
			for(i = 0; i<size && process.priority <= processes[i].priority; i++);
			processes.insert(processes.begin()+i,process);

		} else if (scheduler == PQueue) {
			pqueues[process.priority / 20].push_back(process);
		}


		

    }

	void Kernel::ExitProcess(Process *p) 
	{
		if(scheduler == Kernel::PQueue) 
		{
			// initialize to not found
			int delIndex = -1, delQueue = -1, newIndex=-1, newQueue=-1;

			// check if the current process is the one being remove (likely for all causes except kill)
			if(p == &pqueues[_current_queue][_current_process_index]) 
			{
				delIndex = _current_process_index;
				delQueue = _current_queue;
			}
			// if not the current process then find it
			for(int j = 0; j<_PRIORITY_QUEUE_COUNT && (delIndex == -1); j++) {
				for(int i = 0; i<pqueues[j].size() && (delIndex == -1); i++) {
					if(p == &pqueues[j][i]) {
						delIndex = i;
						delQueue = j;
					}
				}
			}
			// as long as the process to remove was found, then remove it from the process list
			if(delIndex != -1) {
				pqueues[delQueue].erase(pqueues[delQueue].begin() + delIndex);
			}

			// if we're not at the end of the processes queue then the old current index now points to the next process
			for(int i = 0; i >= 0; i++) {
				if((_current_process_index<=pqueues[i].size())&&(_current_queue==i)) {
					newIndex = _current_process_index;
					newQueue = i;
				// otherwise go to beginning of the process queue
				} else if(!pqueues[i].empty()) {
					newIndex = 0;
					newQueue = i;
				}
			}
			if(newIndex != -1) {
				LoadProcess(&pqueues[newQueue][newIndex]);
			}

		}
		else 
		{
			std::cout << "Exit: exit process " << _current_process_index << std::endl;
			// initialize to not found
			int delIndex = -1;
			// check if the current process is the one being remove (likely for all causes except kill)
			if(p == &processes[_current_process_index]) 
			{
				delIndex = _current_process_index;
			}
			// if not the current process then find it
			for(int i = 0; i<processes.size() && (delIndex == -1); i++) {
				if(p == &processes[i]) {
					delIndex = i;
				}
			}
			// as long as the process to remove was found, then remove it from the process list
			if(delIndex != -1) {
				processes.erase(processes.begin() + delIndex);
			}

			// if we're not at the end of the processes queue then the old current index now points to the next process
			if(_current_process_index<processes.size()) {
				LoadProcess(&processes[_current_process_index]);
			// otherwise go to beginning of the process queue
			} else if(_current_process_index!=0) {
				_current_process_index = 0;
			} else {
				// no more processes exit VM or go to idle
			}
		}
	}

	void Kernel::SaveProcess(Process *p) 
	{
		p->registers = machine.cpu.registers;
		p->ChgState(Process::Ready);
		// what other elements need changing?
	}

	void Kernel::LoadProcess(Process *p) 
	{
		machine.cpu.registers = p->registers;
		p->ChgState(Process::Running);
		// what other elements need changing?
	}

}
