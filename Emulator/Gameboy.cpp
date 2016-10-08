#include "HostSystem.h"
#include "Gameboy.h"
#include "UserInputController.h"
#include "StorageController.h"
#include "timer.h"
#include <iostream>
#include "exceptions.h"

Gameboy::Gameboy(HostSystem &host):
		host(&host),
		cpu(*this),
		display_controller(*this),
		input_controller(*this),
		storage_controller(*this, host),
		clock(*this){
	this->cpu.initialize();
	this->realtime_counter_frequency = get_timer_resolution();
}

Gameboy::~Gameboy(){
	this->host->unregister_periodic_notification();
	this->continue_running = false;
	if (this->interpreter_thread){
		this->periodic_notification.signal();
		this->interpreter_thread->join();
	}
}

RenderedFrame *Gameboy::get_current_frame(){
	return this->display_controller.get_current_frame();
}

void Gameboy::return_used_frame(RenderedFrame *frame){
	this->display_controller.return_used_frame(frame);
}

void Gameboy::run(){
	this->host->register_periodic_notification(this->periodic_notification);
	this->continue_running = true;
	auto This = this;
	this->interpreter_thread.reset(new std::thread([This](){ This->interpreter_thread_function(); }));
}

void Gameboy::interpreter_thread_function(){
	auto start = get_timer_count();

	try{
		while (this->continue_running){
			this->run_until_next_frame();
			this->sync_with_real_time(start);
		}
	}catch (GameBoyException &ex){
		this->host->throw_exception(std::shared_ptr<std::exception>(ex.clone()));
	}catch (...){
		this->host->throw_exception(std::make_shared<GenericException>("Unknown exception."));
	}

	/*
	auto end = get_timer_count();
	double real_time = (double)(end - start) / (double)this->realtime_counter_frequency * 1e+6;
	double emulated_time = (double)this->clock.get_clock_value() / (double)gb_cpu_frequency * 1e+6;

	std::cout <<
		"Real time    : " << real_time << " us\n"
		"Emulated time: " << emulated_time << " us\n";
	*/
}

void Gameboy::run_until_next_frame(){
	while (this->continue_running){
		if (this->display_controller.in_new_frame())
			break;
		this->cpu.run_one_instruction();
		if (this->display_controller.ready_to_draw()){
			this->cpu.vblank_irq();
		}
	}
}

void Gameboy::sync_with_real_time(std::uint64_t real_time_start){
	double emulated_time = (double)this->clock.get_clock_value() / (double)gb_cpu_frequency;
	double rt_multiplier = 1.0 / (double)this->realtime_counter_frequency;
	while (true){
		auto now = get_timer_count();
		double real_time = (double)(now - real_time_start) * rt_multiplier;
		if (real_time >= emulated_time)
			break;
		this->periodic_notification.reset_and_wait();
	}
}
