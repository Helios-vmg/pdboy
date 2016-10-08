#include "HostSystem.h"
#include "DisplayController.h"
#include "Gameboy.h"
#include "exceptions.h"
#include "timer.h"
#include <iostream>
#include <fstream>
#include <cassert>

HostSystem::HostSystem(){
	this->reinit();

}

void HostSystem::reinit(){
	this->gameboy.reset(new Gameboy(*this));
}

std::unique_ptr<std::vector<byte_t>> HostSystem::load_file(const char *path, size_t maximum_size){
	std::unique_ptr<std::vector<byte_t>> ret;
	std::ifstream file(path, std::ios::binary);
	if (!file)
		return ret;
	file.seekg(0, std::ios::end);
	if ((size_t)file.tellg() > maximum_size)
		return ret;
	ret.reset(new std::vector<byte_t>(file.tellg()));
	file.seekg(0);
	file.read((char *)&(*ret)[0], ret->size());
	return ret;
}

void HostSystem::run(){
	this->gameboy->run();
	try{
		while (this->handle_events()){
			this->check_exceptions();
			this->render();
		}
	}catch (std::exception &e){
		std::cerr << "Exception: " << e.what() << std::endl;
	}
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

SdlHostSystem::SdlHostSystem(){
	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
	memset(&this->input_state, 0xFF, sizeof(this->input_state));
	this->window = SDL_CreateWindow("Gameboy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, lcd_width * 4, lcd_height * 4, 0);
	if (!this->window)
		throw GenericException("Failed to initialize SDL window.");
	this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_PRESENTVSYNC);
	if (!this->renderer)
		throw GenericException("Failed to initialize SDL renderer.");
	this->main_texture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, lcd_width, lcd_height);
	if (!this->main_texture)
		throw GenericException("Failed to create main texture.");
	this->realtime_counter_frequency = get_timer_resolution();
}

SdlHostSystem::~SdlHostSystem(){
	SdlHostSystem::unregister_periodic_notification();
	this->gameboy.reset();
	SDL_DestroyTexture(this->main_texture);
	SDL_DestroyRenderer(this->renderer);
	SDL_DestroyWindow(this->window);
	SDL_Quit();
}

void SdlHostSystem::render(){
	void *pixels;
	int pitch;

	if (SDL_LockTexture(this->main_texture, nullptr, &pixels, &pitch) < 0)
		return;
	auto current_frame = this->gameboy->get_current_frame();
	assert(pitch == lcd_width * 4);
	if (current_frame){
		memcpy(pixels, current_frame->pixels, sizeof(current_frame->pixels));
		this->gameboy->return_used_frame(current_frame);
	}else
		memset(pixels, 0xFF, sizeof(current_frame->pixels));
	SDL_UnlockTexture(this->main_texture);
	SDL_RenderCopy(this->renderer, this->main_texture, nullptr, nullptr);
	SDL_RenderPresent(this->renderer);
}

static void handle_event(InputState &state, SDL_Event &event, byte_t new_state){
	switch (event.key.keysym.sym){
		case SDLK_UP:
			state.up = new_state;
			break;
		case SDLK_DOWN:
			state.down = new_state;
			break;
		case SDLK_LEFT:
			state.left = new_state;
			break;
		case SDLK_RIGHT:
			state.right = new_state;
			break;
		case SDLK_z:
			state.a = new_state;
			break;
		case SDLK_x:
			state.b = new_state;
			break;
		case SDLK_a:
			state.start = new_state;
			break;
		case SDLK_s:
			state.select = new_state;
			break;
	}
}

bool SdlHostSystem::handle_events(){
	SDL_Event event;
	auto &state = this->input_state;
	while (SDL_PollEvent(&event)){
		switch (event.type){
			case SDL_QUIT:
				return false;
			case SDL_KEYDOWN:
				handle_event(state, event, 0x00);
				break;
			case SDL_KEYUP:
				handle_event(state, event, 0xFF);
				break;
			default:
				break;
		}
	}
	this->gameboy->get_input_controller().set_input_state(state);
	return true;
}

Uint32 SDLCALL SdlHostSystem::timer_callback(Uint32 interval, void *param){
	auto This = (SdlHostSystem *)param;
	{
		std::lock_guard<std::mutex> lg(This->periodic_event_mutex);
		if (This->periodic_event)
			This->periodic_event->signal();
	}
	return interval;
}

void SdlHostSystem::register_periodic_notification(Event &event){
	if (!this->timer_id){
		this->periodic_event = &event;
		this->timer_id = SDL_AddTimer(1, timer_callback, this);
	}else{
		std::lock_guard<std::mutex> lg(this->periodic_event_mutex);
		this->periodic_event = &event;
	}
}

void SdlHostSystem::unregister_periodic_notification(){
	if (!this->timer_id)
		return;
	SDL_RemoveTimer(this->timer_id);
	this->timer_id = 0;
	{
		std::lock_guard<std::mutex> lg(this->periodic_event_mutex);
		this->periodic_event = nullptr;
	}
}
