#include "Gameboy.h"
#include "UserInputController.h"
#include "timer.h"
#include <iostream>
#include "exceptions.h"

Gameboy::Gameboy():
		cpu(*this),
		display_controller(*this),
		input_controller(*this){
	this->cpu.initialize();
	this->realtime_counter_frequency = get_timer_resolution();
}

void Gameboy::run(
		const std::function<void(DisplayController &)> &render_callback,
		const std::function<bool()> &event_handling_callback){
	auto start = get_timer_count();

	while (true){
		if (!event_handling_callback())
			break;

		this->cpu.run_one_instruction();

		if (this->display_controller.ready_to_draw()){
			this->cpu.vblank_irq();
			render_callback(this->display_controller);
			//	auto now = get_timer_count();
			//	double real_time = (double)(now - start) / (double)this->realtime_counter_frequency * 1e+3;
			//	double emulated_time = (double)this->get_clock_value() / (double)gb_cpu_frequency * 1e+3;
			//	std::cout << "Time offset: " << emulated_time - real_time << " ms\n";
		}
	}

	auto end = get_timer_count();
	double real_time = (double)(end - start) / (double)this->realtime_counter_frequency * 1e+6;
	double emulated_time = (double)this->get_clock_value() / (double)gb_cpu_frequency * 1e+6;

	std::cout <<
		"Real time    : " << real_time << " us\n"
		"Emulated time: " << emulated_time << " us\n";
}
