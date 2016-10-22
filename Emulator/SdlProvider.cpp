#include "SdlProvider.h"
#include "exceptions.h"
#include "timer.h"
#include <cassert>
#include "DisplayController.h"

#define X old = (old * 11 + current) / 12
const int lcd_fade_period = 0;
const int lcd_fade = 0xFF / lcd_fade_period;

SdlProvider::SdlProvider(){
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	this->window = SDL_CreateWindow("Gameboy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, lcd_width * 4, lcd_height * 4, 0);
	if (!this->window)
		throw GenericException("Failed to initialize SDL window.");
	this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_PRESENTVSYNC);
	if (!this->renderer)
		throw GenericException("Failed to initialize SDL renderer.");
	this->main_texture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, lcd_width, lcd_height);
	if (!this->main_texture)
		throw GenericException("Failed to create main texture.");
	this->realtime_counter_frequency = get_timer_resolution();

	{
		void *void_pixels;
		int pitch;
		if (SDL_LockTexture(this->main_texture, nullptr, &void_pixels, &pitch) >= 0){
			assert(pitch == lcd_width * 4);
			memset(void_pixels, 0xFF, RenderedFrame::bytes_size);
			SDL_UnlockTexture(this->main_texture);
		}
	}
}

SdlProvider::~SdlProvider(){
	SdlProvider::unregister_periodic_notification();
	SDL_DestroyTexture(this->main_texture);
	SDL_DestroyRenderer(this->renderer);
	SDL_DestroyWindow(this->window);
	SDL_Quit();
}

Uint32 SDLCALL SdlProvider::timer_callback(Uint32 interval, void *param){
	auto This = (SdlProvider *)param;
	{
		std::lock_guard<std::mutex> lg(This->periodic_event_mutex);
		if (This->periodic_event)
			This->periodic_event->signal();
	}
	return interval;
}

void SdlProvider::register_periodic_notification(Event &event){
	if (!this->timer_id){
		this->periodic_event = &event;
		this->timer_id = SDL_AddTimer(1, timer_callback, this);
	} else{
		std::lock_guard<std::mutex> lg(this->periodic_event_mutex);
		this->periodic_event = &event;
	}
}

void SdlProvider::unregister_periodic_notification(){
	if (!this->timer_id)
		return;
	SDL_RemoveTimer(this->timer_id);
	this->timer_id = 0;
	{
		std::lock_guard<std::mutex> lg(this->periodic_event_mutex);
		this->periodic_event = nullptr;
	}
}

void SdlProvider::render(const RenderedFrame *current_frame){
	void *void_pixels;
	int pitch;

	if (SDL_LockTexture(this->main_texture, nullptr, &void_pixels, &pitch) < 0)
		return;
	auto pixels = (byte_t *)void_pixels;
	assert(pitch == lcd_width * 4);
	if (current_frame){
		if (!lcd_fade_period)
			memcpy(pixels, current_frame->pixels, sizeof(current_frame->pixels));
		else{
			auto src = (byte_t *)current_frame->pixels;
			for (unsigned i = sizeof(current_frame->pixels); i--;){
				auto mod = i % 4;
				if (mod == 3){
					pixels[i] = 0xFF;
					continue;
				}
				int old = pixels[i];
				int current = src[i];
				if (current <= old){
					pixels[i] = current;
					continue;
				}
				old += lcd_fade;
				if (old >= current){
					pixels[i] = current;
					continue;
				}
				pixels[i] = old;
			}
		}
	}else{
		if (!lcd_fade_period)
			memset(void_pixels, 0xFF, sizeof(current_frame->pixels));
		else{
			for (unsigned i = sizeof(current_frame->pixels); i--;){
				auto mod = i % 4;
				if (mod == 3){
					pixels[i] = 0xFF;
					continue;
				}
				int old = pixels[i];
				int current = 0xFF;
				if (current <= old){
					pixels[i] = current;
					continue;
				}
				old += lcd_fade;
				if (old >= current){
					pixels[i] = current;
					continue;
				}
				pixels[i] = old;
			}
		}
	}
	SDL_UnlockTexture(this->main_texture);
	SDL_RenderCopy(this->renderer, this->main_texture, nullptr, nullptr);
	SDL_RenderPresent(this->renderer);
}

template <bool DOWN>
static void handle_event(InputState &state, SDL_Event &event, byte_t new_state, bool &flag){
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
				handle_event<true>(state, event, 0xFF, button_down);
#if 0
				{
					switch (event.key.keysym.sym){
						case SDLK_q:
							slow_mode = !slow_mode;
							break;
						case SDLK_w:
							this->stop_and_dump_vram();
							return false;
							break;
					}
				}
#endif
				break;
			case SDL_KEYUP:
				handle_event<false>(state, event, 0x00, button_up);
				break;
			default:
				break;
		}
	}
	if (button_down || button_up){
		result.input_state = new InputState(state);
		result.button_down = button_down;
		result.button_up = button_up;
	}
	return true;
}
