#pragma once

#include "CommonTypes.h"
#include <unordered_map>

class GameboyCpu;

#define DECLARE_DISPLAY_RO_CONTROLLER_PROPERTY(x) byte_t get_##x()

#define DECLARE_DISPLAY_CONTROLLER_PROPERTY(x) \
	DECLARE_DISPLAY_RO_CONTROLLER_PROPERTY(x); \
	void set_##x(byte_t)

#define DEFINE_INLINE_DISPLAY_CONTROLLER_PROPERTY(x) \
	byte_t get_##x() const{ \
		return (byte_t)this->x; \
	} \
	void set_##x(byte_t b){ \
		this->x = (decltype(this->x))b; \
	}

struct RGB{
	byte_t r, g, b, unused;
};

class DisplayController{
	GameboyCpu *cpu;
	byte_t palette_value = 0;
	std::unordered_map<unsigned, RGB> palette;
	unsigned scroll_x = 0,
		scroll_y = 0;
	byte_t lcd_control;

	void synchronize();
	unsigned get_row_status();
public:
	DisplayController(GameboyCpu &cpu);

	DECLARE_DISPLAY_RO_CONTROLLER_PROPERTY(y_coordinate);
	DECLARE_DISPLAY_CONTROLLER_PROPERTY(status);
	DECLARE_DISPLAY_CONTROLLER_PROPERTY(y_coordinate_compare);
	DECLARE_DISPLAY_CONTROLLER_PROPERTY(window_y_position);
	DECLARE_DISPLAY_CONTROLLER_PROPERTY(scroll_x);
	DECLARE_DISPLAY_CONTROLLER_PROPERTY(scroll_y);
	DECLARE_DISPLAY_CONTROLLER_PROPERTY(background_palette);
	DECLARE_DISPLAY_CONTROLLER_PROPERTY(lcd_control);
};

const unsigned lcd_refresh_period = 70224;
//LCD refresh rate: ~59.7275 Hz (exactly lcd_refresh_period/gb_cpu_frequency Hz)
const unsigned lcd_width = 160;
const unsigned lcd_height = 144;
