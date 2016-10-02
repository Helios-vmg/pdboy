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

const size_t io_function_table_sizes = 0x4C;

MemoryController::MemoryController(Gameboy &system, GameboyCpu &cpu):
	system(&system),
	cpu(&cpu),
	display(&system.get_display_controller()),
	joypad(&system.get_input_controller()),
	storage(&system.get_storage_controller()),
	vram(0x2000),
	fixed_ram(0x1000),
	switchable_ram(0x7000),
	oam(0xA0),
	high_ram(0x80),
	io_registers_stor(new io_store_func_t[io_function_table_sizes]),
	io_registers_load(new io_load_func_t[io_function_table_sizes]),
	memory_map_store(new store_func_t[0x100]),
	memory_map_load(new load_func_t[0x100])
{}

void MemoryController::initialize(){
	this->display->set_memory_controller(*this);
	this->initialize_functions();
}

void MemoryController::initialize_functions(){
	this->initialize_memory_map_functions();
	this->initialize_io_register_functions();
}

#define MEMORY_RANGE_FOR(end) \
	old = accum; \
	accum = end; \
	for (unsigned i = old; i < end; i++)

void MemoryController::initialize_memory_map_functions(){
	unsigned accum = 0, old;
	//[0x0000; 0x8000)
	MEMORY_RANGE_FOR(0x80){
		this->memory_map_load[i] = &MemoryController::read_storage;
		this->memory_map_store[i] = &MemoryController::write_storage;
	}
	//[0x8000; 0xA000)
	MEMORY_RANGE_FOR(0xA0){
		this->memory_map_load[i] = &MemoryController::read_vram;
		this->memory_map_store[i] = &MemoryController::write_vram;
	}
	//[0xA000; 0xC000)
	MEMORY_RANGE_FOR(0xC0){
		this->memory_map_load[i] = &MemoryController::read_storage_ram;
		this->memory_map_store[i] = &MemoryController::write_storage_ram;
	}
	//[0xC000; 0xD000)
	MEMORY_RANGE_FOR(0xD0){
		this->memory_map_load[i] = &MemoryController::read_fixed_ram;
		this->memory_map_store[i] = &MemoryController::write_fixed_ram;
	}
	//[0xD000; 0xE000)
	MEMORY_RANGE_FOR(0xE0){
		this->memory_map_load[i] = &MemoryController::read_switchable_ram;
		this->memory_map_store[i] = &MemoryController::write_switchable_ram;
	}
	//[0xE000; 0xF000)
	MEMORY_RANGE_FOR(0xF0){
		this->memory_map_load[i] = &MemoryController::read_ram_mirror1;
		this->memory_map_store[i] = &MemoryController::write_ram_mirror1;
	}
	//[0xF000; 0xFE00)
	MEMORY_RANGE_FOR(0xFE){
		this->memory_map_load[i] = &MemoryController::read_ram_mirror2;
		this->memory_map_store[i] = &MemoryController::write_ram_mirror2;
	}
	//[0xFE00; 0xFF00)
	MEMORY_RANGE_FOR(0xFF){
		this->memory_map_load[i] = &MemoryController::read_oam;
		this->memory_map_store[i] = &MemoryController::write_oam;
	}
	//[0xFF00; 0xFFFF]
	MEMORY_RANGE_FOR(0x100){
		this->memory_map_load[i] = &MemoryController::read_io_registers_and_high_ram;
		this->memory_map_store[i] = &MemoryController::write_io_registers_and_high_ram;
	}
}

void MemoryController::initialize_io_register_functions(){
	std::fill(this->io_registers_stor.get(), this->io_registers_stor.get() + io_function_table_sizes, nullptr);
	std::fill(this->io_registers_load.get(), this->io_registers_load.get() + io_function_table_sizes, nullptr);

	this->io_registers_stor[0x00] = &MemoryController::store_P1;
	this->io_registers_load[0x00] = &MemoryController::load_P1;
	//Serial I/O (SB)
	//this->io_registers_stor[0x01] = &MemoryController::store_not_implemented;
	//this->io_registers_load[0x01] = &MemoryController::load_not_implemented;
	//Serial I/O control (SC)
	//this->io_registers_stor[0x02] = &MemoryController::store_not_implemented;
	//this->io_registers_load[0x02] = &MemoryController::load_not_implemented;
	this->io_registers_stor[0x03] = &MemoryController::store_not_implemented;
	this->io_registers_load[0x03] = &MemoryController::load_not_implemented;
	this->io_registers_stor[0x04] = &MemoryController::store_DIV;
	this->io_registers_load[0x04] = &MemoryController::load_DIV;
	this->io_registers_stor[0x05] = &MemoryController::store_TIMA;
	this->io_registers_load[0x05] = &MemoryController::load_TIMA;
	this->io_registers_stor[0x06] = &MemoryController::store_TMA;
	this->io_registers_load[0x06] = &MemoryController::load_TMA;
	this->io_registers_stor[0x07] = &MemoryController::store_TAC;
	this->io_registers_load[0x07] = &MemoryController::load_TAC;
	this->io_registers_stor[0x08] = &MemoryController::store_not_implemented;
	this->io_registers_load[0x08] = &MemoryController::load_not_implemented;
	this->io_registers_stor[0x09] = &MemoryController::store_not_implemented;
	this->io_registers_load[0x09] = &MemoryController::load_not_implemented;
	this->io_registers_stor[0x0a] = &MemoryController::store_not_implemented;
	this->io_registers_load[0x0a] = &MemoryController::load_not_implemented;
	this->io_registers_stor[0x0b] = &MemoryController::store_not_implemented;
	this->io_registers_load[0x0b] = &MemoryController::load_not_implemented;
	this->io_registers_stor[0x0c] = &MemoryController::store_not_implemented;
	this->io_registers_load[0x0c] = &MemoryController::load_not_implemented;
	this->io_registers_stor[0x0d] = &MemoryController::store_not_implemented;
	this->io_registers_load[0x0d] = &MemoryController::load_not_implemented;
	this->io_registers_stor[0x0e] = &MemoryController::store_not_implemented;
	this->io_registers_load[0x0e] = &MemoryController::load_not_implemented;
	this->io_registers_stor[0x0f] = &MemoryController::store_IF;
	this->io_registers_load[0x0f] = &MemoryController::load_IF;

	//Audio:
	//this->io_registers_stor[0x10] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x10] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x11] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x11] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x12] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x12] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x13] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x13] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x14] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x14] = &MemoryController::load_not_implemented;

	//Unused:
	//this->io_registers_stor[0x15] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x15] = &MemoryController::load_not_implemented;

	//Audio:
	//this->io_registers_stor[0x16] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x16] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x17] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x17] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x18] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x18] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x19] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x19] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x1a] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x1a] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x1b] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x1b] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x1c] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x1c] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x1d] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x1d] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x1e] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x1e] = &MemoryController::load_not_implemented;

	//Unused:
	//this->io_registers_stor[0x1f] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x1f] = &MemoryController::load_not_implemented;

	//Audio:
	//this->io_registers_stor[0x20] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x20] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x21] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x21] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x22] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x22] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x23] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x23] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x24] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x24] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x25] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x25] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x26] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x26] = &MemoryController::load_not_implemented;

	this->io_registers_stor[0x27] = &MemoryController::store_not_implemented;
	this->io_registers_load[0x27] = &MemoryController::load_not_implemented;
	this->io_registers_stor[0x28] = &MemoryController::store_not_implemented;
	this->io_registers_load[0x28] = &MemoryController::load_not_implemented;
	this->io_registers_stor[0x29] = &MemoryController::store_not_implemented;
	this->io_registers_load[0x29] = &MemoryController::load_not_implemented;
	this->io_registers_stor[0x2a] = &MemoryController::store_not_implemented;
	this->io_registers_load[0x2a] = &MemoryController::load_not_implemented;
	this->io_registers_stor[0x2b] = &MemoryController::store_not_implemented;
	this->io_registers_load[0x2b] = &MemoryController::load_not_implemented;
	this->io_registers_stor[0x2c] = &MemoryController::store_not_implemented;
	this->io_registers_load[0x2c] = &MemoryController::load_not_implemented;
	this->io_registers_stor[0x2d] = &MemoryController::store_not_implemented;
	this->io_registers_load[0x2d] = &MemoryController::load_not_implemented;
	this->io_registers_stor[0x2e] = &MemoryController::store_not_implemented;
	this->io_registers_load[0x2e] = &MemoryController::load_not_implemented;
	this->io_registers_stor[0x2f] = &MemoryController::store_not_implemented;
	this->io_registers_load[0x2f] = &MemoryController::load_not_implemented;

	//Audio:
	//this->io_registers_stor[0x30] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x30] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x31] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x31] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x32] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x32] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x33] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x33] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x34] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x34] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x35] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x35] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x36] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x36] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x37] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x37] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x38] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x38] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x39] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x39] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x3a] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x3a] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x3b] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x3b] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x3c] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x3c] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x3d] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x3d] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x3e] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x3e] = &MemoryController::load_not_implemented;
	//this->io_registers_stor[0x3f] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x3f] = &MemoryController::load_not_implemented;

	this->io_registers_stor[0x40] = &MemoryController::store_LCDC;
	this->io_registers_load[0x40] = &MemoryController::load_LCDC;
	this->io_registers_stor[0x41] = &MemoryController::store_STAT;
	this->io_registers_load[0x41] = &MemoryController::load_STAT;
	this->io_registers_stor[0x42] = &MemoryController::store_SCY;
	this->io_registers_load[0x42] = &MemoryController::load_SCY;
	this->io_registers_stor[0x43] = &MemoryController::store_SCX;
	this->io_registers_load[0x43] = &MemoryController::load_SCX;
	this->io_registers_stor[0x44] = &MemoryController::store_LY;
	this->io_registers_load[0x44] = &MemoryController::load_LY;
	this->io_registers_stor[0x45] = &MemoryController::store_LYC;
	this->io_registers_load[0x45] = &MemoryController::load_LYC;
	this->io_registers_stor[0x46] = &MemoryController::store_DMA;
	//this->io_registers_load[0x46] = &MemoryController::load_DMA;
	this->io_registers_stor[0x47] = &MemoryController::store_BGP;
	this->io_registers_load[0x47] = &MemoryController::load_BGP;
	this->io_registers_stor[0x48] = &MemoryController::store_OBP0;
	this->io_registers_load[0x48] = &MemoryController::load_OBP0;
	this->io_registers_stor[0x49] = &MemoryController::store_OBP1;
	this->io_registers_load[0x49] = &MemoryController::load_OBP1;
	this->io_registers_stor[0x4a] = &MemoryController::store_WY;
	this->io_registers_load[0x4a] = &MemoryController::load_WY;
	this->io_registers_stor[0x4b] = &MemoryController::store_WX;
	this->io_registers_load[0x4b] = &MemoryController::load_WX;
}

byte_t MemoryController::read_storage(main_integer_t address) const{
	return this->storage->read8(address);
}

void MemoryController::write_storage(main_integer_t address, byte_t value){
	this->storage->write8(address, value);
}

byte_t MemoryController::read_storage_ram(main_integer_t address) const{
	return this->read_storage(address);
}

void MemoryController::write_storage_ram(main_integer_t address, byte_t value){
	return this->write_storage(address, value);
}

byte_t MemoryController::read_ram_mirror1(main_integer_t address) const{
	return this->read_fixed_ram(address - 0x2000);
}

void MemoryController::write_ram_mirror1(main_integer_t address, byte_t value){
	this->write_fixed_ram(address - 0x2000, value);
}

byte_t MemoryController::read_ram_mirror2(main_integer_t address) const{
	return this->read_switchable_ram(address - 0x2000);
}

void MemoryController::write_ram_mirror2(main_integer_t address, byte_t value){
	this->write_switchable_ram(address - 0x2000, value);
}

byte_t MemoryController::read_io_registers_and_high_ram(main_integer_t address) const{
	if (address < 0xFF00 + io_function_table_sizes){
		// Handle I/O registers.
		auto fp = this->io_registers_load[address & 0xFF];
		if (fp)
			return (this->*fp)();
		throw NotImplementedException();
	}
	if (address < 0xFF80)
		throw NotImplementedException();
	// Handle high RAM.
	return this->high_ram.access(address);
}

void MemoryController::write_io_registers_and_high_ram(main_integer_t address, byte_t value){
	if (address < 0xFF00 + io_function_table_sizes){
		// Handle I/O registers.
		auto fp = this->io_registers_stor[address & 0xFF];
		if (fp)
			(this->*fp)(value);
		throw NotImplementedException();
	}
	if (address < 0xFF80)
		throw NotImplementedException();
	// Handle high RAM.
	this->high_ram.access(address) = value;
}

byte_t MemoryController::read_dmg_bootstrap(main_integer_t address) const{
	return gb_bootstrap_rom[address];
}

byte_t MemoryController::read_vram(main_integer_t address) const{
	return this->vram.access(address);
}

void MemoryController::write_vram(main_integer_t address, byte_t value){
	this->vram.access(address) = value;
}

byte_t MemoryController::read_fixed_ram(main_integer_t address) const{
	return this->fixed_ram.access(address);
}

void MemoryController::write_fixed_ram(main_integer_t address, byte_t value){
	this->fixed_ram.access(address) = value;
}

byte_t MemoryController::read_switchable_ram(main_integer_t address) const{
	return this->fixed_ram.access(address + (this->selected_ram_bank << 12));
}

void MemoryController::write_switchable_ram(main_integer_t address, byte_t value){
	this->fixed_ram.access(address + (this->selected_ram_bank << 12)) = value;
}

byte_t MemoryController::read_oam(main_integer_t address) const{
	if (address >= 0xFEA0)
		throw GenericException("Invalid memory access: Region [0xFEA0; 0xFF00) cannot be used.");

	return this->oam.access(address);
}

void MemoryController::write_oam(main_integer_t address, byte_t value){
	if (address >= 0xFEA0)
		throw GenericException("Invalid memory access: Region [0xFEA0; 0xFF00) cannot be used.");

	this->oam.access(address) = value;
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

byte_t MemoryController::load_WX() const{
	return this->display->get_window_x_position();
}

void MemoryController::store_WX(byte_t b){
	this->display->set_window_x_position(b);
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

byte_t MemoryController::load_P1() const{
	return this->joypad->get_requested_input_state();
}

void MemoryController::store_P1(byte_t b){
	this->joypad->request_input_state(b);
}

byte_t MemoryController::load_OBP0() const{
	return this->display->get_obj0_palette();
}

void MemoryController::store_OBP0(byte_t b){
	this->display->set_obj0_palette(b);
}

byte_t MemoryController::load_OBP1() const{
	return this->display->get_obj1_palette();
}

void MemoryController::store_OBP1(byte_t b){
	this->display->set_obj1_palette(b);
}

byte_t MemoryController::load_DMA() const{
	return 0;
}

void MemoryController::store_DMA(byte_t b){
	this->cpu->begin_dmg_dma_transfer(b);
}

byte_t MemoryController::load_DIV() const{
	return this->system->get_system_clock().get_DIV_register();
}

void MemoryController::store_DIV(byte_t b){
	this->system->get_system_clock().reset_DIV_register();
}

byte_t MemoryController::load_TIMA() const{
	return this->system->get_system_clock().get_TIMA_register();
}

void MemoryController::store_TIMA(byte_t b){
	this->system->get_system_clock().set_TIMA_register(b);
}

byte_t MemoryController::load_TMA() const{
	return this->system->get_system_clock().get_TMA_register();
}

void MemoryController::store_TMA(byte_t b){
	this->system->get_system_clock().set_TMA_register(b);
}

byte_t MemoryController::load_TAC() const{
	return this->system->get_system_clock().get_TAC_register();
}

void MemoryController::store_TAC(byte_t b){
	this->system->get_system_clock().set_TAC_register(b);
}

main_integer_t MemoryController::load8(main_integer_t address) const{
	address &= 0xFFFF;
	auto fp = this->memory_map_load[address >> 8];
	return (this->*fp)(address);
}

void MemoryController::store8(main_integer_t address, main_integer_t value){
	address &= 0xFFFF;
	auto fp = this->memory_map_store[address >> 8];
	(this->*fp)(address, (byte_t)value);
}

main_integer_t MemoryController::load16(main_integer_t address) const{
	return (this->load8(address + 1) << 8) | this->load8(address);
}

void MemoryController::store16(main_integer_t address, main_integer_t value){
	this->store8(address + 1, value >> 8);
	this->store8(address, value);
}

void MemoryController::copy_memory(main_integer_t src, main_integer_t dst, size_t length){
	if (src == dst)
		return;
	if (dst > src && dst < src + length || src > dst && src < dst + length)
		throw GenericException("Invalid memory transfer: source and destination overlap.");

	for (size_t i = 0; i < length; i++)
		this->store8(dst + i, this->load8(src + i));
}

void MemoryController::toggle_boostrap_rom(bool on){
	if (this->boostrap_enabled == on)
		return;
	this->boostrap_enabled = on;
	if (on)
		this->memory_map_load[0x00] = &MemoryController::read_dmg_bootstrap;
	else
		this->memory_map_load[0x00] = &MemoryController::read_storage;
}
