#include "HostSystem.h"

HostSystem::HostSystem(GraphicsOutputProvider &gop, AudioOutputProvider &aop, EventProvider &ep, TimingProvider &tp): libpdboy(nullptr, libpdboy_destroy){
	this->graphics_provider = &gop;
	this->audio_provider = &aop;
	this->event_provider = &ep;
	this->timing_provider = &tp;
	this->libpdboy.reset(libpdboy_create(this));
	if (!this->libpdboy)
		throw std::runtime_error("Failed to initialize libpdboy.");
	libpdboy_set_timing_provider(
		this->libpdboy.get(),
		[](auto ud, auto cb, auto pd){ return ((HostSystem *)ud)->register_periodic_notification(cb, pd); },
		[](auto ud){ return ((HostSystem *)ud)->unregister_periodic_notification(); }
	);
	this->audio_provider->set_callback([this](void *dst, size_t size){ libpdboy_get_audio_data(this->libpdboy.get(), (libpdboy_audiosample *)dst, size / sizeof(libpdboy_audiosample)); });
}

HostSystem::~HostSystem(){
	this->audio_provider->stop_audio();
	this->libpdboy.reset();
}

int HostSystem::register_periodic_notification(libpdboy_periodic_notification_callback_f callback, void *private_data){
	this->timing_provider->register_periodic_notification([callback, private_data](){ callback(private_data); });
	return 0;
}

void HostSystem::unregister_periodic_notification(){
	this->timing_provider->unregister_periodic_notification();
}

void HostSystem::run(const char *path){
	libpdboy_load(this->libpdboy.get(), path);
	while (this->handle_events()){
		this->check_exceptions();
		this->render();
	}
}

void HostSystem::check_exceptions(){
	auto msg = libpdboy_get_exception_message(this->libpdboy.get());
	if (msg)
		throw std::runtime_error(msg);
}

bool HostSystem::handle_events(){
	EventProvider::HandleEventsResult result;
	auto ret = this->event_provider->handle_events(result);
	if (result.input_state)
		libpdboy_set_input_state(this->libpdboy.get(), result.input_state, result.button_down, result.button_up);
	return ret;
}

void HostSystem::render(){
	auto frame = this->graphics_provider->render();
	libpdboy_get_current_frame(this->libpdboy.get(), frame.get());
}
