#pragma once
#include "GameboyCpu.h"
#include "DisplayController.h"
#include "UserInputController.h"
#include <functional>

class Gameboy{
	GameboyCpu cpu;
	DisplayController display_controller;
	UserInputController input_controller;
	std::uint64_t clock = 0;
	std::uint64_t realtime_counter_frequency = 0;
	std::uint64_t realtime_execution = 0;
public:
	Gameboy();

	std::uint64_t get_clock_value() const{
		return this->clock;
	}
	void advance_clock(std::uint64_t clocks){
		this->clock += clocks;
	}
	DisplayController &get_display_controller(){
		return this->display_controller;
	}
	UserInputController &get_input_controller(){
		return this->input_controller;
	}
	void run(
		const std::function<void(DisplayController &)> &render_callback,
		const std::function<bool()> &event_handling_callback
	);

};
