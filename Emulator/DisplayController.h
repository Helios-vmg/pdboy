#pragma once

#include "CommonTypes.h"
#include "MemorySection.h"
#include <unordered_map>
#include <atomic>
#include <mutex>

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

const unsigned lcd_refresh_period = 70224;
//LCD refresh rate: ~59.7275 Hz (exactly gb_cpu_frequency/lcd_refresh_period Hz)
const unsigned lcd_width = 160;
const unsigned lcd_height = 144;

template <typename T1, typename T2>
static bool check_flag(T1 value, T2 mask){
	return ((T2)value & mask) == mask;
}

struct RGB{
	byte_t r, g, b, a;
};

struct RenderedFrame{
	RGB pixels[lcd_width * lcd_height];
};

class DisplayController{
	Gameboy *system;
	MemoryController *memory_controller = nullptr;
	MemorySection<0x8000> vram;
	MemorySection<0xFE00> oam;
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
	unsigned last_in_new_frame = 0;

	std::vector<std::unique_ptr<RenderedFrame>> allocated_frames;
	std::vector<RenderedFrame *> ready_frames;
	std::mutex ready_frames_mutex;
	//Invariant: frame_being_drawn is valid at all times.
	RenderedFrame *frame_being_drawn;
	std::atomic<RenderedFrame *> current_frame;

	static const byte_t stat_coincidence_interrupt_mask = 1 << 6;
	static const byte_t stat_oam_interrupt_mask = 1 << 5;
	static const byte_t stat_vblank_interrupt_mask = 1 << 4;
	static const byte_t stat_hblank_interrupt_mask = 1 << 3;
	static const byte_t stat_coincidence_flag_mask = 1 << 2;
	static const byte_t stat_comp_coincidence_flag_mask = ~stat_coincidence_flag_mask;
	static const byte_t stat_mode_flag_mask = 3;
	static const byte_t stat_writing_filter_mask = 0x78;
	static const byte_t stat_comp_writing_filter_mask = ~stat_writing_filter_mask;

	static const byte_t lcdc_display_enable_mask = 1 << 7;
	static const byte_t lcdc_window_map_select_mask = 1 << 6;
	static const byte_t lcdc_window_enable_mask = 1 << 5;
	static const byte_t lcdc_tile_map_select_mask = 1 << 4;
	static const byte_t lcdc_bg_map_select_mask = 1 << 3;
	static const byte_t lcdc_tall_sprite_enable_mask = 1 << 2;
	static const byte_t lcdc_sprite_enable_mask = 1 << 1;
	static const byte_t lcdc_bg_enable_mask = 1 << 0;

	unsigned get_row_status();
	unsigned get_tile_vram_offset() const{
		return 0x800 * !check_flag(this->lcd_control, lcdc_tile_map_select_mask);
	}
	unsigned get_bg_vram_offset() const{
		return 0x400 * check_flag(this->lcd_control, lcdc_bg_map_select_mask);
	}
	unsigned get_window_vram_offset() const{
		return 0x400 * check_flag(this->lcd_control, lcdc_window_map_select_mask);
	}
	RenderedFrame *allocate_frame();
	RenderedFrame *reuse_or_allocate_frame();
	void publish_rendered_frame();
	void render();
public:
	DisplayController(Gameboy &system);
	void set_memory_controller(MemoryController &mc){
		this->memory_controller = &mc;
	}
	bool in_new_frame();
	bool ready_to_draw();

	RenderedFrame *get_current_frame();
	void return_used_frame(RenderedFrame *);


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

	byte_t &access_vram(main_integer_t address){
		return this->vram.access(address);
	}
	byte_t &access_oam(main_integer_t address){
		return this->oam.access(address);
	}
	const byte_t &access_vram(main_integer_t address) const{
		return this->vram.access(address);
	}
	const byte_t &access_oam(main_integer_t address) const{
		return this->oam.access(address);
	}
	const byte_t *get_sprite_tile_vram() const{
		return &this->access_vram(0x8000);
	}
	const byte_t *get_bg_tile_vram() const{
		return &this->access_vram(0x8000 + this->get_tile_vram_offset());
	}
	const byte_t *get_bg_vram() const{
		return &this->access_vram(0x9800 + this->get_bg_vram_offset());
	}
	const byte_t *get_window_vram() const{
		return &this->access_vram(0x9800 + this->get_window_vram_offset());
	}
	const byte_t *get_oam() const{
		return &this->access_oam(0xFE00);
	}
};
