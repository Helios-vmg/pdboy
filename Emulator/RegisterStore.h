#pragma once
#include "CommonTypes.h"

#define REGISTERS_IN_STRUCT

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
	struct{
		union{
			std::uint16_t af;
			struct{
				byte_t f;
				byte_t a;
			} u8;
		} af;
		union{
			std::uint16_t bc;
			struct{
				byte_t c;
				byte_t b;
			} u8;
		} bc;
		union{
			std::uint16_t de;
			struct{
				byte_t e;
				byte_t d;
			} u8;
		} de;
		union{
			std::uint16_t hl;
			struct{
				byte_t l;
				byte_t h;
			} u8;
		} hl;
		std::uint16_t sp;
		std::uint16_t pc;
	} data;
public:
	RegisterStore(GameboyCpu &cpu);
	bool get(Flags flag) const{
		return !!(this->f() & ((main_integer_t)1 << (main_integer_t)flag));
	}
	void set(Flags flag, bool value){
		main_integer_t mask = (main_integer_t)1 << (main_integer_t)flag;
		auto &val = this->f();
		if (value)
			val |= mask;
		else
			val ^= mask;
	}
	void set_flags(bool zero, bool subtract, bool half_carry, bool carry);
	void set_flags(main_integer_t mode_mask, main_integer_t value_mask);

	byte_t &a(){
		return this->data.af.u8.a;
	}
	byte_t &f(){
		return this->data.af.u8.f;
	}
	const byte_t &f() const{
		return this->data.af.u8.f;
	}
	byte_t &b(){
		return this->data.bc.u8.b;
	}
	byte_t &c(){
		return this->data.bc.u8.c;
	}
	byte_t &d(){
		return this->data.de.u8.d;
	}
	byte_t &e(){
		return this->data.de.u8.e;
	}
	byte_t &h(){
		return this->data.hl.u8.h;
	}
	byte_t &l(){
		return this->data.hl.u8.l;
	}

	std::uint16_t &af(){
		return this->data.af.af;
	}
	std::uint16_t &bc(){
		return this->data.bc.bc;
	}
	std::uint16_t &de(){
		return this->data.de.de;
	}
	std::uint16_t &hl(){
		return this->data.hl.hl;
	}
	std::uint16_t &sp(){
		return this->data.sp;
	}
	std::uint16_t &pc(){
		return this->data.pc;
	}
};
