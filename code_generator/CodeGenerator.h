#pragma once
#include <memory>
#include "BasicDefinitions.h"

class CpuDefinition;
struct FlagSettings;

class CodeGenerator{
protected:
	std::shared_ptr<CpuDefinition> definition;

	virtual void begin_opcode_definition(unsigned first) = 0;
	virtual void end_opcode_definition(unsigned first) = 0;
	virtual void begin_double_opcode_definition(unsigned first, unsigned second) = 0;
	virtual void end_double_opcode_definition(unsigned first, unsigned second) = 0;
public:
	CodeGenerator(std::shared_ptr<CpuDefinition> definition): definition(definition){}
	virtual ~CodeGenerator() = 0{}
	void generate();
	void double_opcode(unsigned first);

	virtual void noop() = 0;
	virtual void halt() = 0;
	virtual uintptr_t load_program_counter8() = 0;
	virtual uintptr_t load_program_counter16() = 0;
	virtual uintptr_t get_register_value8(Register8) = 0;
	uintptr_t get_A(){
		return this->get_register_value8(Register8::A);
	}
	virtual uintptr_t get_register_value16(Register16) = 0;
	uintptr_t get_register_value16(Register16A reg){
		return this->get_register_value16(to_Register16(reg));
	}
	uintptr_t get_register_value16(Register16B reg){
		return this->get_register_value16(to_Register16(reg));
	}
	virtual uintptr_t load_hl8() = 0;
	virtual uintptr_t load_mem8(uintptr_t) = 0;
	virtual uintptr_t load_mem16(uintptr_t) = 0;
	virtual uintptr_t load_ff00_offset8(uintptr_t) = 0;
	virtual uintptr_t load_sp_offset16(uintptr_t) = 0;
	virtual void write_register8(Register8, uintptr_t) = 0;
	virtual void write_register16_literal(Register16, unsigned) = 0;
	virtual void write_register16(Register16, uintptr_t) = 0;
	void write_register16(Register16A reg, uintptr_t val){
		this->write_register16(to_Register16(reg), val);
	}
	void write_A(uintptr_t x){
		this->write_register8(Register8::A, x);
	}
	virtual void store_hl8(uintptr_t) = 0;
	virtual void store_mem8(uintptr_t mem, uintptr_t val) = 0;
	virtual void store_mem16(uintptr_t mem, uintptr_t val) = 0;
	virtual void store_mem_ff00_8(uintptr_t mem, uintptr_t val) = 0;
	virtual void take_time(unsigned) = 0;
	virtual void zero_flags() = 0;
	virtual void dec_register8(Register8 reg) = 0;
	virtual void dec_register16(Register16 reg) = 0;
	void dec_register16(Register16A reg){
		this->dec_register16(to_Register16(reg));
	}
	virtual void dec2_SP() = 0;
	virtual void inc2_SP() = 0;
	virtual void inc_register8(Register8 hl) = 0;
	virtual void inc_register16(Register16 hl) = 0;
	void inc_register16(Register16A reg){
		this->inc_register16(to_Register16(reg));
	}
	virtual std::array<uintptr_t, 3> add8(uintptr_t) = 0;
	virtual std::array<uintptr_t, 3> add_carry(uintptr_t) = 0;
	virtual std::array<uintptr_t, 3> sub(uintptr_t) = 0;
	virtual std::array<uintptr_t, 3> sub_carry(uintptr_t) = 0;
	virtual uintptr_t and (uintptr_t) = 0;
	virtual uintptr_t xor (uintptr_t) = 0;
	virtual uintptr_t or (uintptr_t) = 0;
	virtual std::array<uintptr_t, 3> cmp(uintptr_t) = 0;
	virtual void set_flags(const FlagSettings &) = 0;
	virtual uintptr_t inc_temp(uintptr_t) = 0;
	virtual uintptr_t dec_temp(uintptr_t) = 0;
	virtual void add16(Register16) = 0;
	void add16(Register16A reg){
		this->add16(to_Register16(reg));
	}
	virtual void add_SP(uintptr_t) = 0;
	virtual void flip_A() = 0;
	virtual void disable_interrupts() = 0;
	virtual void enable_interrupts() = 0;
	virtual void rotate_A(bool left, bool through_carry) = 0;
	virtual void add_PC(uintptr_t imm) = 0;
	virtual void set_PC_if(uintptr_t, ConditionalJumpType) = 0;
	virtual void add_PC_if(uintptr_t, ConditionalJumpType) = 0;
	void push_PC(){
		this->dec2_SP();
		auto sp = this->get_register_value16(Register16::SP);
		auto old_pc = this->get_register_value16(Register16::PC);
		this->store_mem16(sp, old_pc);
	}

	virtual void stop() = 0;
	virtual uintptr_t rotate_left(uintptr_t val) = 0;
	virtual uintptr_t rotate_right(uintptr_t val) = 0;
	virtual uintptr_t rotate_left_through_carry(uintptr_t val) = 0;
	virtual uintptr_t rotate_right_through_carry(uintptr_t val) = 0;
	virtual uintptr_t shift_left(uintptr_t val) = 0;
	virtual uintptr_t arithmetic_shift_right(uintptr_t val) = 0;
	virtual uintptr_t bitwise_shift_right(uintptr_t val) = 0;
	virtual uintptr_t get_bit_value(uintptr_t val, unsigned bit) = 0;
	virtual uintptr_t set_bit_value(uintptr_t val, unsigned bit, bool on) = 0;
	virtual std::pair<uintptr_t, uintptr_t> perform_decimal_adjustment(uintptr_t val) = 0;
	virtual uintptr_t swap_nibbles(uintptr_t val) = 0;
};