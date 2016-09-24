#include "MemoryController.h"
#include <cstdlib>
#include <exception>
#include <algorithm>

//#define USE_CPP_UNDEFINED_BEHAVIOR
#define LITTLE_ENDIAN

MemoryController::MemoryController(){
	this->memory = (unsigned char *)malloc(0x10000);
	if (!this->memory)
		throw std::bad_alloc();
}

MemoryController::~MemoryController(){
	free(this->memory);
}

void MemoryController::fix_up_address(unsigned &address){
	address &= 0xFFFF;
	if (address >= 0xE000 && address < 0xFE00)
		address -= 0x2000;
}

unsigned MemoryController::load8(unsigned address) const{
	this->fix_up_address(address);
	return this->memory[address];
}

void MemoryController::store8(unsigned address, unsigned value){
	this->fix_up_address(address);
	this->memory[address] = value & 0xFF;
}

unsigned MemoryController::load16(unsigned address) const{
#if !defined USE_CPP_UNDEFINED_BEHAVIOR
	auto plus_1 = address + 1;
	this->fix_up_address(address);
	this->fix_up_address(plus_1);
	return ((unsigned)this->memory[plus_1] << 8) | (unsigned)this->memory[address];
#elif
#error "Not yet implemented!"
#endif
}

void MemoryController::store16(unsigned address, unsigned value) const{
	auto plus_1 = address + 1;
	this->fix_up_address(address);
	this->fix_up_address(plus_1);
	value &= 0xFFFF;
	this->memory[plus_1] = value >> 8;
	this->memory[address] = value & 0xFF;
}

void MemoryController::copy_out_memory_at(void *dst, size_t length, unsigned address){
	address &= 0xFFFF;
	length = std::min<size_t>(length, 0x10000 - address);
	memcpy(dst, this->memory + address, length);
}

void MemoryController::load_rom_at(const void *buffer, size_t length, unsigned address){
	address &= 0xFFFF;
	length = std::min<size_t>(length, 0x10000 - address);
	memcpy(this->memory + address, buffer, length);
}

void MemoryController::clear_memory_at(unsigned address, size_t length){
	address &= 0xFFFF;
	length = std::min<size_t>(length, 0x10000 - address);
	memset(this->memory + address, 0, length);
}
