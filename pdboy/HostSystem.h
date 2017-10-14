#pragma once
#include "HostSystemServiceProviders.h"
#include <libpdboy.h>

class HostSystem{
	std::unique_ptr<libpdboy_state, void (*)(libpdboy_state *)> libpdboy;
	GraphicsOutputProvider *graphics_provider;
	AudioOutputProvider *audio_provider;
	EventProvider *event_provider;
	TimingProvider *timing_provider;

	int register_periodic_notification(libpdboy_periodic_notification_callback_f callback, void *private_data);
	void unregister_periodic_notification();
	void check_exceptions();
	bool handle_events();
	void render();
public:
	HostSystem(GraphicsOutputProvider &gop, AudioOutputProvider &aop, EventProvider &ep, TimingProvider &tp);
	~HostSystem();
	void reinit();
	void run(const char *path);
};
