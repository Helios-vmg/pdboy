#include "Gameboy.h"
#include "timer.h"
#include <iostream>
#include "exceptions.h"

Gameboy::Gameboy():
		cpu(*this),
		display_controller(*this){
	this->cpu.initialize();
	this->realtime_counter_frequency = get_timer_resolution();
}

void Gameboy::run(const std::function<void(DisplayController &)> &render_callback){
	auto start = get_timer_count();

	while (true){
		this->cpu.run_one_instruction();
		if (this->display_controller.ready_to_draw()){
			render_callback(this->display_controller);
		}
	}

	auto end = get_timer_count();
	double real_time = (double)(end - start) / (double)this->realtime_counter_frequency * 1e+6;
	double emulated_time = (double)this->get_clock_value() / (double)gb_cpu_frequency * 1e+6;

	std::cout <<
		"Real time    : " << real_time << " us\n"
		"Emulated time: " << emulated_time << " us\n";
}

void Gameboy::interrupt_toggle(bool enable){
	throw NotImplementedException();
}
