#include "GameboyCpu.h"
#include "timer.h"
#include "exceptions.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <iomanip>

GameboyCpu::GameboyCpu():
		registers(*this),
		memory_controller(*this),
		display_controller(*this){
}

void GameboyCpu::initialize(){
	this->initialize_opcode_tables();
	{
		std::ifstream input("tetris.gb", std::ios::binary);
		if (!input)
			throw std::exception();
		input.seekg(0, std::ios::end);
		std::vector<char> data(input.tellg());
		input.seekg(0);
		input.read(&data[0], data.size());
		this->memory_controller.load_rom_at(&data[0], data.size(), 0);
	}
	this->break_on_address = 0x100;
	this->memory_controller.toggle_boostrap_rom(true);

	this->counter_frequency = get_timer_resolution();
	auto start = get_timer_count();

	this->run();

	auto end = get_timer_count();
	double real_time = (double)(end - start) / (double)this->counter_frequency * 1e+6;
	double emulated_time = (double)this->total_cycles / (double)gb_cpu_frequency * 1e+6;

	std::cout <<
		"Real time    : " << real_time << " us\n"
		"Emulated time: " << emulated_time << " us\n";
	this->break_on_address = -1;
}

void GameboyCpu::take_time(main_integer_t cycles){
	this->total_cycles += cycles;
}

void GameboyCpu::interrupt_toggle(bool enable){
	throw NotImplementedException();
}

void GameboyCpu::stop(){
	throw NotImplementedException();
}

void GameboyCpu::halt(){
	throw NotImplementedException();
}

void GameboyCpu::abort(){
	this->running = false;
}

void GameboyCpu::run(){
	auto start = get_timer_count();
	double timer_clock_period_us = 1.0 / (this->counter_frequency * 1e-6);
	this->running = true;
	while (this->running){
		auto &pc = this->registers.pc();
		this->current_pc = pc;
		if (this->break_on_address >= 0 && pc >= this->break_on_address){
			this->running = false;
			break;
		}
		auto opcode = this->memory_controller.load8(pc);
		pc++;
		auto function_pointer = this->opcode_table[opcode];
		(this->*function_pointer)();
		this->total_instructions++;

		//std::uint64_t emulated_time = (std::uint64_t)((double)this->total_cycles * gb_cpu_clock_period_us);
		//std::uint64_t real_time;
		//do{
		//	auto now = get_timer_count();
		//	real_time = (std::uint64_t)((double)(now - start) * timer_clock_period_us);
		//}while (real_time < emulated_time);
	}
}

main_integer_t GameboyCpu::decimal_adjust(main_integer_t value){
	value &= 0xFF;
	if (!this->registers.get(Flags::Subtract)){
		if (this->registers.get(Flags::HalfCarry) || (value & 0x0F) > 9)
			value += 6;

		if (this->registers.get(Flags::Carry) || value > 0x9F)
			value += 0x60;
	} else{
		if (this->registers.get(Flags::HalfCarry))
			value = (value - 6) & 0xFF;

		if (this->registers.get(Flags::Carry))
			value -= 0x60;
	}
	return value;
}
