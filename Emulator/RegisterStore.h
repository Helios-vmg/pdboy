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

#define DEFINE_REG8_ACCESSORS(xy, x) \
	byte_t &x(){ \
		return this->data.xy.u8.x; \
	} \
	const byte_t &x() const{ \
		return this->data.xy.u8.x; \
	} \
	byte_t get_##x() const{ \
		return this->x(); \
	} \
	byte_t &set_##x(){ \
		return this->x(); \
	}
#define DEFINE_REG16_ACCESSORS(xy) \
	std::uint16_t &xy(){ \
		return this->data.xy.xy; \
	} \
	const std::uint16_t &xy() const{ \
		return this->data.xy.xy; \
	} \
	std::uint16_t get_##xy() const{ \
		return this->xy(); \
	} \
	std::uint16_t &set_##xy(){ \
		return this->xy(); \
	}

	DEFINE_REG8_ACCESSORS(af, a);
	DEFINE_REG8_ACCESSORS(af, f);
	DEFINE_REG8_ACCESSORS(bc, b);
	DEFINE_REG8_ACCESSORS(bc, c);
	DEFINE_REG8_ACCESSORS(de, d);
	DEFINE_REG8_ACCESSORS(de, e);
	DEFINE_REG8_ACCESSORS(hl, h);
	DEFINE_REG8_ACCESSORS(hl, l);

	DEFINE_REG16_ACCESSORS(af);
	DEFINE_REG16_ACCESSORS(bc);
	DEFINE_REG16_ACCESSORS(de);
	DEFINE_REG16_ACCESSORS(hl);

	std::uint16_t &sp(){
			return this->data.sp;
	}
	const std::uint16_t &sp() const{
		return this->data.sp;
	}
	std::uint16_t get_sp() const{
		return this->sp();
	}
	std::uint16_t &set_sp(){
		return this->sp();
	}

	std::uint16_t &pc(){
		return this->data.pc;
	}
	const std::uint16_t &pc() const{
		return this->data.pc;
	}
	std::uint16_t get_pc() const{
		return this->pc();
	}
	std::uint16_t &set_pc(){
		return this->pc();
	}
};
