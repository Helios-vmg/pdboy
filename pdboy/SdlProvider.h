#pragma once

#include "HostSystemServiceProviders.h"
#include <SDL.h>
#include <mutex>

class SdlProvider : public EventProvider, public TimingProvider, public GraphicsOutputProvider, public AudioOutputProvider{
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *main_texture;
	SDL_TimerID timer_id = 0;
	SDL_AudioDeviceID audio_device = 0;
	static const std::uint64_t invalid_time = std::numeric_limits<std::uint64_t>::max();
	std::uint64_t realtime_counter_frequency = 0;
	libpdboy_inputstate input_state;
	std::mutex periodic_event_mutex;
	std::uint64_t next_frame = 0;

	static Uint32 SDLCALL timer_callback(Uint32 interval, void *param);
	static void SDLCALL audio_callback(void *userdata, Uint8 *stream, int len);
	void initialize_graphics();
	void initialize_audio();
	void stop_audio() override;
public:
	SdlProvider();
	~SdlProvider();
	void register_periodic_notification(const std::function<void()> &) override;
	void unregister_periodic_notification() override;
	typedef std::unique_ptr<libpdboy_rgba, std::function<void(libpdboy_rgba *)>> render_t;
	render_t render() override;
	bool handle_events(HandleEventsResult &) override;
	//void write_frame_to_disk(std::string &path, const RenderedFrame &) override;
};

