#include "SoundController.h"
#include "Gameboy.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <type_traits>
#include <SDL_stdinc.h>

const float tau = (float)(M_PI * 2);

#define BASE_WAVE_SINE 1
#define BASE_WAVE_SQUARE 2
#define BASE_WAVE BASE_WAVE_SINE

const unsigned sampling_frequency = 44100;
const float AudioTypeProperties<float>::max = 1;
const float AudioTypeProperties<float>::max_absolute = 1;
const float AudioTypeProperties<float>::min = -1;
const byte_t Square2Generator::duties[4] = {
	0x80,
	0x81,
	0xE1,
	0x7E,
};

basic_StereoSample<std::int16_t> convert(const basic_StereoSample<float> &src){
	basic_StereoSample<std::int16_t> ret;
	ret.left = (std::int16_t)(src.left * AudioTypeProperties<std::int16_t>::max_absolute);
	ret.right = (std::int16_t)(src.right * AudioTypeProperties<std::int16_t>::max_absolute);
	return ret;
}

intermediate_audio_type SoundController::base_wave_generator_duty_50(std::uint64_t time, unsigned frequency){
	time %= sampling_frequency;
	typedef intermediate_audio_type T;
	if (std::is_floating_point<T>::value){
		T t = (T)time * ((T)1 / (T)sampling_frequency);
#if BASE_WAVE == BASE_WAVE_SINE
		return sinf(time * frequency * (tau / sampling_frequency));
#elif BASE_WAVE == BASE_WAVE_SQUARE
		return ((int)(time * frequency / (sampling_frequency / 2) % 2) * -2 + 1) * AudioTypeProperties<T>::max_absolute;
#endif
	}
}

float sine_variable_duty(std::uint64_t time, unsigned frequency, float duty){
	auto y = SoundController::base_wave_generator_duty_50(time, frequency);
	y = (y + 1) * 0.5f;
	y = pow(y, duty);
	y = y * 2 - 1;
	return y;
}

intermediate_audio_type SoundController::base_wave_generator_duty_12(std::uint64_t time, unsigned frequency){
	time %= sampling_frequency;
	typedef intermediate_audio_type T;
	if (std::is_floating_point<T>::value){
		T t = (T)time * ((T)1 / (T)sampling_frequency);
#if BASE_WAVE == BASE_WAVE_SINE
		return sine_variable_duty(time, frequency, 17.8f);
#elif BASE_WAVE == BASE_WAVE_SQUARE
		return ((int)(time * frequency * 2 / (sampling_frequency / 4) % 8 == 0) * -2 + 1) * AudioTypeProperties<T>::max_absolute;
#endif
	}
}

intermediate_audio_type SoundController::base_wave_generator_duty_25(std::uint64_t time, unsigned frequency){
	time %= sampling_frequency;
	typedef intermediate_audio_type T;
	if (std::is_floating_point<T>::value){
		T t = (T)time * ((T)1 / (T)sampling_frequency);
#if BASE_WAVE == BASE_WAVE_SINE
		return sine_variable_duty(time, frequency, 4.38f);
#elif BASE_WAVE == BASE_WAVE_SQUARE
		return -((int)(time * frequency * 2 / (sampling_frequency / 4) % 8 != 0) * -2 + 1) * AudioTypeProperties<T>::max_absolute;
#endif
	}
}

intermediate_audio_type SoundController::base_wave_generator_duty_75(std::uint64_t time, unsigned frequency){
	time %= sampling_frequency;
	typedef intermediate_audio_type T;
	if (std::is_floating_point<T>::value){
		T t = (T)time * ((T)1 / (T)sampling_frequency);
#if BASE_WAVE == BASE_WAVE_SINE
		return -sine_variable_duty(time, frequency, 4.38f);
#elif BASE_WAVE == BASE_WAVE_SQUARE
		return ((int)(time * frequency * 2 / (sampling_frequency / 4) % 8 == 0) * -2 + 1) * AudioTypeProperties<T>::max_absolute;
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
		typedef StereoSampleIntermediate::type T1;
		typedef StereoSampleFinal::type T2;
		StereoSampleIntermediate sample;
		sample.left = sample.right = 0;
		sample += this->render_square1(t) / 4;
		sample += this->render_square2(t) / 4;
		sample += this->render_voluntary(t) / 4;
		sample += this->render_noise(t) / 4;

		auto &dst = this->publishing_frames.get_private_resource()->buffer[this->current_frame_position];
		dst = convert(sample);

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
	this->publishing_frames.return_resource(frame);
}

StereoSampleIntermediate SoundController::render_square1(std::uint64_t time){
	StereoSampleIntermediate ret;
	ret.left = ret.right = this->square1.render(time);
	return ret;
}

StereoSampleIntermediate SoundController::render_square2(std::uint64_t time){
	StereoSampleIntermediate ret;
	ret.left = ret.right = 0;
	return ret;
}

StereoSampleIntermediate SoundController::render_voluntary(std::uint64_t time){
	StereoSampleIntermediate ret;
	ret.left = ret.right = 0;
	return ret;
}

StereoSampleIntermediate SoundController::render_noise(std::uint64_t time){
	StereoSampleIntermediate ret;
	ret.left = ret.right = 0;
	return ret;
}

unsigned Square2Generator::get_period(){
	if (!this->period)
		this->period = (2048 - this->frequency) * 4;
	return this->period;
}

void Square2Generator::advance_duty(std::uint64_t time){
	if (this->reference_time == std::numeric_limits<decltype(this->reference_time)>::max() & this->reference_duty_position == std::numeric_limits<decltype(this->duty_position)>::max()){
		this->reference_time = time;
		this->reference_duty_position = this->duty_position;
	}else{
		auto delta = time - this->reference_time;
		this->duty_position = this->reference_duty_position;
		this->duty_position += (delta << 13) * gb_cpu_frequency / (sampling_frequency * this->get_period());
		this->duty_position &= 0xFFFF;
	}
}

intermediate_audio_type Square2Generator::render(std::uint64_t time){
	this->advance_duty(time);
	return !(this->duties[this->selected_duty] & bit(this->duty_position >> 13)) * 2 - 1;
	//return SoundController::base_wave_generator_duty_50(time, this->get_period()) * 0.5f;
}

void Oscillator::trigger_event(){
	throw NotImplementedException();
}

void Square2Generator::set_register1(byte_t value){
	this->registers[1] = value;
}

void Square2Generator::set_register2(byte_t value){
	this->registers[2] = value;
}

void Square2Generator::set_register3(byte_t value){
	this->registers[3] = value;
	this->frequency &= ~(decltype(this->frequency))0xFF;
	this->frequency |= value;
	this->period = 0;
	this->reference_time = std::numeric_limits<decltype(this->reference_time)>::max();
	this->reference_duty_position = std::numeric_limits<decltype(this->duty_position)>::max();
}

void Square2Generator::set_register4(byte_t value){
	this->registers[4] = value;

	bool trigger = !!(value & bit(7));
	bool length_enable = !!(value & bit(6));

	//if (trigger)
	//	this->trigger_event();

	unsigned freq = value & 0x07;
	freq <<= 8;
	this->frequency &= ~(decltype(this->frequency))0x0700;
	this->frequency |= freq;
	this->period = 0;
	this->reference_time = std::numeric_limits<decltype(this->reference_time)>::max();
	this->reference_duty_position = std::numeric_limits<decltype(this->duty_position)>::max();
}

byte_t Square2Generator::get_register1() const{
	return this->registers[1];
}

byte_t Square2Generator::get_register2() const{
	return this->registers[2];
}

byte_t Square2Generator::get_register3() const{
	return 0xFF;
}

byte_t Square2Generator::get_register4() const{
	return this->registers[4] | 0x07;
}

void Square1Generator::set_register0(byte_t value){
	this->registers[0] = value;
}

byte_t Square1Generator::get_register0() const{
	return this->registers[0];
}
