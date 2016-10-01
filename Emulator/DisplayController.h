#pragma once

#include "CommonTypes.h"
#include <unordered_map>

class Gameboy;
class GameboyCpu;
class MemoryController;

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
	byte_t r, g, b, a;
};

class DisplayController{
	Gameboy *system;
	MemoryController *memory_controller = nullptr;
	byte_t bg_palette_value = 0;
	byte_t obj0_palette_value = 0;
	byte_t obj1_palette_value = 0;
	RGB bg_palette[4];
	RGB obj0_palette[4];
	RGB obj1_palette[4];
	unsigned scroll_x = 0,
		scroll_y = 0;
	byte_t lcd_control = 0;
	bool renderer_notified = false;
	byte_t lcd_status = 0;
	unsigned window_x = 0,
		window_y = 0;
	byte_t y_compare = 0;

	static const byte_t stat_coincidence_interrupt_mask = 1 << 6;
	static const byte_t stat_oam_interrupt_mask = 1 << 5;
	static const byte_t stat_vblank_interrupt_mask = 1 << 4;
	static const byte_t stat_hblank_interrupt_mask = 1 << 3;
	static const byte_t stat_coincidence_flag_mask = 1 << 2;
	static const byte_t stat_comp_coincidence_flag_mask = ~stat_coincidence_flag_mask;
	static const byte_t stat_mode_flag_mask = 3;
	static const byte_t stat_writing_filter_mask = 0x78;
	static const byte_t stat_comp_writing_filter_mask = ~stat_writing_filter_mask;

	void synchronize();
	unsigned get_row_status();
public:
	DisplayController(Gameboy &system);
	void set_memory_controller(MemoryController &mc){
		this->memory_controller = &mc;
	}
	void dump_background(const char *filename, MemoryController &);
	bool ready_to_draw();
	void render_to(byte_t *pixels, int pitch);

	DECLARE_DISPLAY_RO_CONTROLLER_PROPERTY(y_coordinate);
	DECLARE_DISPLAY_CONTROLLER_PROPERTY(status);
	DECLARE_DISPLAY_CONTROLLER_PROPERTY(y_coordinate_compare);
	DECLARE_DISPLAY_CONTROLLER_PROPERTY(window_x_position);
	DECLARE_DISPLAY_CONTROLLER_PROPERTY(window_y_position);
	DECLARE_DISPLAY_CONTROLLER_PROPERTY(scroll_x);
	DECLARE_DISPLAY_CONTROLLER_PROPERTY(scroll_y);
	DECLARE_DISPLAY_CONTROLLER_PROPERTY(background_palette);
	DECLARE_DISPLAY_CONTROLLER_PROPERTY(obj0_palette);
	DECLARE_DISPLAY_CONTROLLER_PROPERTY(obj1_palette);
	DECLARE_DISPLAY_CONTROLLER_PROPERTY(lcd_control);
};

const unsigned lcd_refresh_period = 70224;
//LCD refresh rate: ~59.7275 Hz (exactly gb_cpu_frequency/lcd_refresh_period Hz)
const unsigned lcd_width = 160;
const unsigned lcd_height = 144;
