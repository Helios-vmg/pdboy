#include "SoundController.h"
#include "Gameboy.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <type_traits>
#include <SDL_stdinc.h>

const float tau = (float)(M_PI * 2);

#define BASE_WAVE_SINE 1
#define BASE_WAVE_SQUARE 2
#define BASE_WAVE_TRIANGLE 3
#define BASE_WAVE BASE_WAVE_SQUARE

const unsigned sampling_frequency = 44100;
const float AudioTypeProperties<float>::max = 1;
const float AudioTypeProperties<float>::max_absolute = 1;
const float AudioTypeProperties<float>::min = -1;

intermediate_audio_type SoundController::base_wave_generator(std::uint64_t time, unsigned frequency){
	time %= sampling_frequency;
	typedef intermediate_audio_type T;
	if (std::is_floating_point<T>::value){
		T t = (T)time * ((T)1 / (T)sampling_frequency);
#if BASE_WAVE == BASE_WAVE_SINE
		return sinf(time * frequency * (tau / sampling_frequency));
#elif BASE_WAVE == BASE_WAVE_SQUARE
		return ((int)(time * frequency / (sampling_frequency / 2) % 2) * -2 + 1) * AudioTypeProperties<T>::max_absolute;
#elif BASE_WAVE == BASE_WAVE_TRIANGLE
		return cosf(sinf(time * frequency * (tau / sampling_frequency))) * (AudioTypeProperties<T>::max_absolute * 2 / M_PI);
#endif
	}
}

template <typename T>
T gcd(T a, T b){
	while (b){
		auto t = b;
		b = a % b;
		a = t;
	}
	return a;
}

SoundController::SoundController(Gameboy &system): system(&system){
	this->modulo = (std::uint64_t)sampling_frequency * (std::uint64_t)gb_cpu_frequency;
	this->modulo /= ::gcd(sampling_frequency, gb_cpu_frequency);
	this->initialize_new_frame();
}

void SoundController::update(bool required){
	auto c = this->system->get_system_clock().get_clock_value();
	c %= this->modulo;
	auto time = (c * sampling_frequency) >> 22;
	if (time == this->last_update && !required)
		return;
	auto t = this->last_update + 1;
	for (; t <= time; t++){
		typedef StereoSampleFinal::type T;
		T value = (T)(this->base_wave_generator(t, 1000) * (t * 3 % sampling_frequency / (float)sampling_frequency) * (/*0.025f **/ AudioTypeProperties<T>::max_absolute));
		auto &sample = this->publishing_frames.get_private_resource()->buffer[this->current_frame_position];
		sample.left = sample.right = value;
		this->current_frame_position++;
		if (this->current_frame_position >= AudioFrame::length){
			this->current_frame_position = 0;
			this->publishing_frames.publish();
			this->initialize_new_frame();
		}
	}
	this->last_update = time;
}

void SoundController::initialize_new_frame(){
	auto frame = this->publishing_frames.get_private_resource();
	frame->frame_no = this->frame_no++;
	auto &buffer = frame->buffer;
	memset(buffer, 0, sizeof(buffer));
}

AudioFrame *SoundController::get_current_frame(){
	return this->publishing_frames.get_public_resource();
}

void SoundController::return_used_frame(AudioFrame *frame){
	this->publishing_frames.return_resource_as_private(frame);
}
