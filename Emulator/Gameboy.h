#pragma once
#include "GameboyCpu.h"
#include "DisplayController.h"
#include "UserInputController.h"
#include "SystemClock.h"
#include "StorageController.h"
#include <functional>

class HostSystem;

enum class GameboyMode{
	DMG,
	CGB,
};

class Gameboy{
	HostSystem *host;
	GameboyCpu cpu;
	DisplayController display_controller;
	UserInputController input_controller;
	StorageController storage_controller;
	SystemClock clock;
	std::uint64_t realtime_counter_frequency = 0;
	std::uint64_t realtime_execution = 0;
	GameboyMode mode = GameboyMode::DMG;
public:
	Gameboy(HostSystem &host);

	DisplayController &get_display_controller(){
		return this->display_controller;
	}
	UserInputController &get_input_controller(){
		return this->input_controller;
	}
	StorageController &get_storage_controller(){
		return this->storage_controller;
	}
	SystemClock &get_system_clock(){
		return this->clock;
	}
	void run(
		const std::function<void(DisplayController &)> &render_callback,
		const std::function<bool()> &event_handling_callback
	);
	GameboyMode get_mode() const{
		return this->mode;
	}
};
