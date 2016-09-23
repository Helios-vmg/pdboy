#include "CodeGenerator.h"
#include <map>
#include <string>
#include <vector>
#include <array>
#include <sstream>

class InterpreterCodeGenerator : public CodeGenerator{
	std::map<std::string, std::stringstream> functions;
	struct DefinitionContext{
		std::stringstream *function_contents;
		unsigned temporary_index;
	};
	std::vector<DefinitionContext> definition_stack;
	std::vector<std::string *> temporary_values;

	std::array<uintptr_t, 3> add(uintptr_t, uintptr_t, unsigned modulo = 8);
protected:
	void begin_opcode_definition(unsigned first) override;
	void end_opcode_definition(unsigned first) override;
	void begin_double_opcode_definition(unsigned first, unsigned second) override;
	void end_double_opcode_definition(unsigned first, unsigned second) override;
public:
	InterpreterCodeGenerator(std::shared_ptr<CpuDefinition> definition): CodeGenerator(definition){}
	~InterpreterCodeGenerator();
	void noop() override;
	void halt() override;
	uintptr_t load_program_counter8() override;
	uintptr_t load_program_counter16() override;
	uintptr_t get_register_value8(Register8) override;
	uintptr_t get_register_value16(Register16) override;
	uintptr_t load_hl8() override;
	uintptr_t load_mem8(uintptr_t) override;
	uintptr_t load_mem16(uintptr_t) override;
	uintptr_t load_ff00_offset8(uintptr_t) override;
	//uintptr_t load_sp_offset8(uintptr_t) override;
	uintptr_t load_sp_offset16(uintptr_t) override;
	void write_register8(Register8, uintptr_t) override;
	void write_register16_literal(Register16, unsigned) override;
	void write_register16(Register16, uintptr_t) override;
	void store_hl8(uintptr_t) override;
	void store_mem8(uintptr_t mem, uintptr_t val) override;
	void store_mem16(uintptr_t mem, uintptr_t val) override;
	void store_mem_ff00_8(uintptr_t mem, uintptr_t val) override;
	void take_time(unsigned) override;
	void zero_flags() override;
	void dec_register8(Register8 reg) override;
	void dec_register16(Register16 reg) override;
	void dec2_SP() override;
	void inc2_SP() override;
	void inc_register8(Register8 reg) override;
	void inc_register16(Register16 reg) override;
	std::array<uintptr_t, 3> add8(uintptr_t a, uintptr_t b) override{
		return this->add(a, b);
	}
	std::array<uintptr_t, 3> add16_using_carry_modulo_16(uintptr_t a, uintptr_t b) override{
		return this->add(a, b, 16);
	}
	std::array<uintptr_t, 3> add8_carry(uintptr_t, uintptr_t) override;
	std::array<uintptr_t, 3> sub8(uintptr_t, uintptr_t) override;
	std::array<uintptr_t, 3> sub8_carry(uintptr_t, uintptr_t) override;
	uintptr_t and8(uintptr_t, uintptr_t) override;
	uintptr_t xor8(uintptr_t, uintptr_t) override;
	uintptr_t or8(uintptr_t, uintptr_t) override;
	std::array<uintptr_t, 3> cmp8(uintptr_t, uintptr_t) override;
	void set_flags(const FlagSettings &) override;
	uintptr_t plus_1(uintptr_t) override;
	uintptr_t minus_1(uintptr_t) override;
	std::array<uintptr_t, 3> add16(uintptr_t a, uintptr_t b) override{
		return this->add(a, b);
	}
	uintptr_t bitwise_not(uintptr_t) override;
	void disable_interrupts() override;
	void enable_interrupts() override;
	uintptr_t rotate8(uintptr_t, bool left, bool through_carry) override;
	void set_PC_if(uintptr_t, ConditionalJumpType) override;
	void add8_PC_if(uintptr_t, ConditionalJumpType) override;
	void stop() override;
	uintptr_t shift_left(uintptr_t val) override;
	uintptr_t arithmetic_shift_right(uintptr_t val) override;
	uintptr_t bitwise_shift_right(uintptr_t val) override;
	uintptr_t get_bit_value(uintptr_t val, unsigned bit) override;
	uintptr_t set_bit_value(uintptr_t val, unsigned bit, bool on) override;
	std::pair<uintptr_t, uintptr_t> perform_decimal_adjustment(uintptr_t val) override;
	uintptr_t swap_nibbles(uintptr_t val) override;
	uintptr_t imm(unsigned val) override;
};
