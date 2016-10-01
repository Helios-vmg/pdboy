#pragma once
#include "Gameboy.h"
#include <SDL.h>
#include <memory>
#include <limits>

class DisplayController;
struct InputState;

class HostSystem{
protected:
	std::unique_ptr<Gameboy> gameboy;
public:
	HostSystem();
	virtual ~HostSystem(){}
	virtual std::unique_ptr<std::vector<byte_t>> load_file(const char *path, size_t maximum_size);
	void reinit();
};

class SdlHostSystem : public HostSystem{
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
	SdlHostSystem();
	void run();
};
