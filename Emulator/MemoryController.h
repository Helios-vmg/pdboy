#pragma once
#include "RegisterStore.h"
#include "CommonTypes.h"
#include "MemorySection.h"
#include <memory>

class GameboyCpu;
class DisplayController;

#define DECLARE_IO_REGISTER(x) \
	byte_t load_##x(main_integer_t) const; \
	void store_##x(main_integer_t, byte_t)

class Gameboy;
class UserInputController;
class StorageController;

class MemoryController{
	typedef void (MemoryController::*store_func_t)(main_integer_t, byte_t);
	typedef byte_t(MemoryController::*load_func_t)(main_integer_t) const;

	Gameboy *system;
	GameboyCpu *cpu;
	DisplayController *display;
	UserInputController *joypad;
	StorageController *storage;

	MemorySection<0xC000> fixed_ram;
	MemorySection<0xD000> switchable_ram;
	MemorySection<0xFF80> high_ram;

	std::unique_ptr<store_func_t[]> io_registers_stor;
	std::unique_ptr<load_func_t[]> io_registers_load;
	std::unique_ptr<store_func_t[]> memory_map_store;
	std::unique_ptr<load_func_t[]> memory_map_load;
	bool boostrap_enabled = false;

	unsigned selected_ram_bank = 0;

	void initialize_functions();
	void initialize_memory_map_functions();
	void initialize_io_register_functions();
	void store_nothing(main_integer_t, byte_t);
	byte_t load_nothing(main_integer_t) const;
	void store_not_implemented(main_integer_t, byte_t);
	byte_t load_not_implemented(main_integer_t) const;
	void store_no_io(main_integer_t, byte_t);
	byte_t load_no_io(main_integer_t) const;
	void store_bootstrap_rom_enable(main_integer_t, byte_t);
	byte_t load_bootstrap_rom_enable(main_integer_t) const;
	void store_high_ram(main_integer_t, byte_t);
	byte_t load_high_ram(main_integer_t) const;
	void store_interrupt_enable(main_integer_t, byte_t);
	byte_t load_interrupt_enable(main_integer_t) const;

	byte_t read_storage(main_integer_t) const;
	void write_storage(main_integer_t, byte_t);
	byte_t read_storage_ram(main_integer_t) const;
	void write_storage_ram(main_integer_t, byte_t);
	byte_t read_ram_mirror1(main_integer_t) const;
	void write_ram_mirror1(main_integer_t, byte_t);
	byte_t read_ram_mirror2(main_integer_t) const;
	void write_ram_mirror2(main_integer_t, byte_t);
	byte_t read_io_registers_and_high_ram(main_integer_t) const;
	void write_io_registers_and_high_ram(main_integer_t, byte_t);
	byte_t read_dmg_bootstrap(main_integer_t) const;

	//Real RAM:
	byte_t read_vram(main_integer_t) const;
	void write_vram(main_integer_t, byte_t);
	byte_t read_fixed_ram(main_integer_t) const;
	void write_fixed_ram(main_integer_t, byte_t);
	byte_t read_switchable_ram(main_integer_t) const;
	void write_switchable_ram(main_integer_t, byte_t);
	byte_t read_oam(main_integer_t) const;
	void write_oam(main_integer_t, byte_t);


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
	void copy_memory(main_integer_t src, main_integer_t dst, size_t length);
	void toggle_boostrap_rom(bool);
	bool get_boostrap_enabled() const{
		return this->boostrap_enabled;
	}
};
