#pragma once
#include "Gameboy.h"
#include <SDL.h>
#include <memory>
#include <limits>

class DisplayController;
struct InputState;

class HostSystem{
	std::unique_ptr<Gameboy> gameboy;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *main_texture;
	static const std::uint64_t invalid_time = std::numeric_limits<std::uint64_t>::max();
	std::uint64_t realtime_counter_frequency = 0;
	std::uint64_t last_render = invalid_time;
	double total_time_rendering = 0;
	std::uint64_t total_frames_rendered = 0;

	void render(DisplayController &dc);
	bool handle_events();
public:
	HostSystem();
	void run();
	void reinit();
};
