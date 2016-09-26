#include "HostSystem.h"
#include "DisplayController.h"
#include "Gameboy.h"
#include "exceptions.h"

HostSystem::HostSystem(){
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
}

void HostSystem::run(){
	auto This = this;
	this->gameboy.run([This](auto &dc){ This->render(dc); });
}

void HostSystem::render(DisplayController &dc){
	void *pixels;
	int pitch;
	if (SDL_LockTexture(this->main_texture, nullptr, &pixels, &pitch) < 0)
		return;
	dc.render_to((byte_t *)pixels, pitch);
	SDL_UnlockTexture(this->main_texture);
	SDL_RenderCopy(this->renderer, this->main_texture, nullptr, nullptr);
	SDL_RenderPresent(this->renderer);
}

void HostSystem::reinit(){
	this->gameboy = Gameboy();
}
