#pragma once

#include "CommonTypes.h"
#include "utility.h"

class Gameboy;

template <typename T>
struct basic_StereoSample{
	typedef T type;
	T left, right;
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

struct AudioFrame{
	static const unsigned length = 1024;
	std::uint64_t frame_no;
	StereoSampleFinal buffer[length];
};

class SoundController{
	Gameboy *system;
	unsigned current_frame_position = 0;
	PublishingResource<AudioFrame> publishing_frames;
	std::uint64_t last_update = std::numeric_limits<std::uint64_t>::max();
	std::uint64_t modulo;
	std::uint64_t frame_no = 0;

	static intermediate_audio_type base_wave_generator(std::uint64_t time, unsigned frequency);
	StereoSampleIntermediate render_square1(std::uint64_t time);
	StereoSampleIntermediate render_square2(std::uint64_t time);
	StereoSampleIntermediate render_voluntary(std::uint64_t time);
	StereoSampleIntermediate render_noise(std::uint64_t time);
	void initialize_new_frame();
public:
	SoundController(Gameboy &);
	void update(bool required = false);
	AudioFrame *get_current_frame();
	void return_used_frame(AudioFrame *);
};
