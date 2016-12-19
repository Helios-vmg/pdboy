#pragma once

#include "CommonTypes.h"
#include "PublishingResource.h"
#include <fstream>

//#define OUTPUT_AUDIO_TO_FILE

class Gameboy;
class SoundController;

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
	SoundController *parent;

	byte_t registers[5];

	//Sound length
	unsigned sound_length = 0,
		shadow_sound_length = 0;
	bool length_enable = false;

	virtual void trigger_event();
	virtual bool enabled() const;
public:
	WaveformGenerator(SoundController &parent);
	virtual ~WaveformGenerator(){}
	virtual void update_state_before_render(std::uint64_t time){}
	virtual intermediate_audio_type render(std::uint64_t time) const = 0;
	virtual void set_register1(byte_t){}
	virtual void set_register2(byte_t){}
	virtual void set_register3(byte_t){}
	virtual void set_register4(byte_t){}
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

	virtual bool enabled() const override;
	virtual void trigger_event() override;
public:
	EnvelopedGenerator(SoundController &parent):
		WaveformGenerator(parent){}
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
	virtual bool enabled() const override;
public:
	Square2Generator(SoundController &parent) :
		EnvelopedGenerator(parent){}
	virtual ~Square2Generator(){}
	void update_state_before_render(std::uint64_t time) override;
	intermediate_audio_type render(std::uint64_t time) const override;

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
	std::uint64_t last_sweep = 0;

	bool enabled() const override;
	void trigger_event() override;
public:
	Square1Generator(SoundController &parent) :
		Square2Generator(parent){}
	void set_register0(byte_t value);
	byte_t get_register0() const;
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
	std::uint64_t last_update;
public:
	ClockDivider(std::uint64_t src_frequency, std::uint64_t dst_frequency, callback_t &&callback);
	void update(std::uint64_t);
	void reset();
};

class SoundController{
	Gameboy *system;
	unsigned current_frame_position = 0;
	QueuedPublishingResource<AudioFrame> publishing_frames;
	std::uint64_t frame_no = 0;
	std::uint64_t audio_turned_on_at = 0;
	std::uint64_t current_clock;
	ClockDivider frame_sequencer_clock,
		audio_sample_clock;

	//Control registers.
	byte_t NR50 = 0;
	byte_t NR51 = 0;
	bool master_toggle = false;

	struct Panning{
		bool left = true,
			right = true,
			either = true;
	};

	Panning stereo_panning[4];

#ifdef OUTPUT_AUDIO_TO_FILE
	std::unique_ptr<std::ofstream> output_file;
#endif

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
	Square2Generator square2;

	SoundController(Gameboy &);
	void update(bool required = false);
	AudioFrame *get_current_frame();
	void return_used_frame(AudioFrame *);
	std::uint64_t get_current_clock() const{
		return this->current_clock;
	}

	void set_NR50(byte_t);
	void set_NR51(byte_t);
	void set_NR52(byte_t);
	byte_t get_NR50() const{
		return this->NR50;
	}
	byte_t get_NR51() const{
		return this->NR51;
	}
	byte_t get_NR52() const;

	static intermediate_audio_type base_wave_generator_duty_50(std::uint64_t time, unsigned frequency);
	static intermediate_audio_type base_wave_generator_duty_12(std::uint64_t time, unsigned frequency);
	static intermediate_audio_type base_wave_generator_duty_25(std::uint64_t time, unsigned frequency);
	static intermediate_audio_type base_wave_generator_duty_75(std::uint64_t time, unsigned frequency);
};
