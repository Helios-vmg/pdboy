#pragma once
#include <cstdint>

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

typedef unsigned integer_type;

class RegisterStore{
	GameboyCpu *cpu;
	struct{
		union{
			std::uint16_t af;
			struct{
				std::uint8_t f;
				std::uint8_t a;
			} u8;
		} af;
		union{
			std::uint16_t bc;
			struct{
				std::uint8_t c;
				std::uint8_t b;
			} u8;
		} bc;
		union{
			std::uint16_t de;
			struct{
				std::uint8_t e;
				std::uint8_t d;
			} u8;
		} de;
		union{
			std::uint16_t hl;
			struct{
				std::uint8_t l;
				std::uint8_t h;
			} u8;
		} hl;
		std::uint16_t sp;
		std::uint16_t pc;
	} data;
public:
	RegisterStore(GameboyCpu &cpu);
	bool get(Flags flag) const{
		return !!(this->f() & ((integer_type)1 << (integer_type)flag));
	}
	void set(Flags flag, bool value){
		integer_type mask = (integer_type)1 << (integer_type)flag;
		auto &val = this->f();
		if (value)
			val |= mask;
		else
			val ^= mask;
	}
	void set_flags(bool zero, bool subtract, bool half_carry, bool carry);
	void set_flags(integer_type mode_mask, integer_type value_mask);

	std::uint8_t &a(){
		return this->data.af.u8.a;
	}
	std::uint8_t &f(){
		return this->data.af.u8.f;
	}
	const std::uint8_t &f() const{
		return this->data.af.u8.f;
	}
	std::uint8_t &b(){
		return this->data.bc.u8.b;
	}
	std::uint8_t &c(){
		return this->data.bc.u8.c;
	}
	std::uint8_t &d(){
		return this->data.de.u8.d;
	}
	std::uint8_t &e(){
		return this->data.de.u8.e;
	}
	std::uint8_t &h(){
		return this->data.hl.u8.h;
	}
	std::uint8_t &l(){
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
