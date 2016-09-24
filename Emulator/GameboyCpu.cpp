#include "GameboyCpu.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <iomanip>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

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

void GameboyCpu::initialize(){
	this->initialize_opcode_tables();
	{
		std::ifstream input("tetris.gb", std::ios::binary);
		if (!input)
			throw std::exception();
		input.seekg(0, std::ios::end);
		std::vector<char> data(input.tellg());
		input.seekg(0);
		input.read(&data[0], data.size());
		this->memory_controller.load_rom_at(&data[0], data.size(), 0);
	}
	char temp[0x100];
	this->memory_controller.copy_out_memory_at(temp, sizeof(temp), 0);
	this->memory_controller.load_rom_at(gb_bootstrap_rom, gb_bootstrap_rom_size, 0);
	this->break_on_address = 0x66;
	this->run();
	this->break_on_address = -1;
	this->memory_controller.load_rom_at(temp, sizeof(temp), 0);
}

void GameboyCpu::take_time(unsigned cycles){
	this->total_cycles += cycles;
}

void GameboyCpu::interrupt_toggle(bool enable){
	
}

void GameboyCpu::stop(){
	
}

void GameboyCpu::halt(){
	
}

void GameboyCpu::abort(){
	this->running = false;
}

void GameboyCpu::run(){
	this->running = true;
	while (this->running){
		auto pc = this->registers.get(Register16::PC);
		this->current_pc = pc;
		if (this->break_on_address >= 0 && pc >= this->break_on_address){
			this->running = false;
			break;
		}
		auto opcode = this->memory_controller.load8(pc);
		this->registers.set(Register16::PC, pc + 1);
		auto function_pointer = this->opcode_table[opcode];
		(this->*function_pointer)();
	}
}
