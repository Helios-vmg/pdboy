#include "SdlProvider.h"
#include "exceptions.h"
#include "timer.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <mutex>

#define X old = (old * 11 + current) / 12
const int lcd_fade_period = 0;
const int lcd_fade = lcd_fade_period ? 0xFF / lcd_fade_period : 0;

SdlProvider::SdlProvider(){
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
	this->initialize_graphics();
	this->initialize_audio();
}

SdlProvider::~SdlProvider(){
	SdlProvider::unregister_periodic_notification();
	SDL_DestroyTexture(this->main_texture);
	SDL_DestroyRenderer(this->renderer);
	SDL_DestroyWindow(this->window);
	SDL_Quit();
}

void SdlProvider::initialize_graphics(){
	this->window = SDL_CreateWindow("Gameboy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, libpdboy_screen_width * 4, libpdboy_screen_height * 4, 0);
	if (!this->window)
		throw GenericException("Failed to initialize SDL window.");
	this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_PRESENTVSYNC);
	if (!this->renderer)
		throw GenericException("Failed to initialize SDL renderer.");
	this->main_texture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, libpdboy_screen_width, libpdboy_screen_height);
	if (!this->main_texture)
		throw GenericException("Failed to create main texture.");
	this->realtime_counter_frequency = get_timer_resolution();

	{
		void *void_pixels;
		int pitch;
		if (SDL_LockTexture(this->main_texture, nullptr, &void_pixels, &pitch) >= 0){
			assert(pitch == libpdboy_screen_width * 4);
			memset(void_pixels, 0xFF, libpdboy_screen_size * sizeof(libpdboy_rgba));
			SDL_UnlockTexture(this->main_texture);
		}
	}
}

bool operator==(const SDL_AudioSpec &a, const SDL_AudioSpec &b){
	return
		a.freq == b.freq &&
		a.format == b.format &&
		a.channels == b.channels &&
		a.samples == b.samples;
}

void SdlProvider::initialize_audio(){
	SDL_AudioSpec desired, actual;
	memset(&desired, 0, sizeof(desired));
	desired.freq = 44100;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = 1024;
	desired.callback = SdlProvider::audio_callback;
	desired.userdata = this;
	this->audio_device = SDL_OpenAudioDevice(nullptr, false, &desired, &actual, 0);
	if (!(actual == desired)){
		SDL_CloseAudioDevice(this->audio_device);
		this->audio_device = 0;
		return;
	}
	SDL_PauseAudioDevice(this->audio_device, 0);
}

Uint32 SDLCALL SdlProvider::timer_callback(Uint32 interval, void *param){
	auto This = (SdlProvider *)param;
	{
		std::lock_guard<std::mutex> lg(This->periodic_event_mutex);
		This->event_callback();
	}
	return interval;
}

void SDLCALL SdlProvider::audio_callback(void *userdata, Uint8 *stream, int len){
	auto This = (SdlProvider *)userdata;
	std::lock_guard<std::mutex> lg(This->mutex);
	if (This->get_audio_data_callback)
		This->get_audio_data_callback(stream, len);
}

void SdlProvider::register_periodic_notification(const std::function<void()> &callback){
	if (!this->timer_id){
		this->event_callback = callback;
		this->timer_id = SDL_AddTimer(1, timer_callback, this);
	}else{
		std::lock_guard<std::mutex> lg(this->periodic_event_mutex);
		this->event_callback = callback;
	}
}

void SdlProvider::unregister_periodic_notification(){
	if (!this->timer_id)
		return;
	SDL_RemoveTimer(this->timer_id);
	this->timer_id = 0;
	{
		std::lock_guard<std::mutex> lg(this->periodic_event_mutex);
		this->event_callback = decltype(this->event_callback)();
	}
}

SdlProvider::render_t SdlProvider::render(){
	void *void_pixels;
	int pitch;

	if (SDL_LockTexture(this->main_texture, nullptr, &void_pixels, &pitch) < 0)
		return render_t();
	return render_t(
		(libpdboy_rgba *)void_pixels,
		[this](libpdboy_rgba *){
			SDL_UnlockTexture(this->main_texture);
			SDL_RenderCopy(this->renderer, this->main_texture, nullptr, nullptr);
			SDL_RenderPresent(this->renderer);
		}
	);
}

template <bool DOWN>
static void handle_event(libpdboy_inputstate &state, SDL_Event &event, byte_t new_state, bool &flag){
	switch (event.key.keysym.sym){
		case SDLK_UP:
			flag = true;
			state.up = new_state;
			break;
		case SDLK_DOWN:
			flag = true;
			state.down = new_state;
			break;
		case SDLK_LEFT:
			flag = true;
			state.left = new_state;
			break;
		case SDLK_RIGHT:
			flag = true;
			state.right = new_state;
			break;
		case SDLK_z:
			flag = true;
			state.a = new_state;
			break;
		case SDLK_x:
			flag = true;
			state.b = new_state;
			break;
		case SDLK_a:
			flag = true;
			state.start = new_state;
			break;
		case SDLK_s:
			flag = true;
			state.select = new_state;
			break;
	}
}

bool SdlProvider::handle_events(HandleEventsResult &result){
	SDL_Event event;
	auto &state = this->input_state;
	bool button_down = false;
	bool button_up = false;
	while (SDL_PollEvent(&event)){
		switch (event.type){
			case SDL_QUIT:
				return false;
			case SDL_KEYDOWN:
				if (!event.key.repeat){
					handle_event<true>(state, event, 0xFF, button_down);
					{
						switch (event.key.keysym.sym){
							case SDLK_p:
								result.pause_toggled = true;
								break;
							case SDLK_SPACE:
								result.fast_mode = 1;
								break;
							case SDLK_LCTRL:
								result.slow_mode = 1;
								break;
						}
					}
				}
				break;
			case SDL_KEYUP:
				if (!event.key.repeat){
					handle_event<false>(state, event, 0x00, button_up);
					{
						switch (event.key.keysym.sym){
							case SDLK_SPACE:
								result.fast_mode = 0;
								break;
							case SDLK_LCTRL:
								result.slow_mode = 0;
								break;
						}
					}
				}
				break;
			default:
				break;
		}
	}
	if (button_down || button_up){
		result.input_state = new libpdboy_inputstate(state);
		result.button_down = button_down;
		result.button_up = button_up;
	}
	return true;
}

//void SdlProvider::write_frame_to_disk(std::string &path, const RenderedFrame &frame){
//	auto surface = SDL_CreateRGBSurface(0, libpdboy_screen_width, libpdboy_screen_height, 32, 0xFF, 0xFF00, 0xFF0000, 0xFF000000);
//	SDL_LockSurface(surface);
//	memcpy(surface->pixels, frame.pixels, sizeof(frame.pixels));
//	SDL_UnlockSurface(surface);
//	SDL_SaveBMP(surface, path.c_str());
//	SDL_FreeSurface(surface);
//}

void SdlProvider::stop_audio(){
	if (!this->audio_device)
		return;
	SDL_PauseAudioDevice(this->audio_device, 1);
}
