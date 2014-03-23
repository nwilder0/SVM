#ifndef KERNEL_H
#define KERNEL_H

#include <deque>
#include <queue>
#include <string>

#include "machine.h"
#include "process.h"

namespace vm
{
	// OS Kernel
    class Kernel
    {
    private:
        static const unsigned int _MAX_CYCLES_BEFORE_PREEMPTION = 100;
		static const unsigned int _MAX_PROCESS_PRIORITY = 99;
		static const unsigned int _PRIORITY_QUEUE_COUNT = 5;
		static const unsigned int _REALTIME_QUEUE = 4;

        Process::process_id_type _last_issued_process_id;
        Memory::ram_type::size_type _last_ram_position;

        unsigned int _cycles_passed_after_preemption;
        unsigned int _current_process_index;
		int _last_process_index[_PRIORITY_QUEUE_COUNT];
		int _current_queue, _switch_count;

	public:
        enum Scheduler
        {
            FirstComeFirstServed,
            ShortestJob,
            RoundRobin,
            Priority,
			PQueue
        };

        typedef std::deque<Process> process_list_type;
        typedef std::priority_queue<Process> process_priorities_type;

        Machine machine;

        process_list_type processes;
        process_priorities_type priorities;
		process_list_type pqueues[_PRIORITY_QUEUE_COUNT];

        Scheduler scheduler;

        Kernel(Scheduler scheduler, std::vector<Memory::ram_type> executables_paths, std::vector<int> priorities); // Kernel boot process (setup ISRs, create processes, etc.)
        virtual ~Kernel();

        void CreateProcess(Memory::ram_type &executable, int priority); // Creates a new PCB, places the executable image into memory

		void ExitProcess(Process);

		void LoadProcess(Process);
		void SaveProcess(Process);

    };
}

#endif
