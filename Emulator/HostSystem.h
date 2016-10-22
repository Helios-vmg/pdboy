#pragma once
#include "Gameboy.h"
#include "HostSystemServiceProviders.h"
#include "threads.h"
#include "exceptions.h"
#include "point.h"
#include "utility.h"
#include <memory>
#include <limits>

//#define BENCHMARKING

class DisplayController;
class Cartridge;
struct InputState;

class HostSystem{
protected:
	std::unique_ptr<Gameboy> gameboy;
	StorageProvider *storage_provider;
	std::unique_ptr<StorageProvider> owned_storage_provider;
	TimingProvider *timing_provider;
	GraphicsOutputProvider *graphics_provider;
	EventProvider *event_provider;
	std::shared_ptr<std::exception> thrown_exception;
	std::mutex thrown_exception_mutex;

	void check_exceptions();
	void render();
	bool handle_events();
public:
	HostSystem(StorageProvider *, TimingProvider *, GraphicsOutputProvider *, EventProvider *);
	~HostSystem();
	void reinit();
	Gameboy &get_guest(){
		return *this->gameboy;
	}
	void throw_exception(const std::shared_ptr<std::exception> &);
	void run();
	void stop_and_dump_vram();
	StorageProvider *get_storage_provider() const{
		return this->storage_provider;
	}
	TimingProvider *get_timing_provider() const{
		return this->timing_provider;
	}
	GraphicsOutputProvider *get_graphics_provider() const{
		return this->graphics_provider;
	}
	EventProvider *get_event_provider() const{
		return this->event_provider;
	}
	void save_ram(Cartridge &, const std::vector<byte_t> &ram);
	std::unique_ptr<std::vector<byte_t>> load_ram(Cartridge &, size_t expected_size);
};
