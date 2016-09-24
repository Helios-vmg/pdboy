#pragma once

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

class RegisterStore{
	GameboyCpu *cpu;
	unsigned registers[10];
public:
	RegisterStore(GameboyCpu &cpu);
	unsigned get(Register8 reg) const;
	void set(Register8 reg, unsigned value);
	unsigned get(Register16 reg) const;
	void set(Register16 reg, unsigned value);
	bool get(Flags flag) const{
		return !!(this->get(Register8::Flags) & (1 << (unsigned)flag));
	}
	void set(Flags flag, bool value){
		unsigned mask = 1 << (unsigned)flag;
		unsigned val = this->get(Register8::Flags);
		if (value)
			val |= mask;
		else
			val ^= mask;
		this->set(Register8::Flags, val);
	}
	void set_flags(bool zero, bool subtract, bool half_carry, bool carry);
	void set_flags(unsigned mode_mask, unsigned value_mask);
	unsigned sp() const{
		return this->get(Register16::SP);
	}
	unsigned pc() const{
		return this->get(Register16::PC);
	}
};
