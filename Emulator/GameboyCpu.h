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
	bool interrupts_enabled = false;
	byte_t interrupt_flag = 0;
	byte_t interrupt_enable_flag = 0;

	static const unsigned vblank_flag_bit = 0;
	static const unsigned lcd_stat_flag_bit = 1;
	static const unsigned timer_flag_bit = 2;
	static const unsigned serial_flag_bit = 3;
	static const unsigned joypad_flag_bit = 4;

	static const unsigned vblank_mask   = 1 << vblank_flag_bit;
	static const unsigned lcd_stat_mask = 1 << lcd_stat_flag_bit;
	static const unsigned timer_mask    = 1 << timer_flag_bit;
	static const unsigned serial_mask   = 1 << serial_flag_bit;
	static const unsigned joypad_mask   = 1 << joypad_flag_bit;

	static const unsigned vblank_interrupt_handler_address    = 0x0040 + 8 * vblank_flag_bit;
	static const unsigned lcd_stat_interrupt_handler_address  = 0x0040 + 8 * lcd_stat_flag_bit;
	static const unsigned timer_interrupt_handler_address     = 0x0040 + 8 * timer_flag_bit;
	static const unsigned serial_interrupt_handler_address    = 0x0040 + 8 * serial_flag_bit;
	static const unsigned joypad_interrupt_handler_address    = 0x0040 + 8 * joypad_flag_bit;

	main_integer_t decimal_adjust(main_integer_t);
	byte_t load_pc_and_increment();
	void attempt_to_handle_interrupts();

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
	void vblank_irq();
	byte_t get_interrupt_flag() const;
	void set_interrupt_flag(byte_t b);
	byte_t get_interrupt_enable_flag() const;
	void set_interrupt_enable_flag(byte_t b);
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
