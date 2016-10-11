#include "HostSystem.h"
#include "DisplayController.h"
#include "Gameboy.h"
#include "exceptions.h"
#include "timer.h"
#include <iostream>
#include <fstream>
#include <cassert>

std::atomic<bool> slow_mode(false);
const int lcd_fade_period = 0;
const int lcd_fade = 0xFF / lcd_fade_period;

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

	{
		void *void_pixels;
		int pitch;
		if (SDL_LockTexture(this->main_texture, nullptr, &void_pixels, &pitch) >= 0){
			assert(pitch == lcd_width * 4);
			memset(void_pixels, 0xFF, sizeof(this->gameboy->get_current_frame()->pixels));
			SDL_UnlockTexture(this->main_texture);
		}
	}
}

SdlHostSystem::~SdlHostSystem(){
	SdlHostSystem::unregister_periodic_notification();
	this->gameboy.reset();
	SDL_DestroyTexture(this->main_texture);
	SDL_DestroyRenderer(this->renderer);
	SDL_DestroyWindow(this->window);
	SDL_Quit();
}

#define X old = (old * 11 + current) / 12

void SdlHostSystem::render(){
	void *void_pixels;
	int pitch;

	if (SDL_LockTexture(this->main_texture, nullptr, &void_pixels, &pitch) < 0)
		return;
	auto pixels = (byte_t *)void_pixels;
	auto current_frame = this->gameboy->get_current_frame();
	assert(pitch == lcd_width * 4);
	if (current_frame){
		if (!lcd_fade_period)
			memcpy(pixels, current_frame->pixels, sizeof(current_frame->pixels));
		else{
			auto src = (byte_t *)current_frame->pixels;
			for (unsigned i = sizeof(current_frame->pixels); i--;){
				auto mod = i % 4;
				if (mod == 3){
					pixels[i] = 0xFF;
					continue;
				}
				int old = pixels[i];
				int current = src[i];
				if (current <= old){
					pixels[i] = current;
					continue;
				}
				old += lcd_fade;
				if (old >= current){
					pixels[i] = current;
					continue;
				}
				pixels[i] = old;
			}
		}
		this->gameboy->return_used_frame(current_frame);
	} else{
		if (!lcd_fade_period)
			memset(void_pixels, 0xFF, sizeof(current_frame->pixels));
		else{
			for (unsigned i = sizeof(current_frame->pixels); i--;){
				auto mod = i % 4;
				if (mod == 3){
					pixels[i] = 0xFF;
					continue;
				}
				int old = pixels[i];
				int current = 0xFF;
				if (current <= old){
					pixels[i] = current;
					continue;
				}
				old += lcd_fade;
				if (old >= current){
					pixels[i] = current;
					continue;
				}
				pixels[i] = old;
			}
		}
	}
	SDL_UnlockTexture(this->main_texture);
	SDL_RenderCopy(this->renderer, this->main_texture, nullptr, nullptr);
	SDL_RenderPresent(this->renderer);
}

template <bool DOWN>
static void handle_event(InputState &state, SDL_Event &event, byte_t new_state, bool &flag){
	switch (event.key.keysym.sym){
		case SDLK_UP:
			flag = true;
			state.up = new_state;
			break;
		case SDLK_DOWN:
			flag = true;
			state.down = new_state;
			break;
		case SDLK_LEFT:
			flag = true;
			state.left = new_state;
			break;
		case SDLK_RIGHT:
			flag = true;
			state.right = new_state;
			break;
		case SDLK_z:
			flag = true;
			state.a = new_state;
			break;
		case SDLK_x:
			flag = true;
			state.b = new_state;
			break;
		case SDLK_a:
			flag = true;
			state.start = new_state;
			break;
		case SDLK_s:
			flag = true;
			state.select = new_state;
			break;
		case SDLK_q:
			if (DOWN)
				slow_mode = !slow_mode;
			break;
	}
}

bool SdlHostSystem::handle_events(){
	SDL_Event event;
	auto &state = this->input_state;
	bool button_down = false;
	bool button_up = false;
	while (SDL_PollEvent(&event)){
		switch (event.type){
			case SDL_QUIT:
				return false;
			case SDL_KEYDOWN:
				handle_event<true>(state, event, 0x00, button_down);
				break;
			case SDL_KEYUP:
				handle_event<false>(state, event, 0xFF, button_up);
				break;
			default:
				break;
		}
	}
	if (button_down || button_up)
		this->gameboy->get_input_controller().set_input_state(new InputState(state), button_down, button_up);
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
