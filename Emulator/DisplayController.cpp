#include "DisplayController.h"
#include "GameboyCpu.h"
#include "MemoryController.h"
#include "exceptions.h"
#include <memory>
#include <fstream>

DisplayController::DisplayController(GameboyCpu &cpu): cpu(&cpu){
	
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
					pixel = this->palette[color].r;
				}
			}
		}
	}

	std::ofstream file(filename, std::ios::binary);
	file.write((const char *)dump.get(), dump_size);
}

unsigned DisplayController::get_row_status(){
	auto clock = this->cpu->get_current_clock();
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
	return this->palette_value;
}

void DisplayController::set_background_palette(byte_t palette){
	for (unsigned i = 4; i--;){
		auto index = (palette >> (i * 2)) & 3;
		byte_t c = ~(byte_t)(index * 0xFF / 3);
		this->palette[i] = { c, c, c, 0xFF };
	}
	this->palette_value = palette;
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

DEFINE_EMPTY_DISPLAY_CONTROLLER_PROPERTY(status);
DEFINE_EMPTY_DISPLAY_CONTROLLER_PROPERTY(y_coordinate_compare);
DEFINE_EMPTY_DISPLAY_CONTROLLER_PROPERTY(window_y_position);

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

/*
byte_t DisplayController::get_(){

}

void DisplayController::set_(byte_t){

}
*/
