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
	Zero = 0,
	Subtract = 1,
	HalfCarry = 2,
	Carry = 3,
};

class RegisterStore{
	unsigned registers[10];
public:
	RegisterStore();
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
	void set_flags(int zero, int subtract, int half_carry, int carry){
		if (zero < 0)
			zero = this->get(Flags::Zero) - (zero + 1);
		if (subtract < 0)
			subtract = this->get(Flags::Subtract) - (zero + 1);
		if (half_carry < 0)
			half_carry = this->get(Flags::HalfCarry) - (zero + 1);
		if (carry < 0)
			carry = this->get(Flags::Carry) - (zero + 1);
		unsigned val =
			((unsigned)zero       << (unsigned)Flags::Zero) |
			((unsigned)subtract   << (unsigned)Flags::Subtract) |
			((unsigned)half_carry << (unsigned)Flags::HalfCarry) |
			((unsigned)carry      << (unsigned)Flags::Carry);
		this->set(Register8::Flags, val);
	}
	void set_flags_bit_twiddling(unsigned mode_mask, unsigned value_mask);
	unsigned sp() const{
		return this->get(Register16::SP);
	}
	unsigned pc() const{
		return this->get(Register16::PC);
	}
};
