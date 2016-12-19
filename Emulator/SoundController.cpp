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

template <typename T>
T gcd(T a, T b){
	while (b){
		auto t = b;
		b = a % b;
		a = t;
	}
	return a;
}

ClockDivider::ClockDivider(std::uint64_t src_frequency, std::uint64_t dst_frequency, callback_t &&callback):
		callback(callback),
		src_frequency(src_frequency),
		dst_frequency(dst_frequency){
	this->modulo = this->dst_frequency * this->src_frequency;
	this->modulo /= ::gcd(this->dst_frequency, this->src_frequency);
}

void ClockDivider::update(std::uint64_t source_clock){
	source_clock %= this->modulo;
	auto time = source_clock * this->dst_frequency / this->src_frequency;
	if (time == this->last_update)
		return;
	auto dst_clock = this->last_update + 1;
	for (; dst_clock <= time; dst_clock++)
		this->callback(dst_clock);
	this->last_update = time;
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

SoundController::SoundController(Gameboy &system):
		system(&system),
		audio_sample_clock(gb_cpu_frequency, sampling_frequency, [this](std::uint64_t n){ this->sample_callback(n); }),
		frame_sequencer_clock(gb_cpu_frequency, 512, [this](std::uint64_t n){ this->frame_sequencer_callback(n); }){
	this->initialize_new_frame();
}

void SoundController::update(bool required){
	auto c = this->system->get_system_clock().get_clock_value();
	this->frame_sequencer_clock.update(c);
	this->audio_sample_clock.update(c);
}

void SoundController::frame_sequencer_callback(std::uint64_t clock){
	if (!this->master_toggle)
		return;

	if (!(clock % 2))
		this->length_counter_event();
	if (clock % 8 == 7)
		this->volume_event();
	if (clock % 4 == 2)
		this->sweep_event();
}

void SoundController::sample_callback(std::uint64_t sample_no){
	StereoSampleIntermediate sample;
	sample.left = sample.right = 0;
	auto &dst = this->publishing_frames.get_private_resource()->buffer[this->current_frame_position];
	if (this->master_toggle){
		sample += this->render_square1(sample_no) / 4;
		sample += this->render_square2(sample_no) / 4;
		sample += this->render_voluntary(sample_no) / 4;
		sample += this->render_noise(sample_no) / 4;
		dst = convert(sample);
	}else
		dst.left = dst.right = 0;

	this->current_frame_position++;
	if (this->current_frame_position >= AudioFrame::length){
		this->current_frame_position = 0;
		this->publishing_frames.publish();
		this->initialize_new_frame();
	}
}

void SoundController::length_counter_event(){
	this->square1.length_counter_event();
}

void SoundController::volume_event(){
	this->square1.volume_event();
}

void SoundController::sweep_event(){
	this->square1.sweep_event();
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

template <typename T1, typename T2>
StereoSampleIntermediate compute_channel_panning_and_silence(const T1 &generator, std::uint64_t time, const T2 &pan){
	StereoSampleIntermediate ret;
	if (!!pan.either){
		auto value = generator.render(time);

		ret.left = value * !!pan.left;
		ret.right = value * !!pan.right;
	}else
		ret.left = ret.right = 0;

	return ret;
}

StereoSampleIntermediate SoundController::render_square1(std::uint64_t time){
	this->square1.update_state_before_render(time);
	return compute_channel_panning_and_silence(this->square1, time, this->stereo_panning[0]);
}

StereoSampleIntermediate SoundController::render_square2(std::uint64_t time){
	this->square2.update_state_before_render(time);
	return compute_channel_panning_and_silence(this->square2, time, this->stereo_panning[1]);
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

WaveformGenerator::WaveformGenerator(){
	std::fill(this->registers, this->registers + array_size(this->registers), 0);
}

void WaveformGenerator::trigger_event(){
}

void EnvelopedGenerator::trigger_event(){
	WaveformGenerator::trigger_event();
	this->envelope_time = this->envelope_period;
	this->volume = this->shadow_volume;
}

void Square1Generator::trigger_event(){
	Square2Generator::trigger_event();
	this->shadow_frequency = this->frequency;
	if (!!this->sweep_period & !!this->sweep_shift){
		this->sweep_event(true);
	}
}

unsigned Square2Generator::get_period(){
	if (!this->period)
		this->period = (2048 - this->frequency) * 4;
	return this->period;
}

void Square2Generator::advance_duty(std::uint64_t time){
	bool und1 = this->reference_time == this->undefined_reference_time;
	bool und2 = this->reference_duty_position == this->undefined_reference_duty_position;
	if (und1 & und2){
		this->reference_time = time;
		this->reference_duty_position = this->duty_position;
	}else{
		auto delta = time - this->reference_time;
		this->duty_position = this->reference_duty_position;
		this->duty_position += (unsigned)((delta << 13) * gb_cpu_frequency / (sampling_frequency * this->get_period()));
		this->duty_position &= 0xFFFF;
	}
}

void Square2Generator::update_state_before_render(std::uint64_t time){
	this->advance_duty(time);
}

intermediate_audio_type Square2Generator::render(std::uint64_t time) const{
	if (!this->enabled())
		return 0;

	int y = !!(this->duties[this->selected_duty] & bit(this->duty_position >> 13));
	y = (y * 2 - 1) * this->volume;
	return (intermediate_audio_type)y * (1.f / 15.f);
}

bool WaveformGenerator::enabled() const{
	return !this->length_enable | !!this->sound_length;
}

bool EnvelopedGenerator::enabled() const{
	return WaveformGenerator::enabled() & !!this->volume;
}

bool Square2Generator::enabled() const{
	//Note: if frequency value > 2041, sound frequency > 20 kHz
	return EnvelopedGenerator::enabled() & (this->frequency > 0) & (this->frequency <= 2041);
}

bool Square1Generator::enabled() const{
	return Square2Generator::enabled() & (this->shadow_frequency != this->audio_disabled_by_sweep);
}

void Square2Generator::set_register1(byte_t value){
	this->registers[1] = value;

	this->selected_duty = value >> 6;
	this->sound_length = this->shadow_sound_length = 64 - (value & 0x3F);
}

void Square2Generator::set_register2(byte_t value){
	this->registers[2] = value;

	this->volume = this->shadow_volume = value >> 4;
	this->envelope_sign = value & bit(3) ? 1 : -1;
	this->envelope_period = value & 0x07;
}

void Square2Generator::set_register3(byte_t value){
	this->registers[3] = value;

	auto old = this->frequency;
	this->frequency &= ~(decltype(this->frequency))0xFF;
	this->frequency |= value;
	this->frequency_change(old);
	this->sound_length = this->shadow_sound_length;
}

void Square2Generator::set_register4(byte_t value){
	this->registers[4] = value;

	bool trigger = !!(value & bit(7));
	this->length_enable = !!(value & bit(6));

	unsigned freq = value & 0x07;
	freq <<= 8;
	auto old = this->frequency;
	this->frequency &= ~(decltype(this->frequency))0x0700;
	this->frequency |= freq;
	this->frequency_change(old);

	this->sound_length = this->shadow_sound_length;

	if (trigger)
		this->trigger_event();
}

byte_t Square2Generator::get_register1() const{
	return this->registers[1] & 0x3F;
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

	this->sweep_period = (value & 0x70) >> 4;
	this->sweep_sign = value & bit(3) ? -1 : 1;
	this->sweep_shift = value & 0x07;
}

byte_t Square1Generator::get_register0() const{
	return this->registers[0];
}

void Square2Generator::frequency_change(unsigned old_frequency){
	if (this->frequency == old_frequency)
		return;
	this->period = 0;
	this->reference_time = this->undefined_reference_time;
	this->reference_duty_position = this->undefined_reference_duty_position;
}

void Square1Generator::sweep_event(bool force){
	if (!force){
		if (!this->sweep_period | !this->sweep_shift | !this->sweep_time)
			return;
		this->sweep_time--;
		if (this->sweep_time)
			return;
	}

	this->sweep_time = this->sweep_period;
	auto old = this->frequency;
	this->frequency = this->shadow_frequency;
	this->frequency_change(old);
	this->shadow_frequency += this->sweep_sign * (this->shadow_frequency >> this->sweep_shift);

	if (this->shadow_frequency < 0)
		this->shadow_frequency = 0;
	else if (this->shadow_frequency >= 2048){
		this->sweep_time = 0;
		this->shadow_frequency = this->audio_disabled_by_sweep;
	}
}

void EnvelopedGenerator::volume_event(){
	if (!this->envelope_period | !this->envelope_time)
		return;
	this->envelope_time--;
	if (this->envelope_time)
		return;

	this->envelope_time = this->envelope_period;
	this->volume += this->envelope_sign;
	//Saturate value into range [0; 15]. Note: this only works if this->envelope_sign == +/-1.
	this->volume -= this->volume >> 4;
}

void WaveformGenerator::length_counter_event(){
	if (this->sound_length)
		this->sound_length--;
}

void SoundController::set_NR50(byte_t value){
	this->NR50 = value;
}

void SoundController::set_NR51(byte_t value){
	this->NR51 = value;

	for (int channel = 0; channel < 4; channel++){
		auto &pan = this->stereo_panning[channel];
		pan.right = !!(value & bit(channel));
		pan.left = !!(value & bit(channel + 4));
		pan.either = pan.right | pan.left;
	}
}

void SoundController::set_NR52(byte_t value){
	this->master_toggle = !!(value & bit(7));
}

byte_t SoundController::get_NR52() const{
	//TODO: implement notification of length expirations.
	return ((byte_t)this->master_toggle << 7) & 0x7F;
}
