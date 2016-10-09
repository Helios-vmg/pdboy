#pragma once
#include "GameboyCpu.h"
#include "DisplayController.h"
#include "UserInputController.h"
#include "SystemClock.h"
#include "StorageController.h"
#include "threads.h"

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
	std::atomic<bool> continue_running;
	std::unique_ptr<std::thread> interpreter_thread;
	std::mutex interpreter_thread_mutex;
	Event periodic_notification;

	void interpreter_thread_function();
	void run_until_next_frame();
	void sync_with_real_time(std::uint64_t);
public:
	Gameboy(HostSystem &host);
	~Gameboy();

	DisplayController &get_display_controller(){
		return this->display_controller;
	}
	UserInputController &get_input_controller(){
		return this->input_controller;
	}
	StorageController &get_storage_controller(){
		return this->storage_controller;
	}
	GameboyCpu &get_cpu(){
		return this->cpu;
	}
	SystemClock &get_system_clock(){
		return this->clock;
	}
	GameboyMode get_mode() const{
		return this->mode;
	}

	void run();
	//Note: May return nullptr! In which case, no frame is currently ready, and white should be drawn.
	RenderedFrame *get_current_frame();
	void return_used_frame(RenderedFrame *);
	std::mutex &get_interpreter_thread_mutex(){
		return this->interpreter_thread_mutex;
	}
};
