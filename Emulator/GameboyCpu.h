#pragma once

#include "RegisterStore.h"
#include "MemoryController.h"
#include "DisplayController.h"
#include "CommonTypes.h"
#include <cstdint>
#include <type_traits>

const unsigned gb_cpu_frequency = 4194304;
const double gb_cpu_clock_period_us = 1.0 / ((double)gb_cpu_frequency * 1e-6);

class GameboyCpu{
	RegisterStore registers;
	MemoryController memory_controller;
	DisplayController display_controller;
	bool running = false;
	int break_on_address = -1;
	std::uint64_t total_cycles = 0;
	std::uint64_t total_instructions = 0;
	main_integer_t current_pc = 0;
	std::uint64_t counter_frequency = 0;

	main_integer_t decimal_adjust(main_integer_t);
#include "../generated_files/cpu.generated.h"
public:

	GameboyCpu();
	void initialize();
	void take_time(main_integer_t cycles);
	void interrupt_toggle(bool);
	void stop();
	void halt();
	void abort();
	void run();
	DisplayController &get_display_controller(){
		return this->display_controller;
	}
	std::uint64_t get_current_clock() const{
		return this->total_cycles;
	}
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
