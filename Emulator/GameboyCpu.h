#include "RegisterStore.h"
#include "MemoryController.h"
#include <cstdint>
#include <type_traits>

class GameboyCpu{
	RegisterStore registers;
	MemoryController memory_controller;
	bool running = false;
	int break_on_address = -1;
#include "../generated_files/cpu.generated.h"
public:
	std::uint64_t total_cycles = 0;
	std::uint64_t total_instructions = 0;
	integer_type current_pc;

	GameboyCpu(): registers(*this){}
	void initialize();
	void take_time(integer_type cycles);
	void interrupt_toggle(bool);
	void stop();
	void halt();
	void abort();
	void run();
};

template <typename T>
T sign_extend8(T n){
#if 0
	if (n & 0x80)
		n |= ~(T)0xFF;
	return n;
#else
	return (T)(typename std::make_signed<T>::type)(char)n;
#endif
}
