#include "MemoryController.h"
#include "GameboyCpu.h"
#include "Gameboy.h"
#include "exceptions.h"
#include <cstdlib>
#include <exception>
#include <algorithm>

//#define USE_CPP_UNDEFINED_BEHAVIOR
#define LITTLE_ENDIAN

unsigned char gb_bootstrap_rom[] = {
	0x31,0xFE,0xFF,0xAF,0x21,0xFF,0x9F,0x32,0xCB,0x7C,0x20,0xFB,0x21,0x26,0xFF,0x0E,
	0x11,0x3E,0x80,0x32,0xE2,0x0C,0x3E,0xF3,0xE2,0x32,0x3E,0x77,0x77,0x3E,0xFC,0xE0,
	0x47,0x11,0x04,0x01,0x21,0x10,0x80,0x1A,0xCD,0x95,0x00,0xCD,0x96,0x00,0x13,0x7B,
	0xFE,0x34,0x20,0xF3,0x11,0xD8,0x00,0x06,0x08,0x1A,0x13,0x22,0x23,0x05,0x20,0xF9,
	0x3E,0x19,0xEA,0x10,0x99,0x21,0x2F,0x99,0x0E,0x0C,0x3D,0x28,0x08,0x32,0x0D,0x20,
	0xF9,0x2E,0x0F,0x18,0xF3,0x67,0x3E,0x64,0x57,0xE0,0x42,0x3E,0x91,0xE0,0x40,0x04,
	0x1E,0x02,0x0E,0x0C,0xF0,0x44,0xFE,0x90,0x20,0xFA,0x0D,0x20,0xF7,0x1D,0x20,0xF2,
	0x0E,0x13,0x24,0x7C,0x1E,0x83,0xFE,0x62,0x28,0x06,0x1E,0xC1,0xFE,0x64,0x20,0x06,
	0x7B,0xE2,0x0C,0x3E,0x87,0xE2,0xF0,0x42,0x90,0xE0,0x42,0x15,0x20,0xD2,0x05,0x20,
	0x4F,0x16,0x20,0x18,0xCB,0x4F,0x06,0x04,0xC5,0xCB,0x11,0x17,0xC1,0xCB,0x11,0x17,
	0x05,0x20,0xF5,0x22,0x23,0x22,0x23,0xC9,0xCE,0xED,0x66,0x66,0xCC,0x0D,0x00,0x0B,
	0x03,0x73,0x00,0x83,0x00,0x0C,0x00,0x0D,0x00,0x08,0x11,0x1F,0x88,0x89,0x00,0x0E,
	0xDC,0xCC,0x6E,0xE6,0xDD,0xDD,0xD9,0x99,0xBB,0xBB,0x67,0x63,0x6E,0x0E,0xEC,0xCC,
	0xDD,0xDC,0x99,0x9F,0xBB,0xB9,0x33,0x3E,0x3C,0x42,0xB9,0xA5,0xB9,0xA5,0x42,0x3C,
	0x21,0x04,0x01,0x11,0xA8,0x00,0x1A,0x13,0xBE,0x20,0xFE,0x23,0x7D,0xFE,0x34,0x20,
	0xF5,0x06,0x19,0x78,0x86,0x23,0x05,0x20,0xFB,0x86,0x20,0xFE,0x3E,0x01,0xE0,0x50
};
const size_t gb_bootstrap_rom_size = sizeof(gb_bootstrap_rom);

const size_t function_table_sizes = 0x4C;

MemoryController::MemoryController(Gameboy &system, GameboyCpu &cpu):
		system(&system),
		cpu(&cpu),
		display(&system.get_display_controller()),
		memoryp(new byte_t[0x10000]),
		stor_functions(new store_func_t[function_table_sizes]),
		load_functions(new load_func_t[function_table_sizes]){
}

void MemoryController::initialize(){
	this->display->set_memory_controller(*this);
	this->memory = this->memoryp.get();
	this->initialize_functions();
}

void MemoryController::initialize_functions(){
	std::fill(this->stor_functions.get(), this->stor_functions.get() + function_table_sizes, nullptr);
	std::fill(this->load_functions.get(), this->load_functions.get() + function_table_sizes, nullptr);

	this->stor_functions[0x00] = &MemoryController::store_not_implemented;
	this->load_functions[0x00] = &MemoryController::load_not_implemented;
	this->stor_functions[0x01] = &MemoryController::store_not_implemented;
	this->load_functions[0x01] = &MemoryController::load_not_implemented;
	this->stor_functions[0x02] = &MemoryController::store_not_implemented;
	this->load_functions[0x02] = &MemoryController::load_not_implemented;
	this->stor_functions[0x03] = &MemoryController::store_not_implemented;
	this->load_functions[0x03] = &MemoryController::load_not_implemented;
	this->stor_functions[0x04] = &MemoryController::store_not_implemented;
	this->load_functions[0x04] = &MemoryController::load_not_implemented;
	this->stor_functions[0x05] = &MemoryController::store_not_implemented;
	this->load_functions[0x05] = &MemoryController::load_not_implemented;
	this->stor_functions[0x06] = &MemoryController::store_not_implemented;
	this->load_functions[0x06] = &MemoryController::load_not_implemented;
	this->stor_functions[0x07] = &MemoryController::store_not_implemented;
	this->load_functions[0x07] = &MemoryController::load_not_implemented;
	this->stor_functions[0x08] = &MemoryController::store_not_implemented;
	this->load_functions[0x08] = &MemoryController::load_not_implemented;
	this->stor_functions[0x09] = &MemoryController::store_not_implemented;
	this->load_functions[0x09] = &MemoryController::load_not_implemented;
	this->stor_functions[0x0a] = &MemoryController::store_not_implemented;
	this->load_functions[0x0a] = &MemoryController::load_not_implemented;
	this->stor_functions[0x0b] = &MemoryController::store_not_implemented;
	this->load_functions[0x0b] = &MemoryController::load_not_implemented;
	this->stor_functions[0x0c] = &MemoryController::store_not_implemented;
	this->load_functions[0x0c] = &MemoryController::load_not_implemented;
	this->stor_functions[0x0d] = &MemoryController::store_not_implemented;
	this->load_functions[0x0d] = &MemoryController::load_not_implemented;
	this->stor_functions[0x0e] = &MemoryController::store_not_implemented;
	this->load_functions[0x0e] = &MemoryController::load_not_implemented;
	this->stor_functions[0x0f] = &MemoryController::store_IF;
	this->load_functions[0x0f] = &MemoryController::load_IF;

	//Audio:
	//this->stor_functions[0x10] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x10] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x11] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x11] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x12] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x12] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x13] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x13] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x14] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x14] = &MemoryController::load_not_implemented;

	//Unused:
	//this->stor_functions[0x15] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x15] = &MemoryController::load_not_implemented;

	//Audio:
	//this->stor_functions[0x16] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x16] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x17] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x17] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x18] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x18] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x19] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x19] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x1a] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x1a] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x1b] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x1b] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x1c] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x1c] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x1d] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x1d] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x1e] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x1e] = &MemoryController::load_not_implemented;

	//Unused:
	//this->stor_functions[0x1f] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x1f] = &MemoryController::load_not_implemented;

	//Audio:
	//this->stor_functions[0x20] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x20] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x21] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x21] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x22] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x22] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x23] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x23] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x24] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x24] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x25] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x25] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x26] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x26] = &MemoryController::load_not_implemented;

	this->stor_functions[0x27] = &MemoryController::store_not_implemented;
	this->load_functions[0x27] = &MemoryController::load_not_implemented;
	this->stor_functions[0x28] = &MemoryController::store_not_implemented;
	this->load_functions[0x28] = &MemoryController::load_not_implemented;
	this->stor_functions[0x29] = &MemoryController::store_not_implemented;
	this->load_functions[0x29] = &MemoryController::load_not_implemented;
	this->stor_functions[0x2a] = &MemoryController::store_not_implemented;
	this->load_functions[0x2a] = &MemoryController::load_not_implemented;
	this->stor_functions[0x2b] = &MemoryController::store_not_implemented;
	this->load_functions[0x2b] = &MemoryController::load_not_implemented;
	this->stor_functions[0x2c] = &MemoryController::store_not_implemented;
	this->load_functions[0x2c] = &MemoryController::load_not_implemented;
	this->stor_functions[0x2d] = &MemoryController::store_not_implemented;
	this->load_functions[0x2d] = &MemoryController::load_not_implemented;
	this->stor_functions[0x2e] = &MemoryController::store_not_implemented;
	this->load_functions[0x2e] = &MemoryController::load_not_implemented;
	this->stor_functions[0x2f] = &MemoryController::store_not_implemented;
	this->load_functions[0x2f] = &MemoryController::load_not_implemented;

	//Audio:
	//this->stor_functions[0x30] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x30] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x31] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x31] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x32] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x32] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x33] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x33] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x34] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x34] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x35] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x35] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x36] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x36] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x37] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x37] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x38] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x38] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x39] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x39] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x3a] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x3a] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x3b] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x3b] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x3c] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x3c] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x3d] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x3d] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x3e] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x3e] = &MemoryController::load_not_implemented;
	//this->stor_functions[0x3f] = &MemoryController::stor_not_implemented;
	//this->load_functions[0x3f] = &MemoryController::load_not_implemented;

	this->stor_functions[0x40] = &MemoryController::store_LCDC;
	this->load_functions[0x40] = &MemoryController::load_LCDC;
	this->stor_functions[0x41] = &MemoryController::store_STAT;
	this->load_functions[0x41] = &MemoryController::load_STAT;
	this->stor_functions[0x42] = &MemoryController::store_SCY;
	this->load_functions[0x42] = &MemoryController::load_SCY;
	this->stor_functions[0x43] = &MemoryController::store_SCX;
	this->load_functions[0x43] = &MemoryController::load_SCX;
	this->stor_functions[0x44] = &MemoryController::store_LY;
	this->load_functions[0x44] = &MemoryController::load_LY;
	this->stor_functions[0x45] = &MemoryController::store_LYC;
	this->load_functions[0x45] = &MemoryController::load_LYC;
	this->stor_functions[0x46] = &MemoryController::store_not_implemented;
	this->load_functions[0x46] = &MemoryController::load_not_implemented;
	this->stor_functions[0x47] = &MemoryController::store_BGP;
	this->load_functions[0x47] = &MemoryController::load_BGP;
	this->stor_functions[0x48] = &MemoryController::store_not_implemented;
	this->load_functions[0x48] = &MemoryController::load_not_implemented;
	this->stor_functions[0x49] = &MemoryController::store_not_implemented;
	this->load_functions[0x49] = &MemoryController::load_not_implemented;
	this->stor_functions[0x4a] = &MemoryController::store_WY;
	this->load_functions[0x4a] = &MemoryController::load_WY;
	this->stor_functions[0x4b] = &MemoryController::store_not_implemented;
	this->load_functions[0x4b] = &MemoryController::load_not_implemented;
}

void MemoryController::store_not_implemented(byte_t){
	throw NotImplementedException();
}

byte_t MemoryController::load_not_implemented() const{
	throw NotImplementedException();
}

byte_t MemoryController::load_STAT() const{
	return this->display->get_status();
}

void MemoryController::store_STAT(byte_t b){
	this->display->set_status(b);
}

byte_t MemoryController::load_LY() const{
	return this->display->get_y_coordinate();
}

void MemoryController::store_LY(byte_t b){
	this->display->set_y_coordinate_compare(b);
}

byte_t MemoryController::load_LYC() const{
	return this->display->get_y_coordinate_compare();
}

void MemoryController::store_LYC(byte_t b){
	this->display->set_y_coordinate_compare(b);
}

byte_t MemoryController::load_WY() const{
	return this->display->get_window_y_position();
}

void MemoryController::store_WY(byte_t b){
	this->display->set_window_y_position(b);
}

byte_t MemoryController::load_BGP() const{
	return this->display->get_background_palette();
}

void MemoryController::store_BGP(byte_t b){
	this->display->set_background_palette(b);
}

byte_t MemoryController::load_SCY() const{
	return this->display->get_scroll_y();
}

void MemoryController::store_SCY(byte_t b){
	this->display->set_scroll_y(b);
}

byte_t MemoryController::load_SCX() const{
	return this->display->get_scroll_x();
}

void MemoryController::store_SCX(byte_t b){
	this->display->set_scroll_x(b);
}

byte_t MemoryController::load_LCDC() const{
	return this->display->get_lcd_control();
}

void MemoryController::store_LCDC(byte_t b){
	this->display->set_lcd_control(b);
}

byte_t MemoryController::load_IF() const{
	return this->cpu->get_interrupt_flag();
}

void MemoryController::store_IF(byte_t b){
	this->cpu->set_interrupt_flag(b);
}

void MemoryController::fix_up_address(main_integer_t &address){
	address &= 0xFFFF;
	if (address >= 0xE000 && address < 0xFE00)
		address -= 0x2000;
}

main_integer_t MemoryController::load8(main_integer_t address) const{
	this->fix_up_address(address);

	if (address < 0xFF00)
		return this->memory[address];

	if (address < 0xFF00 + function_table_sizes){
		// Handle I/O registers.
		auto fp = this->load_functions[address & 0xFF];
		if (fp)
			return (this->*fp)();
		return this->memory[address];
	}

	if (address < 0xFF80)
		return 0xFF;

	return this->memory[address];
}

void MemoryController::store8(main_integer_t address, main_integer_t value){
	this->fix_up_address(address);
	auto b = (byte_t)value;
	this->memory[address] = b;

	if (address < 0xFF00)
		return;

	if (address < 0xFF00 + function_table_sizes){
		// Handle I/O registers.
		auto fp = this->stor_functions[address & 0xFF];
		if (fp)
			(this->*fp)(b);
		return;
	}

	if (address < 0xFF50)
		return;

	if (address == 0xFF50){
		this->toggle_boostrap_rom(!value);
		return;
	}
	return;
}

main_integer_t MemoryController::load16(main_integer_t address) const{
#if !defined USE_CPP_UNDEFINED_BEHAVIOR
	return (this->load8(address + 1) << 8) | this->load8(address);
#elif defined LITTLE_ENDIAN
	this->fix_up_address(address);
	return *(std::uint16_t *)(this->memory + address);
#else
#error Not implemented!
#endif
}

void MemoryController::store16(main_integer_t address, main_integer_t value){
#if !defined USE_CPP_UNDEFINED_BEHAVIOR
	this->store8(address + 1, value >> 8);
	this->store8(address, value);
#elif defined LITTLE_ENDIAN
	this->fix_up_address(address);
	*(std::uint16_t *)(this->memory + address) = (std::uint16_t)value;
#else
#error Not implemented!
#endif
}

void MemoryController::copy_out_memory_at(void *dst, size_t length, main_integer_t address){
	address &= 0xFFFF;
	length = std::min<size_t>(length, 0x10000 - address);
	memcpy(dst, this->memory + address, length);
}

void MemoryController::load_rom_at(const void *buffer, size_t length, main_integer_t address){
	address &= 0xFFFF;
	length = std::min<size_t>(length, 0x10000 - address);
	memcpy(this->memory + address, buffer, length);
}

void MemoryController::clear_memory_at(main_integer_t address, size_t length){
	address &= 0xFFFF;
	length = std::min<size_t>(length, 0x10000 - address);
	memset(this->memory + address, 0, length);
}

void MemoryController::toggle_boostrap_rom(bool on){
	if ((bool)this->boostrap_swap == on)
		return;
	if (on){
		this->boostrap_swap.reset(new byte_t[gb_bootstrap_rom_size]);
		memcpy(this->boostrap_swap.get(), this->memory, gb_bootstrap_rom_size);
		memcpy(this->memory, gb_bootstrap_rom, gb_bootstrap_rom_size);
	}else{
		memcpy(this->memory, this->boostrap_swap.get(), gb_bootstrap_rom_size);
		this->boostrap_swap.reset();
	}
}
