#pragma once
#include <memory>
#include "BasicDefinitions.h"
#include <array>

class CpuDefinition;
struct FlagSettings;

class CodeGenerator{
protected:
	std::shared_ptr<CpuDefinition> definition;

	virtual void begin_opcode_definition(unsigned first) = 0;
	virtual void end_opcode_definition(unsigned first) = 0;
	virtual void begin_double_opcode_definition(unsigned first, unsigned second) = 0;
	virtual void end_double_opcode_definition(unsigned first, unsigned second) = 0;
	virtual void opcode_cb_branching() = 0;
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
	void dec_register16(Register16 reg){
		auto val = this->get_register_value16(reg);
		val = this->minus_1(val);
		this->write_register16(reg, val);
	}
	void dec2_SP(){
		auto val = this->get_register_value16(Register16::SP);
		auto imm = this->get_imm_value(2);
		val = this->sub16_no_carry(val, imm);
		this->write_register16(Register16::SP, val);
	}
	void inc2_SP(){
		auto val = this->get_register_value16(Register16::SP);
		auto imm = this->get_imm_value(2);
		val = this->add16(val, imm)[0];
		this->write_register16(Register16::SP, val);
	}
	void inc_register16(Register16 reg){
		auto val = this->get_register_value16(reg);
		val = this->plus_1(val);
		this->write_register16(reg, val);
	}
	virtual std::array<uintptr_t, 3> add8(uintptr_t, uintptr_t) = 0;
	virtual std::array<uintptr_t, 3> add16_using_carry_modulo_16(uintptr_t valA, uintptr_t valB) = 0;
	virtual std::array<uintptr_t, 3> add8_carry(uintptr_t, uintptr_t) = 0;
	virtual std::array<uintptr_t, 3> sub8(uintptr_t, uintptr_t) = 0;
	virtual std::array<uintptr_t, 3> sub8_carry(uintptr_t, uintptr_t) = 0;
	virtual uintptr_t sub16_no_carry(uintptr_t, uintptr_t) = 0;
	virtual uintptr_t and8(uintptr_t, uintptr_t) = 0;
	virtual uintptr_t xor8(uintptr_t, uintptr_t) = 0;
	virtual uintptr_t or8(uintptr_t, uintptr_t) = 0;
	virtual std::array<uintptr_t, 3> cmp8(uintptr_t, uintptr_t) = 0;
	virtual void set_flags(const FlagSettings &) = 0;
	virtual uintptr_t plus_1(uintptr_t) = 0;
	virtual uintptr_t minus_1(uintptr_t) = 0;
	virtual std::array<uintptr_t, 3> add16(uintptr_t valA, uintptr_t valB) = 0;
	virtual uintptr_t bitwise_not(uintptr_t) = 0;
	virtual void disable_interrupts() = 0;
	virtual void enable_interrupts() = 0;
	virtual uintptr_t rotate8(uintptr_t, bool left, bool through_carry) = 0;
	virtual void set_PC_if(uintptr_t, ConditionalJumpType) = 0;
	virtual void add8_PC_if(uintptr_t, ConditionalJumpType) = 0;
	void push_PC(){
		this->dec2_SP();
		auto sp = this->get_register_value16(Register16::SP);
		auto old_pc = this->get_register_value16(Register16::PC);
		this->store_mem16(sp, old_pc);
	}

	virtual void stop() = 0;
	virtual uintptr_t shift_left(uintptr_t val) = 0;
	virtual uintptr_t arithmetic_shift_right(uintptr_t val) = 0;
	virtual uintptr_t bitwise_shift_right(uintptr_t val) = 0;
	virtual uintptr_t get_bit_value(uintptr_t val, unsigned bit) = 0;
	virtual uintptr_t set_bit_value(uintptr_t val, unsigned bit, bool on) = 0;
	virtual std::pair<uintptr_t, uintptr_t> perform_decimal_adjustment(uintptr_t val) = 0;
	virtual uintptr_t swap_nibbles(uintptr_t val) = 0;
	virtual uintptr_t get_imm_value(unsigned val) = 0;
	virtual void require_equals(uintptr_t,uintptr_t) = 0;
	virtual void do_nothing_if(uintptr_t, bool invert = false) = 0;
	virtual uintptr_t condition_to_value(ConditionalJumpType) = 0;
	virtual void abort() = 0;
};