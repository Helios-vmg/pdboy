#include "InterpreterCodeGenerator.h"
#include "FlagSetting.h"
#include <iomanip>
#include <type_traits>

#define TEMPTYPE "unsigned"
#define TEMPDECL "\t" TEMPTYPE " "
#define CONSTTEMPDECL "\tconst " TEMPTYPE " "

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

template <typename T>
static typename std::enable_if<std::is_integral<T>::value, std::string>::type to_string(T n){
	std::stringstream s;
	s << n;
	return s.str();
}

static const std::string &temp_to_string(uintptr_t p){
	return *(std::string *)p;
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
	s << TEMPDECL << name << " = this->memory_controller.load8(this->registers.pc());\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::load_program_counter16(){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto name = get_temp_name(n++);
	s << TEMPDECL << name << " = this->memory_controller.load16(this->registers.pc());\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::get_register_value8(Register8 reg){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto name = get_temp_name(n++);
	s << TEMPDECL << name << " = this->registers.get(" << to_string(reg) << ");\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::get_register_value16(Register16 reg){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto name = get_temp_name(n++);
	s << TEMPDECL << name << " = this->registers.get(" << to_string(reg) << ");\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::load_hl8(){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto name = get_temp_name(n++);
	s << TEMPDECL << name << " = this->memory_controller.load8(this->registers.get(Register16::HL));\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::load_mem8(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto name = get_temp_name(n++);
	s << TEMPDECL << name << " = this->memory_controller.load8(" << temp_to_string(val) << ");\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::load_mem16(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto name = get_temp_name(n++);
	s << TEMPDECL << name << " = this->memory_controller.load16(" << temp_to_string(val) << ");\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::load_ff00_offset8(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto name = get_temp_name(n++);
	s << TEMPDECL << name << " = this->memory_controller.load16(0xFF00 + " << temp_to_string(val) << ");\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

//uintptr_t InterpreterCodeGenerator::load_sp_offset8(uintptr_t val){
//	auto &back = this->definition_stack.back();
//	auto &s = *back.function_contents;
//	auto &n = back.temporary_index;
//	auto name = get_temp_name(n++);
//	s << TEMPDECL << name << " = this->memory_controller.load16(this->registers.sp() + " << (std::string *)val << ");\n";
//	auto ret = copy(name);
//	this->temporary_values.push_back(ret);
//	return (uintptr_t)ret;
//}

uintptr_t InterpreterCodeGenerator::load_sp_offset16(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto name = get_temp_name(n++);
	s << TEMPDECL << name << " = this->memory_controller.load16(this->registers.sp() + " << temp_to_string(val) << ");\n";
	auto ret = copy(name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

void InterpreterCodeGenerator::write_register8(Register8 reg, uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->registers.set(" << to_string(reg) << ", " << temp_to_string(val) << ");\n";
}

void InterpreterCodeGenerator::write_register16_literal(Register16 reg, unsigned val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->registers.set(" << to_string(reg) << ", " << val << ");\n";
}

void InterpreterCodeGenerator::write_register16(Register16 reg, uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->registers.set(" << to_string(reg) << ", " << temp_to_string(val) << ");\n";
}

void InterpreterCodeGenerator::store_hl8(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->memory_controller.store8(this->registers.get(Register16::HL), " << temp_to_string(val) << ");\n";
}

void InterpreterCodeGenerator::store_mem8(uintptr_t mem, uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->memory_controller.store8(" << temp_to_string(mem) << ", " << temp_to_string(val) << ");\n";
}

void InterpreterCodeGenerator::store_mem16(uintptr_t mem, uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->memory_controller.store16(" << temp_to_string(mem) << ", " << temp_to_string(val) << ");\n";
}

void InterpreterCodeGenerator::store_mem_ff00_8(uintptr_t offset, uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->memory_controller.store8(0xFF00 + " << temp_to_string(offset) << ", " << temp_to_string(val) << ");\n";
}

void InterpreterCodeGenerator::take_time(unsigned cycles){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->take_time(" << cycles << ");\n";
}

void InterpreterCodeGenerator::zero_flags(){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->set(Register8::Flags, 0);\n";
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

std::array<uintptr_t, 3> InterpreterCodeGenerator::add(uintptr_t valA, uintptr_t valB, unsigned modulo){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	auto carrybits_name = get_temp_name(n++);
	auto half_carry_name = get_temp_name(n++);
	auto full_carry_name = get_temp_name(n++);
	auto &valA_name = temp_to_string(valA);
	auto &valB_name = temp_to_string(valB);
	unsigned half_carry_mask, full_carry_mask;
	if (modulo == 8){
		half_carry_mask = 0x10;
		full_carry_mask = 0x100;
	}else if (modulo == 16){
		half_carry_mask = 0x1000;
		full_carry_mask = 0x10000;
	}else
		abort();
	s
		<< TEMPDECL << result_name << " = " << valA_name << " + " << valB_name << ";\n"
		<< TEMPDECL << carrybits_name << " = " << valA_name << " ^ " << valB_name << " ^ " << result_name << ";\n"
		<< TEMPDECL << half_carry_name << " = " << carrybits_name << " & 0x10;\n"
		<< TEMPDECL << full_carry_name << " = " << carrybits_name << " & 0x100;\n";

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
	auto &valB_name = temp_to_string(valB);
	s
		<< TEMPDECL << carry_name << " = this->registers.get(Flags::Carry);\n"
		<< TEMPDECL << temp_name << " = " << valB_name << " + " << carry_name << ";\n";
	auto temp = copy(temp_name);
	this->temporary_values.push_back(temp);
	return this->add8(valA, (uintptr_t)temp);
}

std::array<uintptr_t, 3> InterpreterCodeGenerator::sub8(uintptr_t valA, uintptr_t valB){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto negated_name = get_temp_name(n++);
	s << TEMPDECL << negated_name << " = 0xFF & (1 + ~" << valB << ");\n";
	auto temp = copy(negated_name);
	this->temporary_values.push_back(temp);
	return this->add8(valA, (uintptr_t)temp);
}

std::array<uintptr_t, 3> InterpreterCodeGenerator::sub8_carry(uintptr_t valA, uintptr_t valB){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto negated_name = get_temp_name(n++);
	s << TEMPDECL << negated_name << " = 0xFF & (1 + ~" << valB << ");\n";
	auto temp = copy(negated_name);
	this->temporary_values.push_back(temp);
	return this->add8_carry(valA, (uintptr_t)temp);
}

uintptr_t InterpreterCodeGenerator::and8(uintptr_t valA, uintptr_t valB){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << TEMPDECL << result_name << " = " << temp_to_string(valA) << " & " << temp_to_string(valA) << ";\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::xor8(uintptr_t valA, uintptr_t valB){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << TEMPDECL << result_name << " = " << temp_to_string(valA) << " ^ " << temp_to_string(valA) << ";\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::or8(uintptr_t valA, uintptr_t valB){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << TEMPDECL << result_name << " = " << temp_to_string(valA) << " | " << temp_to_string(valA) << ";\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

std::array<uintptr_t, 3> InterpreterCodeGenerator::cmp8(uintptr_t valA, uintptr_t valB){
	return this->sub8(valA, valB);
}

static std::pair<std::string, std::string> to_string(const FlagSetting &setting){
	std::string temp;
	switch (setting.op){
		case FlagSetting::Operation::Reset:
			return { "0", "0" };
		case FlagSetting::Operation::Set:
			return { "0", "1" };
		case FlagSetting::Operation::Keep:
			return { "1", "0" };
		case FlagSetting::Operation::Flip:
			return { "1", "1" };
		case FlagSetting::Operation::IfNonZero:
			temp = "!";
		case FlagSetting::Operation::IfZero:
			temp += "!(";
			temp += temp_to_string(setting.src_value);
			temp += ")";
			break;
		default:
			abort();
	}
	return{ "0", temp };
}

void InterpreterCodeGenerator::set_flags(const FlagSettings &fs){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	std::pair<FlagSetting, const char *> settings[] = {
		{fs.zero, "0"},
		{fs.subtract, "1"},
		{fs.half_carry, "2"},
		{fs.carry, "3"},
	};
	std::sort(settings, settings + array_length(settings), [](auto &a, auto &b){ return (int)a.first.op < (int)b.first.op; });
	int i = 0;
	std::string A, B;
	for (auto &setting : settings){
		if (i++){
			A += " | ";
			B += " | ";
		}

		auto p = to_string(setting.first);

		A += "((";
		A += p.first;
		A += ") << ";
		A += setting.second;
		A += ")";

		B += "((";
		B += p.second;
		B += ") << ";
		B += setting.second;
		B += ")";
	}
	s << "\tthis->registers.set_flags(" << A << ", " << B << ");\n";
}

uintptr_t InterpreterCodeGenerator::plus_1(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << TEMPDECL << result_name << " = " << temp_to_string(val) << " + 1;\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::minus_1(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << TEMPDECL << result_name << " = " << temp_to_string(val) << " - 1;\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::bitwise_not(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << TEMPDECL << result_name << " = ~" << temp_to_string(val) << ";\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

void InterpreterCodeGenerator::disable_interrupts(){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->interrupt_toggle(false);\n";
}

void InterpreterCodeGenerator::enable_interrupts(){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->interrupt_toggle(true);\n";
}

uintptr_t InterpreterCodeGenerator::rotate8(uintptr_t val, bool left, bool through_carry){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	auto carry_name = get_temp_name(n++);
	if (left){
		if (!through_carry){
			s
				<< TEMPDECL << carry_name << " = this->registers.get(Flags::Carry) ? 1U : 0U;\n"
				<< TEMPDECL << result_name << " = (" << temp_to_string(val) << " << 1) | carry;\n";
		}else{
			auto bit_name = get_temp_name(n++);
			s
				<< TEMPDECL << bit_name << " = " << temp_to_string(val) << " & 0xFF;\n"
				<< TEMPDECL << result_name << " = (" << bit_name << " << 1) | (" << bit_name << " >> 7);\n";

		}
	}else{
		if (!through_carry){
			s
				<< TEMPDECL << carry_name << " = this->registers.get(Flags::Carry) ? 0x80U : 0U;\n"
				<< TEMPDECL << result_name << " = (" << temp_to_string(val) << " >> 1) | carry;\n";
		}else{
			auto bit_name = get_temp_name(n++);
			s
				<< TEMPDECL << bit_name << " = " << temp_to_string(val) << " & 0xFF;\n"
				<< TEMPDECL << result_name << " = (" << bit_name << " >> 1) | (" << bit_name << " << 7);\n";
		}
	}

	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

void InterpreterCodeGenerator::set_PC_if(uintptr_t val, ConditionalJumpType type){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;

	const char *condition;
	switch (type){
		case ConditionalJumpType::NotZero:
			condition = "\tif (!this->registers.get(Flags::Zero))\n";
			break;
		case ConditionalJumpType::Zero:
			condition = "\tif (this->registers.get(Flags::Zero))\n";
			break;
		case ConditionalJumpType::NotCarry:
			condition = "\tif (!this->registers.get(Flags::Carry))\n";
			break;
		case ConditionalJumpType::Carry:
			condition = "\tif (this->registers.get(Flags::Carry))\n";
			break;
		default:
			abort();
	}
	s << condition << "\t\tthis->registers.set(Register16::PC, " << temp_to_string(val) << ");\n";
}

void InterpreterCodeGenerator::add8_PC_if(uintptr_t val, ConditionalJumpType type){
	std::string temp = "(this->registers.pc())";
	this->set_PC_if((uintptr_t)&temp, type);
}

void InterpreterCodeGenerator::stop(){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->stop();";
}

uintptr_t InterpreterCodeGenerator::shift_left(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);

	s << TEMPDECL << result_name << " = " << temp_to_string(val) << " << 1;\n";

	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::arithmetic_shift_right(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	auto temp_name = get_temp_name(n++);

	s
		<< TEMPDECL << temp_name << " = " << temp_to_string(val) << " & 0xFF;\n"
		<< TEMPDECL << result_name << " = (" << temp_name << " & 0x80) | (" << temp_name << " >> 1);\n";

	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::bitwise_shift_right(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);

	s << TEMPDECL << result_name << " = (" << temp_to_string(val) << " & 0xFF) >> 1;\n";

	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::get_bit_value(uintptr_t val, unsigned bit){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);

	s << TEMPDECL << result_name << " = (" << temp_to_string(val);
	if (bit)
		s << " >> " << bit;
	s << ") & 1;\n";

	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::set_bit_value(uintptr_t val, unsigned bit, bool on){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	auto mask_name = get_temp_name(n++);

	s
		<< CONSTTEMPDECL << mask_name << " = 1 << " << bit << ";\n"
		<< TEMPDECL << result_name << " = " << temp_to_string(val);
	if (!bit)
		s << " & ~";
	else
		s << " | ";
	s << mask_name << ";\n";

	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

std::pair<uintptr_t, uintptr_t> InterpreterCodeGenerator::perform_decimal_adjustment(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	auto carry_name = get_temp_name(n++);

	s
		<< TEMPDECL << result_name << " = " << temp_to_string(val) << " & 0xFF;\n"
		<< "\tif (!this->registers.get(Flags::Subtract)){\n"
		<< "\t\tif (this->registers.get(Flags::HalfCarry) || (" << result_name << " & 0x0F) > 9)\n"
		<< "\t\t\t" << result_name << " += 6;\n"
		<< "\n"
		<< "\t\tif (this->registers.get(Flags::Carry) || " << result_name << " > 0x9F)\n"
		<< "\t\t\t" << result_name << " += 0x60;\n"
		<< "\t}else{\n"
		<< "\t\tif (this->registers.get(Flags::HalfCarry))\n"
		<< "\t\t\t" << result_name << " = (" << result_name << " - 6) & 0xFF;\n"
		<< "\n"
		<< "\t\tif (this->registers.get(Flags::Carry))\n"
		<< "\t\t\t" << result_name << " -= 0x60;\n"
		<< "\t}\n";

	s << TEMPDECL << carry_name << " = " << result_name << " & 0x100;\n";

	auto ret0 = copy(result_name);
	auto ret1 = copy(carry_name);
	this->temporary_values.push_back(ret0);
	this->temporary_values.push_back(ret1);
	return { (uintptr_t)ret0, (uintptr_t)ret1 };
}

uintptr_t InterpreterCodeGenerator::swap_nibbles(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	auto temp_name = get_temp_name(n++);

	s
		<< TEMPDECL << temp_name << " = " << temp_to_string(val) << " & 0xFF;\n"
		<< TEMPDECL << result_name << " = (" << temp_name << "<< 4) | (" << temp_name << " >> 4);\n";

	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::imm(unsigned val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);

	s << TEMPDECL << result_name << " = " << val << ";\n";

	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}
