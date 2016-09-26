#pragma once
#include "Gameboy.h"
#include <SDL.h>

class DisplayController;

class HostSystem{
	Gameboy gameboy;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *main_texture;

	void render(DisplayController &dc);
public:
	HostSystem();
	void run();
	void reinit();
};
