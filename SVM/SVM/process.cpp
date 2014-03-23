#include "process.h"

namespace vm
{
    Process::Process(process_id_type id, Memory::ram_size_type memory_start_position,
                                         Memory::ram_size_type memory_end_position)
        : id(id),
		  registers(),
		  state(Ready),
		  priority(0),
          memory_start_position(memory_start_position),
          memory_end_position(memory_end_position)
    {
        registers.ip = memory_start_position;

        sequential_instruction_count = (memory_end_position - memory_start_position) / 2;
    }

    Process::~Process() {}

    bool Process::operator<(const Process &anotherProcess) const {
        return priority < anotherProcess.priority;
    }

	bool Process::ChgState(States newState) {

		state = newState;
		if(newState == Ready && state == Running) {
			// anything else to do 
		} else if (state == Running && newState == Ready) {
			// anything else to do
		} else if (state == Ready && newState == Blocked) {
			// anything else to do
		} else if (state == Running && newState == Blocked) {
			// anything else to do
		} else if (state == Blocked && newState == Ready) {
			// anything else to do
		}
		// load/unload state into PCB
		return true;
	}


}
