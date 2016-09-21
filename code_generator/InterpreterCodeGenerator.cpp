#include "InterpreterCodeGenerator.h"
#include <iomanip>

static std::string get_temp_name(unsigned i){
	std::stringstream stream;
	stream << "temp" << i;
	return stream.str();
}

template <typename T>
static T *copy(const T &x){
	return new T(x);
}

static const char *to_string(Register8 reg){
	switch (reg){
		case Register8::B:
			return "Register8::B";
		case Register8::C:
			return "Register8::C";
		case Register8::D:
			return "Register8::D";
		case Register8::E:
			return "Register8::E";
		case Register8::H:
			return "Register8::H";
		case Register8::L:
			return "Register8::L";
		case Register8::None:
			return nullptr;
		case Register8::A:
			return "Register8::A";
		default:
			return nullptr;
	}
}

static const char *to_string(Register16 reg){
	switch (reg){
		case Register16::AF:
			return "Register16::AF";
		case Register16::BC:
			return "Register16::BC";
		case Register16::DE:
			return "Register16::DE";
		case Register16::HL:
			return "Register16::HL";
		case Register16::SP:
			return "Register16::SP";
		case Register16::PC:
			return "Register16::PC";
		default:
			return nullptr;
	}
}

InterpreterCodeGenerator::~InterpreterCodeGenerator(){
	for (auto p : this->temporary_values)
		delete p;
}

void InterpreterCodeGenerator::begin_opcode_definition(unsigned first){
	std::stringstream stream;
	stream << "opcode" << std::hex << std::setw(2) << std::setfill('0') << first;
	auto name = stream.str();
	auto contents = &this->functions[name];
	this->definition_stack.push_back({ contents, 0 });
}

void InterpreterCodeGenerator::end_opcode_definition(unsigned first){
	this->definition_stack.pop_back();
}

void InterpreterCodeGenerator::begin_double_opcode_definition(unsigned first, unsigned second){
	std::stringstream stream;
	stream
		<< "opcode"
		<< std::hex << std::setw(2) << std::setfill('0') << first
		<< std::hex << std::setw(2) << std::setfill('0') << second;
	auto name = stream.str();
	auto contents = &this->functions[name];
	this->definition_stack.push_back({ contents, 0 });
}

void InterpreterCodeGenerator::end_double_opcode_definition(unsigned first, unsigned second){
	this->definition_stack.pop_back();
}

void InterpreterCodeGenerator::noop(){
}

void InterpreterCodeGenerator::halt(){
	auto &s = *this->definition_stack.back().function_contents;
	s << "\tthis->halt();\n";
}

uintptr_t InterpreterCodeGenerator::load_program_counter8(){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto name = get_temp_name(n++);
	s << "\tauto " << name << " = this->memory_controller.load8(this->registers.pc());\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::load_program_counter16(){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto name = get_temp_name(n++);
	s << "\tauto " << name << " = this->memory_controller.load16(this->registers.pc());\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::get_register_value8(Register8 reg){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto name = get_temp_name(n++);
	s << "\tauto " << name << " = this->registers.get(" << to_string(reg) << ");\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::get_register_value16(Register16 reg){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto name = get_temp_name(n++);
	s << "\tauto " << name << " = this->registers.get(" << to_string(reg) << ");\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::load_hl8(){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto name = get_temp_name(n++);
	s << "\tauto " << name << " = this->memory_controller.load8(this->registers.get(Register16::HL));\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::load_mem8(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto name = get_temp_name(n++);
	s << "\tauto " << name << " = this->memory_controller.load8(" << (std::string *)val << ");\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::load_mem16(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto name = get_temp_name(n++);
	s << "\tauto " << name << " = this->memory_controller.load16(" << (std::string *)val << ");\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::load_ff00_offset8(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto name = get_temp_name(n++);
	s << "\tauto " << name << " = this->memory_controller.load16(0xFF00 + " << (std::string *)val << ");\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

//uintptr_t InterpreterCodeGenerator::load_sp_offset8(uintptr_t val){
//	auto &back = this->definition_stack.back();
//	auto &s = *back.function_contents;
//	auto &n = back.temporary_index;
//	auto name = get_temp_name(n++);
//	s << "\tauto " << name << " = this->memory_controller.load16(this->registers.sp() + " << (std::string *)val << ");\n";
//	auto ret = copy(name);
//	this->temporary_values.push_back(ret);
//	return (uintptr_t)ret;
//}

uintptr_t InterpreterCodeGenerator::load_sp_offset16(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto name = get_temp_name(n++);
	s << "\tauto " << name << " = this->memory_controller.load16(this->registers.sp() + " << (std::string *)val << ");\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

void InterpreterCodeGenerator::write_register8(Register8 reg, uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->registers.set(" << to_string(reg) << ", " << (std::string *)val << ");\n";
}

void InterpreterCodeGenerator::write_register16_literal(Register16 reg, unsigned val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->registers.set(" << to_string(reg) << ", " << val << ");\n";
}

void InterpreterCodeGenerator::write_register16(Register16 reg, uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->registers.set(" << to_string(reg) << ", " << (std::string *)val << ");\n";
}

void InterpreterCodeGenerator::store_hl8(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->memory_controller.store8(this->registers.get(Register16::HL), " << (std::string *)val << ");\n";
}

void InterpreterCodeGenerator::store_mem8(uintptr_t mem, uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->memory_controller.store8(" << (std::string *)mem << ", " << (std::string *)val << ");\n";
}

void InterpreterCodeGenerator::store_mem16(uintptr_t mem, uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->memory_controller.store16(" << (std::string *)mem << ", " << (std::string *)val << ");\n";
}

void InterpreterCodeGenerator::store_mem_ff00_8(uintptr_t offset, uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->memory_controller.store8(0xFF00 + " << (std::string *)offset << ", " << (std::string *)val << ");\n";
}

void InterpreterCodeGenerator::take_time(unsigned cycles){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->take_time(" << cycles << ");\n";
}

void InterpreterCodeGenerator::dec_register8(Register8 reg){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->registers.set(" << to_string(reg) << ", this->registers.get(" << to_string(reg) << ") - 1);\n";
}

void InterpreterCodeGenerator::dec_register16(Register16 reg){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->registers.set(" << to_string(reg) << ", this->registers.get(" << to_string(reg) << ") - 1);\n";
}

void InterpreterCodeGenerator::dec2_SP(){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->registers.set(" << to_string(Register16::SP) << ", this->registers.sp() - 2);\n";
}

void InterpreterCodeGenerator::inc2_SP(){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->registers.set(" << to_string(Register16::SP) << ", this->registers.sp() + 2);\n";
}

void InterpreterCodeGenerator::inc_register8(Register8 reg){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->registers.set(" << to_string(reg) << ", this->registers.get(" << to_string(reg) << ") + 1);\n";
}

void InterpreterCodeGenerator::inc_register16(Register16 reg){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->registers.set(" << to_string(reg) << ", this->registers.get(" << to_string(reg) << ") + 1);\n";
}

std::array<uintptr_t, 3> InterpreterCodeGenerator::add8(uintptr_t){}

std::array<uintptr_t, 3> InterpreterCodeGenerator::add_carry(uintptr_t){}

std::array<uintptr_t, 3> InterpreterCodeGenerator::sub(uintptr_t){}

std::array<uintptr_t, 3> InterpreterCodeGenerator::sub_carry(uintptr_t){}

uintptr_t InterpreterCodeGenerator::and (uintptr_t){}

uintptr_t InterpreterCodeGenerator::xor (uintptr_t){}

uintptr_t InterpreterCodeGenerator::or (uintptr_t){}

std::array<uintptr_t, 3> InterpreterCodeGenerator::cmp(uintptr_t){}

void InterpreterCodeGenerator::set_flags(const FlagSettings &){}

uintptr_t InterpreterCodeGenerator::inc_temp(uintptr_t){}

uintptr_t InterpreterCodeGenerator::dec_temp(uintptr_t){}

void InterpreterCodeGenerator::add16(Register16){}

void InterpreterCodeGenerator::add_SP(uintptr_t){}

void InterpreterCodeGenerator::flip_A(){}

void InterpreterCodeGenerator::disable_interrupts(){}

void InterpreterCodeGenerator::enable_interrupts(){}

void InterpreterCodeGenerator::rotate_A(bool left, bool through_carry){}

void InterpreterCodeGenerator::add_PC(uintptr_t imm){}

void InterpreterCodeGenerator::set_PC_if(uintptr_t, ConditionalJumpType){}

void InterpreterCodeGenerator::add_PC_if(uintptr_t, ConditionalJumpType){}

void InterpreterCodeGenerator::stop(){}

uintptr_t InterpreterCodeGenerator::rotate_left(uintptr_t val){}

uintptr_t InterpreterCodeGenerator::rotate_right(uintptr_t val){}

uintptr_t InterpreterCodeGenerator::rotate_left_through_carry(uintptr_t val){}

uintptr_t InterpreterCodeGenerator::rotate_right_through_carry(uintptr_t val){}

uintptr_t InterpreterCodeGenerator::shift_left(uintptr_t val){}

uintptr_t InterpreterCodeGenerator::arithmetic_shift_right(uintptr_t val){}

uintptr_t InterpreterCodeGenerator::bitwise_shift_right(uintptr_t val){}

uintptr_t InterpreterCodeGenerator::get_bit_value(uintptr_t val, unsigned bit){}

uintptr_t InterpreterCodeGenerator::set_bit_value(uintptr_t val, unsigned bit, bool on){}

std::pair<uintptr_t, uintptr_t> InterpreterCodeGenerator::perform_decimal_adjustment(uintptr_t val){}

uintptr_t InterpreterCodeGenerator::swap_nibbles(uintptr_t val){}
