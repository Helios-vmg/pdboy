#pragma once
#include "Gameboy.h"
#include "threads.h"
#include "exceptions.h"
#include <SDL.h>
#include <memory>
#include <limits>

class DisplayController;
struct InputState;

class HostSystem{
protected:
	std::unique_ptr<Gameboy> gameboy;
	Event *periodic_event = nullptr;
	std::mutex periodic_event_mutex;
	std::shared_ptr<std::exception> thrown_exception;
	std::mutex thrown_exception_mutex;

	void check_exceptions();
	virtual void render() = 0;
	virtual bool handle_events() = 0;
public:
	HostSystem();
	virtual ~HostSystem(){}
	virtual std::unique_ptr<std::vector<byte_t>> load_file(const char *path, size_t maximum_size);
	void reinit();
	Gameboy &get_guest(){
		return *this->gameboy;
	}
	virtual void register_periodic_notification(Event &) = 0;
	virtual void unregister_periodic_notification() = 0;
	void throw_exception(const std::shared_ptr<std::exception> &);
	void run();
};

class SdlHostSystem : public HostSystem{
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *main_texture;
	SDL_TimerID timer_id = 0;
	static const std::uint64_t invalid_time = std::numeric_limits<std::uint64_t>::max();
	std::uint64_t realtime_counter_frequency = 0;
	InputState input_state;

	void render() override;
	bool handle_events() override;
	static Uint32 SDLCALL timer_callback(Uint32 interval, void *param);
public:
	SdlHostSystem();
	~SdlHostSystem();
	void register_periodic_notification(Event &) override;
	void unregister_periodic_notification() override;
};
