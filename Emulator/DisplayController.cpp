#include "DisplayController.h"
#include "Gameboy.h"
#include "MemoryController.h"
#include "exceptions.h"
#include <memory>
#include <fstream>
#include <iostream>

DisplayController::DisplayController(Gameboy &system):
		system(&system),
		vram(0x2000),
		oam(0xA0){
	this->set_background_palette(0);
	this->set_obj0_palette(0);
	this->set_obj1_palette(0);
}

bool DisplayController::ready_to_draw(){
	auto status = (this->get_row_status() & 3) != 3;
	if (this->renderer_notified){
		if (status)
			this->renderer_notified = false;
		return false;
	}
	return this->renderer_notified = !status;
}

const unsigned sprite_y_pos_offset = 0;
const unsigned sprite_x_pos_offset = 1;
const unsigned sprite_tile_offset = 2;
const unsigned sprite_attr_offset = 3;

unsigned frames_drawn = 0;

void DisplayController::render_to(byte_t *pixels, int pitch){
	if (!this->memory_controller)
		throw GenericException("Emulator internal error: DisplayController::render_to() has been called before calling DisplayController::set_memory_controller().");

	auto bg_tile_vram = this->get_bg_tile_vram();
	auto bg_vram = this->get_bg_vram();
	auto oam = this->get_oam();
	auto sprite_tile_vram = this->get_sprite_tile_vram();
	bool bg_enabled = check_flag(this->lcd_control, lcdc_bg_enable_mask);
	bool sprites_enabled = check_flag(this->lcd_control, lcdc_sprite_enable_mask);
	const unsigned sprite_width = 8;
	bool tall_sprites = check_flag(this->lcd_control, lcdc_tall_sprite_enable_mask);
	unsigned sprite_height = tall_sprites ? 16 : 8;
	for (unsigned y = 0; y != lcd_height; y++){
		auto row = pixels + pitch * y;
		auto src_y = (y + this->scroll_y) & 0xFF;
		auto src_y_prime = src_y / 8 * 32;
		auto tile_offset_y = src_y & 7;

		const byte_t *sprites_for_scanline[40];
		unsigned sprites_for_scanline_size = 0;
		if (sprites_enabled){
			for (unsigned i = 40; i--;){
				auto sprite = oam + i * 4;
				auto spry = (unsigned)sprite[sprite_y_pos_offset] - 16U;
				if (y >= spry && y < spry + sprite_height)
					sprites_for_scanline[sprites_for_scanline_size++] = sprite;
			}
		}

		if (sprites_for_scanline_size > 10)
			sprites_for_scanline_size = 0;

		for (unsigned x = 0; x != lcd_width; x++){
			unsigned color = 0;
			auto rgba = this->bg_palette[color];
			auto dst_pixel = row + x * 4;
			if (bg_enabled){
				auto src_x = (x + this->scroll_x) & 0xFF;
				auto src_bg_tile = src_x / 8 + src_y_prime;
				auto tile_offset_x = src_x & 7;
				auto tile_no = bg_vram[src_bg_tile];
				auto tile = bg_tile_vram + tile_no * 16;
				auto src_pixelA = tile[tile_offset_y * 2 + 0];
				auto src_pixelB = tile[tile_offset_y * 2 + 1];
				color = (src_pixelA >> (7 - tile_offset_x) & 1) | (((src_pixelB >> (7 - tile_offset_x)) & 1) << 1);
				rgba = this->bg_palette[color];
			}
			for (unsigned i = sprites_for_scanline_size; i--;){
				auto sprite = sprites_for_scanline[i];
				auto sprx = (unsigned)sprite[sprite_x_pos_offset] - 8U;
				if (x >= sprx && x < sprx + sprite_width){
					auto attr = sprite[sprite_attr_offset];
					auto tile_no = sprite[sprite_tile_offset];
					auto spry = (unsigned)sprite[sprite_y_pos_offset] - 16U;
					auto tile_offset_y = y - spry;
					auto tile_offset_x = x - sprx;
					if (tall_sprites){
						tile_no &= 0xFE;
						tile_no += tile_offset_y / 8;
					}
					auto tile = sprite_tile_vram + tile_no * 16;
					auto src_pixelA = tile[tile_offset_y * 2 + 0];
					auto src_pixelB = tile[tile_offset_y * 2 + 1];
					color = (src_pixelA >> (7 - tile_offset_x) & 1) | (((src_pixelB >> (7 - tile_offset_x)) & 1) << 1);
					rgba = this->bg_palette[color];
					break;
				}
			}
			dst_pixel[0] = rgba.r;
			dst_pixel[1] = rgba.g;
			dst_pixel[2] = rgba.b;
			dst_pixel[3] = 0xFF;
		}
	}
	frames_drawn++;
}

unsigned DisplayController::get_row_status(){
	auto clock = this->system->get_system_clock().get_clock_value();
	auto cycle = (unsigned)(clock % lcd_refresh_period);
	auto row = cycle / 456;
	auto sub_row = cycle % 456;
	if (row >= lcd_height)
		return row * 4 + 3;
	row *= 4;
	if (sub_row < 80)
		return row + 0;
	if (sub_row < 252)
		return row + 1;
	return row + 2;
}

byte_t DisplayController::get_background_palette(){
	return this->bg_palette_value;
}

template <bool BG>
static void set_palette_array(RGB dst[4], byte_t palette){
	for (unsigned i = 4; i--;){
		auto index = (palette >> (i * 2)) & 3;
		byte_t c = ~(byte_t)(index * 0xFF / 3);
		dst[i] = { c, c, c, 0xFF };
	}
	if (!BG)
		dst[0] = { 0, 0, 0, 0 };
}

void DisplayController::set_background_palette(byte_t palette){
	set_palette_array<true>(this->bg_palette, palette);
	this->bg_palette_value = palette;
}

#define DEFINE_EMPTY_DISPLAY_RO_CONTROLLER_PROPERTY(x) \
	byte_t DisplayController::get_##x(){ \
		throw NotImplementedException(); \
	}

#define DEFINE_EMPTY_DISPLAY_CONTROLLER_PROPERTY(x) \
	DEFINE_EMPTY_DISPLAY_RO_CONTROLLER_PROPERTY(x) \
	void DisplayController::set_##x(byte_t){ \
		throw NotImplementedException(); \
	}

byte_t DisplayController::get_y_coordinate(){
	return (byte_t)(this->get_row_status() / 4);
}

byte_t DisplayController::get_status(){
	auto ret = this->lcd_status & this->stat_comp_coincidence_flag_mask;
	ret |= (this->get_y_coordinate_compare() == this->get_y_coordinate()) * this->stat_coincidence_flag_mask;
	return ret;
}

void DisplayController::set_status(byte_t b){
	b &= this->stat_writing_filter_mask;
	this->lcd_status &= this->stat_comp_writing_filter_mask;
	this->lcd_status |= b;
}

byte_t DisplayController::get_y_coordinate_compare(){
	return this->y_compare;
}

void DisplayController::set_y_coordinate_compare(byte_t b){
	this->y_compare = b;
}

byte_t DisplayController::get_window_x_position(){
	return this->window_x;
}

void DisplayController::set_window_x_position(byte_t b){
	this->window_x = b;
}

byte_t DisplayController::get_window_y_position(){
	return this->window_y;
}

void DisplayController::set_window_y_position(byte_t b){
	this->window_y = b;
}

byte_t DisplayController::get_scroll_x(){
	return (byte_t)this->scroll_x;
}

void DisplayController::set_scroll_x(byte_t b){
	this->scroll_x = b;
}

byte_t DisplayController::get_scroll_y(){
	return (byte_t)this->scroll_y;
}

void DisplayController::set_scroll_y(byte_t b){
	this->scroll_y = b;
}

byte_t DisplayController::get_lcd_control(){
	return this->lcd_control;
}

#define CHECK_FLAG(x) std::cout << (check_flag(this->lcd_control, x) ? " " : "~") << #x "\n"

void DisplayController::set_lcd_control(byte_t b){
	this->lcd_control = b;
	std::cout << "\nLCDC changed: " << std::hex << (int)this->lcd_control << std::endl;
	CHECK_FLAG(lcdc_display_enable_mask);
	CHECK_FLAG(lcdc_window_map_select_mask);
	CHECK_FLAG(lcdc_window_enable_mask);
	CHECK_FLAG(lcdc_tile_map_select_mask);
	CHECK_FLAG(lcdc_bg_map_select_mask);
	CHECK_FLAG(lcdc_tall_sprite_enable_mask);
	CHECK_FLAG(lcdc_sprite_enable_mask);
	CHECK_FLAG(lcdc_bg_enable_mask);
}

byte_t DisplayController::get_obj0_palette(){
	return this->obj0_palette_value;
}

void DisplayController::set_obj0_palette(byte_t palette){
	set_palette_array<false>(this->obj0_palette, palette);
	this->obj0_palette_value = palette;
}

byte_t DisplayController::get_obj1_palette(){
	return this->obj1_palette_value;
}

void DisplayController::set_obj1_palette(byte_t palette){
	set_palette_array<false>(this->obj1_palette, palette);
	this->obj1_palette_value = palette;
}

/*
byte_t DisplayController::get_(){

}

void DisplayController::set_(byte_t){

}
*/
