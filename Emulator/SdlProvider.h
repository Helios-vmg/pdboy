#pragma once

#include "HostSystemServiceProviders.h"
#include "UserInputController.h"
#include <SDL.h>

class SdlProvider : public EventProvider, public TimingProvider, public GraphicsOutputProvider{
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *main_texture;
	SDL_TimerID timer_id = 0;
	static const std::uint64_t invalid_time = std::numeric_limits<std::uint64_t>::max();
	std::uint64_t realtime_counter_frequency = 0;
	InputState input_state;
	std::mutex periodic_event_mutex;

	static Uint32 SDLCALL timer_callback(Uint32 interval, void *param);
public:
	SdlProvider();
	~SdlProvider();
	void register_periodic_notification(Event &) override;
	void unregister_periodic_notification() override;
	void render(const RenderedFrame *) override;
	bool handle_events(HandleEventsResult &) override;
};
