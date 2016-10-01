#pragma once
#include "RegisterStore.h"
#include "CommonTypes.h"
#include <memory>

class GameboyCpu;
class DisplayController;

#define DECLARE_IO_REGISTER(x) \
	byte_t load_##x() const; \
	void store_##x(byte_t)

class Gameboy;
class UserInputController;

class MemoryController{
	Gameboy *system;
	GameboyCpu *cpu;
	DisplayController *display;
	UserInputController *joypad;
	std::unique_ptr<byte_t[]> memoryp;
	byte_t *memory;
	static void fix_up_address(main_integer_t &address);
	typedef void (MemoryController::*store_func_t)(byte_t);
	typedef byte_t (MemoryController::*load_func_t)() const;
	std::unique_ptr<store_func_t[]> stor_functions;
	std::unique_ptr<load_func_t[]> load_functions;
	std::unique_ptr<byte_t[]> boostrap_swap;

	void initialize_functions();
	void store_not_implemented(byte_t);
	byte_t load_not_implemented() const;

	DECLARE_IO_REGISTER(STAT);
	DECLARE_IO_REGISTER(LY);
	DECLARE_IO_REGISTER(LYC);
	DECLARE_IO_REGISTER(WX);
	DECLARE_IO_REGISTER(WY);
	DECLARE_IO_REGISTER(BGP);
	DECLARE_IO_REGISTER(SCY);
	DECLARE_IO_REGISTER(SCX);
	DECLARE_IO_REGISTER(LCDC);
	DECLARE_IO_REGISTER(IF);
	DECLARE_IO_REGISTER(P1);
	DECLARE_IO_REGISTER(OBP0);
	DECLARE_IO_REGISTER(OBP1);
	DECLARE_IO_REGISTER(DMA);
	DECLARE_IO_REGISTER(DIV);
	DECLARE_IO_REGISTER(TIMA);
	DECLARE_IO_REGISTER(TMA);
	DECLARE_IO_REGISTER(TAC);
public:
	MemoryController(Gameboy &, GameboyCpu &);
	void initialize();
	main_integer_t load8(main_integer_t address) const;
	void store8(main_integer_t address, main_integer_t value);
	main_integer_t load16(main_integer_t address) const;
	void store16(main_integer_t address, main_integer_t value);
	void copy_out_memory_at(void *dst, size_t length, main_integer_t address);
	void load_rom_at(const void *buffer, size_t length, main_integer_t address);
	void clear_memory_at(main_integer_t address, size_t length);
	void copy_memory(main_integer_t src, main_integer_t dst, size_t length);
	void toggle_boostrap_rom(bool);
	byte_t *direct_memory_access(main_integer_t address){
		return this->memory + address;
	}
};
