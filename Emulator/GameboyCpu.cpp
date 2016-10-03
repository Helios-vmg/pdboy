#include "GameboyCpu.h"
#include "Gameboy.h"
#include "exceptions.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>

GameboyCpu::GameboyCpu(Gameboy &system):
		registers(*this),
		memory_controller(*this->system, *this),
		system(&system){
}

void GameboyCpu::initialize(){
	this->memory_controller.initialize();
	this->initialize_opcode_tables();
	this->memory_controller.toggle_boostrap_rom(true);
}

void GameboyCpu::take_time(std::uint32_t cycles){
	this->system->get_system_clock().advance_clock(cycles);
}

void GameboyCpu::interrupt_toggle(bool enable){
	this->interrupts_enabled = enable;
}

void GameboyCpu::stop(){
	throw NotImplementedException();
}

void GameboyCpu::halt(){
	throw NotImplementedException();
}

void GameboyCpu::abort(){
	throw GenericException("Gameboy program executed an illegal operation.");
}

byte_t GameboyCpu::load_pc_and_increment(){
	return (byte_t)this->memory_controller.load8(this->registers.pc()++);
}

#define BREAKPOINT(x) if (this->current_pc == x) __debugbreak()

void GameboyCpu::run_one_instruction(){
	this->current_pc = this->registers.pc();
	auto opcode = this->load_pc_and_increment();

	auto function_pointer = this->opcode_table[opcode];

	(this->*function_pointer)();
	this->total_instructions++;

	this->check_timer();
	this->perform_dmg_dma();
	this->attempt_to_handle_interrupts();
}

main_integer_t GameboyCpu::decimal_adjust(main_integer_t value){
	value &= 0xFF;
	if (!this->registers.get(Flags::Subtract)){
		if (this->registers.get(Flags::HalfCarry) || (value & 0x0F) > 9)
			value += 6;

		if (this->registers.get(Flags::Carry) || value > 0x9F)
			value += 0x60;
	}else{
		if (this->registers.get(Flags::HalfCarry))
			value = (value - 6) & 0xFF;

		if (this->registers.get(Flags::Carry))
			value -= 0x60;
	}
	return value;
}

void GameboyCpu::vblank_irq(){
	this->interrupt_flag |= (1 << this->vblank_flag_bit);
	this->attempt_to_handle_interrupts();
}

void GameboyCpu::attempt_to_handle_interrupts(){
	if (!this->interrupts_enabled)
		return;
	for (int i = 0; i < 5; i++){
		auto mask = 1 << i;
		if (!(this->interrupt_flag & mask) || !(this->interrupt_enable_flag & mask))
			continue;
		main_integer_t stack_pointer = this->registers.sp();
		main_integer_t new_stack_pointer = stack_pointer - 2;
		this->registers.sp() = (std::uint16_t)new_stack_pointer;
		main_integer_t program_counter = this->registers.pc();
		this->memory_controller.store16(new_stack_pointer, program_counter);
		this->registers.pc() = (std::uint16_t)(0x0040 + i * 8);
		this->interrupt_flag &= ~mask;
		this->interrupt_toggle(false);
		this->take_time(4 * 5);
	}
}

byte_t GameboyCpu::get_interrupt_flag() const{
	return this->interrupt_flag;
}

void GameboyCpu::set_interrupt_flag(byte_t b){
	this->interrupt_flag = b;
}

byte_t GameboyCpu::get_interrupt_enable_flag() const{
	return this->interrupt_enable_flag;
}

void GameboyCpu::set_interrupt_enable_flag(byte_t b){
	this->interrupt_enable_flag = b;
}

void GameboyCpu::begin_dmg_dma_transfer(byte_t position){
	this->dma_scheduled = position;
}

void GameboyCpu::perform_dmg_dma(){
	if (this->dma_scheduled < 0)
		return;
	this->memory_controller.copy_memory(this->dma_scheduled << 8, 0xFE00, 0xA0);
	this->dma_scheduled = -1;
	this->last_dma_at = this->system->get_system_clock().get_clock_value();
}

void GameboyCpu::check_timer(){
	if (this->system->get_system_clock().get_trigger_interrupt())
		this->interrupt_flag |= this->timer_mask;
}
