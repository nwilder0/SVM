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

	bool Process::chgState(States newState) {

		if(newState == Ready && state == Running) {
			loadState();
		} else if (state == Running && newState == Ready) {
			saveState();
		} else if (state == Ready && newState == Blocked) {

		} else if (state == Running && newState == Blocked) {
			saveState();
		} else if (state == Blocked && newState == Ready) {

		}
		// load/unload state into PCB
		return true;
	}

	bool Process::saveState() {
		return false;
	}

	bool Process::loadState() {
		return false;
	}
}
