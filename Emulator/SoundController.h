#pragma once

#include "CommonTypes.h"
#include "PublishingResource.h"

class Gameboy;

template <typename T>
struct basic_StereoSample{
	typedef T type;
	T left, right;

	const basic_StereoSample<T> &operator+=(const basic_StereoSample<T> &other){
		this->left += other.left;
		this->right += other.right;
		return *this;
	}
	const basic_StereoSample<T> operator-=(const basic_StereoSample<T> &other){
		this->left -= other.left;
		this->right -= other.right;
		return *this;
	}
	basic_StereoSample<T> operator+(const basic_StereoSample<T> &other) const{
		auto ret = *this;
		ret += other;
		return ret;
	}
	basic_StereoSample<T> operator-(const basic_StereoSample<T> &other) const{
		auto ret = *this;
		ret -= other;
		return ret;
	}
	basic_StereoSample<T> operator-() const{
		auto ret = *this;
		ret.left = -ret.left;
		ret.right = -ret.right;
		return ret;
	}
	basic_StereoSample<T> operator/(T n) const{
		auto ret = *this;
		ret.left /= n;
		ret.right /= n;
		return ret;
	}
};

typedef float intermediate_audio_type;
typedef basic_StereoSample<std::int16_t> StereoSampleFinal;
typedef basic_StereoSample<intermediate_audio_type> StereoSampleIntermediate;

template <typename T>
struct AudioTypeProperties{
};

template <>
struct AudioTypeProperties<std::int16_t>{
	static const std::int16_t max = std::numeric_limits<std::int16_t>::max();
	static const std::int16_t min = std::numeric_limits<std::int16_t>::min();
	static const std::int16_t max_absolute = (std::int32_t)max <= -(std::int32_t)min ? max : -min;
};

template <>
struct AudioTypeProperties<float>{
	static const float max;
	static const float min;
	static const float max_absolute;
};

basic_StereoSample<std::int16_t> convert(const basic_StereoSample<float> &);

struct AudioFrame{
	static const unsigned length = 1024;
	std::uint64_t frame_no;
	StereoSampleFinal buffer[length];
};

class WaveformGenerator{
protected:
	byte_t registers[5];

	//Sound length
	unsigned sound_length = 0,
		shadow_sound_length = 0;
	bool length_enable = false;

	virtual void trigger_event();
	virtual bool enabled();
public:
	WaveformGenerator();
	virtual ~WaveformGenerator(){}
	virtual intermediate_audio_type render(std::uint64_t time) = 0;
	virtual void set_register0(byte_t){}
	virtual void set_register1(byte_t){}
	virtual void set_register2(byte_t){}
	virtual void set_register3(byte_t){}
	virtual void set_register4(byte_t){}
	virtual byte_t get_register0() const{
		return 0xFF;
	}
	virtual byte_t get_register1() const{
		return 0xFF;
	}
	virtual byte_t get_register2() const{
		return 0xFF;
	}
	virtual byte_t get_register3() const{
		return 0xFF;
	}
	virtual byte_t get_register4() const{
		return 0xFF;
	}
	void length_counter_event();
};

class EnvelopedGenerator : public WaveformGenerator{
protected:
	//Envelope
	int envelope_sign = 0;
	unsigned envelope_period = 0;
	unsigned envelope_time = 0;
	int volume = 0,
		shadow_volume = 0;

	virtual bool enabled() override;
	virtual void trigger_event() override;
public:
	virtual ~EnvelopedGenerator(){}
	void volume_event();
};

class Square2Generator : public EnvelopedGenerator{
protected:
	unsigned frequency = 0;
	unsigned period = 0;
	unsigned selected_duty = 2;
	std::uint64_t reference_time = 0;
	unsigned duty_position = 0;
	unsigned reference_duty_position = 0;
	const decltype(reference_time) undefined_reference_time = std::numeric_limits<decltype(reference_time)>::max();
	const decltype(reference_duty_position) undefined_reference_duty_position = std::numeric_limits<decltype(reference_duty_position)>::max();

	static const byte_t duties[4];

	unsigned get_period();
	void advance_duty(std::uint64_t time);
	void frequency_change(unsigned old_frequency);
	virtual bool enabled() override;
public:
	virtual ~Square2Generator(){}
	intermediate_audio_type render(std::uint64_t time);

	virtual void set_register1(byte_t value) override;
	virtual void set_register2(byte_t value) override;
	virtual void set_register3(byte_t value) override;
	virtual void set_register4(byte_t value) override;
	virtual byte_t get_register1() const override;
	virtual byte_t get_register2() const override;
	virtual byte_t get_register3() const override;
	virtual byte_t get_register4() const override;
};

class Square1Generator : public Square2Generator{
	unsigned sweep_period = 0;
	unsigned sweep_time = 0;
	int sweep_sign = 0;
	unsigned sweep_shift = 0;
	int shadow_frequency = 0;
	static const unsigned audio_disabled_by_sweep = 2048;

	bool enabled() override;
	void trigger_event() override;
public:
	void set_register0(byte_t value) override;
	byte_t get_register0() const override;
	void sweep_event(bool force = false);
};

class ClockDivider{
public:
	typedef std::function<void(std::uint64_t)> callback_t;
private:
	callback_t callback;
	std::uint64_t src_frequency,
		dst_frequency;
	std::uint64_t modulo;
	std::uint64_t last_update = std::numeric_limits<std::uint64_t>::max();
public:
	ClockDivider(std::uint64_t src_frequency, std::uint64_t dst_frequency, callback_t &&callback);
	void update(std::uint64_t);
};

class SoundController{
	Gameboy *system;
	unsigned current_frame_position = 0;
	QueuedPublishingResource<AudioFrame> publishing_frames;
	std::uint64_t frame_no = 0;
	ClockDivider frame_sequencer_clock,
		audio_sample_clock;

	StereoSampleIntermediate render_square1(std::uint64_t time);
	StereoSampleIntermediate render_square2(std::uint64_t time);
	StereoSampleIntermediate render_voluntary(std::uint64_t time);
	StereoSampleIntermediate render_noise(std::uint64_t time);
	void initialize_new_frame();
	void sample_callback(std::uint64_t);
	void frame_sequencer_callback(std::uint64_t);
	void length_counter_event();
	void volume_event();
	void sweep_event();
public:
	Square1Generator square1;

	SoundController(Gameboy &);
	void update(bool required = false);
	AudioFrame *get_current_frame();
	void return_used_frame(AudioFrame *);

	static intermediate_audio_type base_wave_generator_duty_50(std::uint64_t time, unsigned frequency);
	static intermediate_audio_type base_wave_generator_duty_12(std::uint64_t time, unsigned frequency);
	static intermediate_audio_type base_wave_generator_duty_25(std::uint64_t time, unsigned frequency);
	static intermediate_audio_type base_wave_generator_duty_75(std::uint64_t time, unsigned frequency);
};
