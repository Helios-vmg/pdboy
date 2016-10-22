#include "HostSystem.h"
#include "DisplayController.h"
#include "Gameboy.h"
#include <iostream>
#include <iomanip>

HostSystem::HostSystem(StorageProvider *storage_provider, TimingProvider *timing_provider, GraphicsOutputProvider *graphics_provider, EventProvider *event_provider):
		storage_provider(storage_provider),
		timing_provider(timing_provider),
		graphics_provider(graphics_provider),
		event_provider(event_provider){

	if (!this->storage_provider){
		this->storage_provider = new StdStorageProvider;
		this->owned_storage_provider.reset(this->storage_provider);
	}
	this->reinit();
}

HostSystem::~HostSystem(){
	this->gameboy.reset();
}

void HostSystem::reinit(){
	this->gameboy.reset(new Gameboy(*this));
}

void HostSystem::run(){
	this->gameboy->run();
	try{
#ifdef BENCHMARKING
		auto start = SDL_GetTicks();
#endif
		while (this->handle_events()){
#ifdef BENCHMARKING
			if (SDL_GetTicks() - start >= 20000)
				break;
#endif
			this->check_exceptions();
			this->render();
		}
	}catch (std::exception &e){
		std::cerr << "Exception: " << e.what() << std::endl;
	}
}


void HostSystem::stop_and_dump_vram(){
	this->gameboy->stop_and_dump_vram("vram.bin");
}

void HostSystem::check_exceptions(){
	std::lock_guard<std::mutex> lg(this->thrown_exception_mutex);
	if (this->thrown_exception)
		throw *this->thrown_exception;
}

void HostSystem::throw_exception(const std::shared_ptr<std::exception> &ex){
	std::lock_guard<std::mutex> lg(this->thrown_exception_mutex);
	this->thrown_exception = ex;
}

void HostSystem::render(){
	if (!this->graphics_provider)
		return;
	auto frame = this->gameboy->get_current_frame();
	this->graphics_provider->render(frame);
	if (frame)
		this->gameboy->return_used_frame(frame);
}

bool HostSystem::handle_events(){
	if (!this->event_provider)
		return false;
	EventProvider::HandleEventsResult result;
	auto ret = this->event_provider->handle_events(result);
	if (result.input_state)
		this->gameboy->get_input_controller().set_input_state(result.input_state, result.button_down, result.button_up);
	return ret;
}
