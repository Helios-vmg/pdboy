#include "HostSystem.h"
#include "Gameboy.h"
#include "UserInputController.h"
#include "StorageController.h"
#include "timer.h"
#include "exceptions.h"
#include <iostream>
#include <sstream>

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
	std::uint64_t time_running = 0;
	std::uint64_t time_waiting = 0;

	std::shared_ptr<std::exception> thrown;
	try{
		while (this->continue_running){
			auto t0 = get_timer_count();
			this->run_until_next_frame();
			auto t1 = get_timer_count();
			this->sync_with_real_time(start);
			auto t2 = get_timer_count();
			time_running += t1 - t0;
			time_waiting += t2 - t1;
		}
	}catch (GameBoyException &ex){
		thrown.reset(ex.clone());
	}catch (...){
		thrown.reset(new GenericException("Unknown exception."));
	}

	std::cout
		<< "Time spent running: " << (double)time_running / (double)this->realtime_counter_frequency << " s.\n"
		<< "Time spent waiting: " << (double)time_waiting / (double)this->realtime_counter_frequency << " s.\n"
		<< "CPU usage:          " << (double)time_running / (double)(time_running + time_waiting) * 100 << " %\n"
		<< "Speed:              " << (double)(time_running + time_waiting) / (double)time_running << "x\n";

	this->host->throw_exception(thrown);
}

void Gameboy::run_until_next_frame(){
	while (this->continue_running){
		{
			automutex_t am(this->interpreter_thread_mutex);
			this->cpu.run_one_instruction();
		}
		if (this->display_controller.update())
			break;
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
		this->periodic_notification.reset_and_wait_for(250);
	}
}
