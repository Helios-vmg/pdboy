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
protected:
	void begin_opcode_definition(unsigned first) override;
	void end_opcode_definition(unsigned first) override;
	void begin_double_opcode_definition(unsigned first, unsigned second) override;
	void end_double_opcode_definition(unsigned first, unsigned second) override;
public:
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
	void dec_register8(Register8 reg) override;
	void dec_register16(Register16 reg) override;
	void dec2_SP() override;
	void inc2_SP() override;
	void inc_register8(Register8 reg) override;
	void inc_register16(Register16 reg) override;
	std::array<uintptr_t, 3> add8(uintptr_t) override;
	std::array<uintptr_t, 3> add_carry(uintptr_t) override;
	std::array<uintptr_t, 3> sub(uintptr_t) override;
	std::array<uintptr_t, 3> sub_carry(uintptr_t) override;
	uintptr_t and(uintptr_t) override;
	uintptr_t xor(uintptr_t) override;
	uintptr_t or(uintptr_t) override;
	std::array<uintptr_t, 3> cmp(uintptr_t) override;
	void set_flags(const FlagSettings &) override;
	uintptr_t inc_temp(uintptr_t) override;
	uintptr_t dec_temp(uintptr_t) override;
	void add16(Register16) override;
	void add_SP(uintptr_t) override;
	void flip_A() override;
	void disable_interrupts() override;
	void enable_interrupts() override;
	void rotate_A(bool left, bool through_carry) override;
	void add_PC(uintptr_t imm) override;
	void set_PC_if(uintptr_t, ConditionalJumpType) override;
	void add_PC_if(uintptr_t, ConditionalJumpType) override;
	void stop() override;
	uintptr_t rotate_left(uintptr_t val) override;
	uintptr_t rotate_right(uintptr_t val) override;
	uintptr_t rotate_left_through_carry(uintptr_t val) override;
	uintptr_t rotate_right_through_carry(uintptr_t val) override;
	uintptr_t shift_left(uintptr_t val) override;
	uintptr_t arithmetic_shift_right(uintptr_t val) override;
	uintptr_t bitwise_shift_right(uintptr_t val) override;
	uintptr_t get_bit_value(uintptr_t val, unsigned bit) override;
	uintptr_t set_bit_value(uintptr_t val, unsigned bit, bool on) override;
	std::pair<uintptr_t, uintptr_t> perform_decimal_adjustment(uintptr_t val) override;
	uintptr_t swap_nibbles(uintptr_t val) override;
};
