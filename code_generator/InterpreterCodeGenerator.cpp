#include "InterpreterCodeGenerator.h"
#include "FlagSetting.h"
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

template <typename T, size_t N>
static constexpr size_t array_length(const T (&)[N]){
	return N;
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
	s << "\tauto " << name << " = this->memory_controller.load8(" << *(std::string *)val << ");\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::load_mem16(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto name = get_temp_name(n++);
	s << "\tauto " << name << " = this->memory_controller.load16(" << *(std::string *)val << ");\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::load_ff00_offset8(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto name = get_temp_name(n++);
	s << "\tauto " << name << " = this->memory_controller.load16(0xFF00 + " << *(std::string *)val << ");\n";
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
	s << "\tauto " << name << " = this->memory_controller.load16(this->registers.sp() + " << *(std::string *)val << ");\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

void InterpreterCodeGenerator::write_register8(Register8 reg, uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->registers.set(" << to_string(reg) << ", " << *(std::string *)val << ");\n";
}

void InterpreterCodeGenerator::write_register16_literal(Register16 reg, unsigned val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->registers.set(" << to_string(reg) << ", " << val << ");\n";
}

void InterpreterCodeGenerator::write_register16(Register16 reg, uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->registers.set(" << to_string(reg) << ", " << *(std::string *)val << ");\n";
}

void InterpreterCodeGenerator::store_hl8(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->memory_controller.store8(this->registers.get(Register16::HL), " << *(std::string *)val << ");\n";
}

void InterpreterCodeGenerator::store_mem8(uintptr_t mem, uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->memory_controller.store8(" << *(std::string *)mem << ", " << *(std::string *)val << ");\n";
}

void InterpreterCodeGenerator::store_mem16(uintptr_t mem, uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->memory_controller.store16(" << *(std::string *)mem << ", " << *(std::string *)val << ");\n";
}

void InterpreterCodeGenerator::store_mem_ff00_8(uintptr_t offset, uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->memory_controller.store8(0xFF00 + " << *(std::string *)offset << ", " << *(std::string *)val << ");\n";
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

std::array<uintptr_t, 3> InterpreterCodeGenerator::add8(uintptr_t valA, uintptr_t valB){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	auto carrybits_name = get_temp_name(n++);
	auto half_carry_name = get_temp_name(n++);
	auto full_carry_name = get_temp_name(n++);
	auto &valA_name = *(std::string *)valA;
	auto &valB_name = *(std::string *)valB;
	s
		<< "\tauto " << result_name << " = " << valA_name << " + " << valB_name << ";\n"
		<< "\tauto " << carrybits_name << " = " << valA_name << " ^ " << valB_name << " ^ " << result_name << ";\n"
		<< "\tauto " << half_carry_name << " = " << carrybits_name << " & 0x10;\n"
		<< "\tauto " << full_carry_name << " = " << carrybits_name << " & 0x100;\n";

	std::string *retp[] = { copy(result_name), copy(half_carry_name), copy(full_carry_name) };
	std::array<uintptr_t, 3> ret;
	for (auto i = 3; i--;){
		this->temporary_values.push_back(retp[i]);
		ret[i] = (uintptr_t)retp[i];
	}
	return ret;
}

std::array<uintptr_t, 3> InterpreterCodeGenerator::add8_carry(uintptr_t valA, uintptr_t valB){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto carry_name = get_temp_name(n++);
	auto temp_name = get_temp_name(n++);
	auto &valB_name = *(std::string *)valB;
	s
		<< "\tauto " << carry_name << " = this->registers.get(Flags::Carry);\n"
		<< "\tauto " << temp_name << " = " << valB_name << " + " << carry_name << ";\n";
	auto temp = copy(temp_name);
	this->temporary_values.push_back(temp);
	return this->add8(valA, (uintptr_t)temp);
}

std::array<uintptr_t, 3> InterpreterCodeGenerator::sub8(uintptr_t valA, uintptr_t valB){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto negated_name = get_temp_name(n++);
	s << "\tauto " << negated_name << " = 0xFF & (1 + ~" << valB << ");\n";
	auto temp = copy(negated_name);
	this->temporary_values.push_back(temp);
	return this->add8(valA, (uintptr_t)temp);
}

std::array<uintptr_t, 3> InterpreterCodeGenerator::sub8_carry(uintptr_t valA, uintptr_t valB){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto negated_name = get_temp_name(n++);
	s << "\tauto " << negated_name << " = 0xFF & (1 + ~" << valB << ");\n";
	auto temp = copy(negated_name);
	this->temporary_values.push_back(temp);
	return this->add8_carry(valA, (uintptr_t)temp);
}

uintptr_t InterpreterCodeGenerator::and8(uintptr_t valA, uintptr_t valB){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << "\tauto " << result_name << " = " << *(std::string *)valA << " & " << *(std::string *)valA << ";\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::xor8(uintptr_t valA, uintptr_t valB){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << "\tauto " << result_name << " = " << *(std::string *)valA << " ^ " << *(std::string *)valA << ";\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::or8(uintptr_t valA, uintptr_t valB){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << "\tauto " << result_name << " = " << *(std::string *)valA << " | " << *(std::string *)valA << ";\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

std::array<uintptr_t, 3> InterpreterCodeGenerator::cmp8(uintptr_t valA, uintptr_t valB){
	return this->sub8(valA, valB);
}

static void to_string(std::ostream &s, const FlagSetting &setting){
	switch (setting.op){
		case FlagSetting::Operation::Reset:
			s << "0";
			break;
		case FlagSetting::Operation::Set:
			s << "1";
			break;
		case FlagSetting::Operation::Keep:
			s << "-1";
			break;
		case FlagSetting::Operation::Flip:
			s << "-2";
			break;
		case FlagSetting::Operation::IfNonZero:
			s << "!";
		case FlagSetting::Operation::IfZero:
			s << "!(" << *(std::string *)setting.src_value << ")";
			break;
	}
}

void InterpreterCodeGenerator::set_flags(const FlagSettings &settings){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->registers.set_flags(";
	to_string(s, settings.zero);
	s << ", ";
	to_string(s, settings.subtract);
	s << ", ";
	to_string(s, settings.half_carry);
	s << ", ";
	to_string(s, settings.carry);
	s << ");\n";
}

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
