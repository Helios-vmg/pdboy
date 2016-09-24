#include "MemoryController.h"
#include <cstdlib>
#include <exception>
#include <algorithm>

#define USE_CPP_UNDEFINED_BEHAVIOR
#define LITTLE_ENDIAN

MemoryController::MemoryController(){
	this->memory = (unsigned char *)malloc(0x10000);
	if (!this->memory)
		throw std::bad_alloc();
}

MemoryController::~MemoryController(){
	free(this->memory);
}

void MemoryController::fix_up_address(integer_type &address){
	//address &= 0xFFFF;
	//if (address >= 0xE000 && address < 0xFE00)
	//	address -= 0x2000;
}

integer_type MemoryController::load8(integer_type address) const{
	this->fix_up_address(address);
	return this->memory[address];
}

void MemoryController::store8(integer_type address, integer_type value){
	this->fix_up_address(address);
	this->memory[address] = value /*& 0xFF*/;
}

integer_type MemoryController::load16(integer_type address) const{
#if !defined USE_CPP_UNDEFINED_BEHAVIOR
	integer_type plus_1 = address + (integer_type)1;
	this->fix_up_address(address);
	this->fix_up_address(plus_1);
	return ((integer_type)this->memory[plus_1] << 8) | (integer_type)this->memory[address];
#elif defined LITTLE_ENDIAN
	this->fix_up_address(address);
	return *(std::uint16_t *)(this->memory + address);
#else
#error Not implemented!
#endif
}

void MemoryController::store16(integer_type address, integer_type value) const{
#if !defined USE_CPP_UNDEFINED_BEHAVIOR
	integer_type plus_1 = address + (integer_type)1;
	this->fix_up_address(address);
	this->fix_up_address(plus_1);
	value &= 0xFFFF;
	this->memory[plus_1] = value >> 8;
	this->memory[address] = value & 0xFF;
#elif defined LITTLE_ENDIAN
	this->fix_up_address(address);
	*(std::uint16_t *)(this->memory + address) = (std::uint16_t)value;
#else
#error Not implemented!
#endif
}

void MemoryController::copy_out_memory_at(void *dst, size_t length, integer_type address){
	address &= 0xFFFF;
	length = std::min<size_t>(length, 0x10000 - address);
	memcpy(dst, this->memory + address, length);
}

void MemoryController::load_rom_at(const void *buffer, size_t length, integer_type address){
	address &= 0xFFFF;
	length = std::min<size_t>(length, 0x10000 - address);
	memcpy(this->memory + address, buffer, length);
}

void MemoryController::clear_memory_at(integer_type address, size_t length){
	address &= 0xFFFF;
	length = std::min<size_t>(length, 0x10000 - address);
	memset(this->memory + address, 0, length);
}
