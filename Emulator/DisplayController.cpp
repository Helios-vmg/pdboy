#include "DisplayController.h"
#include "Gameboy.h"
#include "MemoryController.h"
#include "exceptions.h"
#include <memory>
#include <fstream>
#include <iostream>

DisplayController::DisplayController(Gameboy &system): system(&system){
	this->set_background_palette(0);
	this->set_obj0_palette(0);
	this->set_obj1_palette(0);
}

void DisplayController::dump_background(const char *filename, MemoryController &mc){
	const size_t size_tiles = 0x9800 - 0x8000;
	const size_t size_bg = 0x9C00 - 0x9800;
	std::unique_ptr<byte_t[]> tiles(new byte_t[size_tiles]);
	std::unique_ptr<byte_t[]> bg(new byte_t[size_bg]);
	mc.copy_out_memory_at(tiles.get(), size_tiles, 0x8000);
	mc.copy_out_memory_at(bg.get(), size_bg, 0x9800);

	const size_t dump_size = 8 * 8 * 32 * 32;
	std::unique_ptr<byte_t[]> dump(new byte_t[dump_size]);
	for (int y = 0; y < 32; y++){
		for (int x = 0; x < 32; x++){
			byte_t bg_value = bg[x + y * 32];
			auto tile = tiles.get() + bg_value * 16;
			for (int y2 = 0; y2 < 8; y2++){
				auto bitsA = tile[y2 * 2 + 0];
				auto bitsB = tile[y2 * 2 + 1];
				for (int x2 = 0; x2 < 8; x2++){
					auto &pixel = dump[x * 8 + x2 + (32 * 8)*(y * 8 + y2)];
					unsigned color = ((bitsA >> (7 - x2)) & 1) | (((bitsB >> (7 - x2)) & 1) << 1);
					pixel = this->bg_palette[color].r;
				}
			}
		}
	}

	std::ofstream file(filename, std::ios::binary);
	file.write((const char *)dump.get(), dump_size);
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

unsigned frames_drawn = 0;

void DisplayController::render_to(byte_t *pixels, int pitch){
	if (!this->memory_controller)
		throw GenericException("Emulator internal error: DisplayController::render_to() has been called before calling DisplayController::set_memory_controller().");

	auto tile_vram = this->memory_controller->direct_memory_access(0x8000);
	auto bg_vram = this->memory_controller->direct_memory_access(0x9800);
	for (int y = 0; y < lcd_height; y++){
		auto row = pixels + pitch * y;
		auto src_y = (y + this->scroll_y) & 0xFF;
		auto src_y_prime = src_y / 8 * 32;
		auto tile_offset_y = src_y & 7;
		for (int x = 0; x < lcd_width; x++){
			auto dst_pixel = row + x * 4;
			auto src_x = (x + this->scroll_x) & 0xFF;
			auto src_bg_tile = src_x / 8 + src_y_prime;
			auto tile_offset_x = src_x & 7;
			auto tile_no = bg_vram[src_bg_tile];
			auto tile = tile_vram + tile_no * 16;
			auto src_pixelA = tile[tile_offset_y * 2 + 0];
			auto src_pixelB = tile[tile_offset_y * 2 + 1];
			unsigned color = (src_pixelA >> (7 - tile_offset_x) & 1) | (((src_pixelB >> (7 - tile_offset_x)) & 1) << 1);
			auto rgba = this->bg_palette[color];
			dst_pixel[0] = rgba.r;
			dst_pixel[1] = rgba.g;
			dst_pixel[2] = rgba.b;
			dst_pixel[3] = 0xFF;
		}
	}
	frames_drawn++;
}

unsigned DisplayController::get_row_status(){
	auto clock = this->system->get_clock_value();
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

DEFINE_EMPTY_DISPLAY_CONTROLLER_PROPERTY(y_coordinate_compare);

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

void DisplayController::set_lcd_control(byte_t b){
	this->lcd_control = b;
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
