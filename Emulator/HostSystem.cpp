#include "HostSystem.h"
#include "DisplayController.h"
#include "Gameboy.h"
#include "exceptions.h"
#include "timer.h"
#include <iostream>
#include <fstream>

HostSystem::HostSystem(){
	this->reinit();

}

void HostSystem::reinit(){
	this->gameboy.reset(new Gameboy);
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

SdlHostSystem::SdlHostSystem(){
	SDL_Init(SDL_INIT_VIDEO);
	this->window = SDL_CreateWindow("Gameboy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, lcd_width, lcd_height, 0);
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

void SdlHostSystem::run(){
	auto This = this;
	try{
		this->gameboy->run(
			[This](auto &dc){ This->render(dc); },
			[This](){ return This->handle_events(); }
		);
	}catch (...){
		auto quotient = ((double)this->total_frames_rendered * (double)lcd_refresh_period / (double)gb_cpu_frequency) / this->total_time_rendering;
		std::cout << "Average speed advantage over real Gameboy hardware: " << quotient << "x\n";
	}
}

void SdlHostSystem::render(DisplayController &dc){
	void *pixels;
	int pitch;

	if (SDL_LockTexture(this->main_texture, nullptr, &pixels, &pitch) < 0)
		return;
	dc.render_to((byte_t *)pixels, pitch);
	SDL_UnlockTexture(this->main_texture);
	SDL_RenderCopy(this->renderer, this->main_texture, nullptr, nullptr);

	this->total_frames_rendered++;
	auto now = get_timer_count();
	if (this->last_render != invalid_time){
		auto diff = (double)(now - this->last_render) / (double)this->realtime_counter_frequency;
		total_time_rendering += diff;
		//std::cout << "Rendering done in " << diff * 1e+6 << " us\n";
	}

	SDL_RenderPresent(this->renderer);

	this->last_render = get_timer_count();
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
	auto &controller = this->gameboy->get_input_controller();
	auto state = controller.get_input_state();
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
	controller.set_input_state(state);
	return true;
}
