#include "DisplayController.h"
#include "GameboyCpu.h"
#include "exceptions.h"

DisplayController::DisplayController(GameboyCpu &cpu): cpu(&cpu){
	
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
