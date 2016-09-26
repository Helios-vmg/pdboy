#pragma once

#include "RegisterStore.h"
#include "MemoryController.h"
#include "DisplayController.h"
#include "CommonTypes.h"
#include <cstdint>
#include <type_traits>

const unsigned gb_cpu_frequency = 4194304;
const double gb_cpu_clock_period_us = 1.0 / ((double)gb_cpu_frequency * 1e-6);

class Gameboy;

class GameboyCpu{
	Gameboy *system;
	RegisterStore registers;
	MemoryController memory_controller;
	std::uint64_t total_instructions = 0;
	main_integer_t current_pc = 0;

	main_integer_t decimal_adjust(main_integer_t);
	byte_t load_pc_and_increment();
#include "../generated_files/cpu.generated.h"

public:
	GameboyCpu(Gameboy &);
	void initialize();
	void take_time(main_integer_t cycles);
	void interrupt_toggle(bool);
	void stop();
	void halt();
	void abort();
	void run_one_instruction();
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
