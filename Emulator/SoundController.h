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

class Oscillator{
protected:
	bool enabled = false;
	byte_t registers[5];

	virtual void trigger_event();
public:
	virtual ~Oscillator(){}
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
};

class Square2Generator : public Oscillator{
protected:
	unsigned frequency = 0;
	unsigned period = 0;
	unsigned selected_duty = 2;
	std::uint64_t reference_time = 0;
	unsigned duty_position = 0;
	unsigned reference_duty_position = 0;

	static const byte_t duties[4];

	unsigned get_period();
	void advance_duty(std::uint64_t time);
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
public:
	void set_register0(byte_t value) override;
	byte_t get_register0() const override;
};

class SoundController{
	Gameboy *system;
	unsigned current_frame_position = 0;
	QueuedPublishingResource<AudioFrame> publishing_frames;
	std::uint64_t last_update = std::numeric_limits<std::uint64_t>::max();
	std::uint64_t modulo;
	std::uint64_t frame_no = 0;

	StereoSampleIntermediate render_square1(std::uint64_t time);
	StereoSampleIntermediate render_square2(std::uint64_t time);
	StereoSampleIntermediate render_voluntary(std::uint64_t time);
	StereoSampleIntermediate render_noise(std::uint64_t time);
	void initialize_new_frame();
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
