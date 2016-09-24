#pragma once
#include <cstdint>

enum class Register8{
	B = 0,
	C = 1,
	D = 2,
	E = 3,
	H = 4,
	L = 5,
	Flags = 6,
	A = 7,
};

enum class Register16{
	AF = 0,
	BC = 1,
	DE = 2,
	HL = 3,
	SP = 4,
	PC = 5,
	Reserved1 = 6,
	Reserved2 = 7,
	StackPointer = 8,
	ProgramCounter = 9,
};

enum class Flags{
	Zero = 7,
	Subtract = 6,
	HalfCarry = 5,
	Carry = 4,
};

class GameboyCpu;

typedef unsigned integer_type;

class RegisterStore{
	GameboyCpu *cpu;
	integer_type registers[10];
public:
	RegisterStore(GameboyCpu &cpu);
	integer_type get(Register8 reg) const;
	void set(Register8 reg, integer_type value);
	integer_type get(Register16 reg) const;
	void set(Register16 reg, integer_type value);
	bool get(Flags flag) const{
		return !!(this->get(Register8::Flags) & ((integer_type)1 << (integer_type)flag));
	}
	void set(Flags flag, bool value){
		integer_type mask = (integer_type)1 << (integer_type)flag;
		integer_type val = this->get(Register8::Flags);
		if (value)
			val |= mask;
		else
			val ^= mask;
		this->set(Register8::Flags, val);
	}
	void set_flags(bool zero, bool subtract, bool half_carry, bool carry);
	void set_flags(integer_type mode_mask, integer_type value_mask);
	integer_type sp() const{
		return this->get(Register16::SP);
	}
	integer_type pc() const{
		return this->get(Register16::PC);
	}
};
