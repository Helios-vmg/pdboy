#include "RegisterStore.h"
#include "GameboyCpu.h"
#include <cstring>
#include <iostream>
#include <iomanip>

RegisterStore::RegisterStore(GameboyCpu &cpu): cpu(&cpu){
	memset(this->registers, 0, sizeof(this->registers));
}

unsigned RegisterStore::get(Register8 reg) const{
	return this->registers[(int)reg];
}

void RegisterStore::set(Register8 reg, unsigned value){
	//if (reg == Register8::A){
	//	std::cout << "PC = 0x" << std::hex << std::setw(4) << std::setfill('0') << this->cpu->current_pc << std::endl;
	//	std::cout << "A  = 0x" << std::hex << std::setw(4) << std::setfill('0') << (value & 0xFF) << std::endl;
	//}
	this->registers[(int)reg] = value & 0xFF;
}

unsigned RegisterStore::get(Register16 reg) const{
	Register8 hi, lo;
	switch (reg){
		case Register16::AF:
			hi = Register8::A;
			lo = Register8::Flags;
			break;
		case Register16::BC:
			hi = Register8::B;
			lo = Register8::C;
			break;
		case Register16::DE:
			hi = Register8::D;
			lo = Register8::E;
			break;
		case Register16::HL:
			hi = Register8::H;
			lo = Register8::L;
			break;
		case Register16::SP:
			return this->registers[(int)Register16::StackPointer];
		case Register16::PC:
			return this->registers[(int)Register16::ProgramCounter];
		default:
			return 0;
	}
	return (this->get(hi) << 8) | this->get(lo);
}

void RegisterStore::set(Register16 reg, unsigned value){
	value &= 0xFFFF;
	Register8 hi, lo;
	switch (reg){
		case Register16::AF:
			hi = Register8::A;
			lo = Register8::Flags;
			break;
		case Register16::BC:
			hi = Register8::B;
			lo = Register8::C;
			break;
		case Register16::DE:
			hi = Register8::D;
			lo = Register8::E;
			break;
		case Register16::HL:
			hi = Register8::H;
			lo = Register8::L;
			break;
		case Register16::SP:
			this->registers[(int)Register16::StackPointer] = value;
			return;
		case Register16::PC:
			this->registers[(int)Register16::ProgramCounter] = value;
			return;
		default:
			return;
	}
	this->set(lo, value & 0xFF);
	this->set(hi, value >> 8);
}

void RegisterStore::set_flags(bool zero, bool subtract, bool half_carry, bool carry){
	unsigned val =
		((unsigned)zero << (unsigned)Flags::Zero) |
		((unsigned)subtract << (unsigned)Flags::Subtract) |
		((unsigned)half_carry << (unsigned)Flags::HalfCarry) |
		((unsigned)carry << (unsigned)Flags::Carry);
	this->set(Register8::Flags, val);
}

void RegisterStore::set_flags(unsigned mode_mask, unsigned value_mask){
	auto value = this->get(Register8::Flags);
	value = (~mode_mask & value_mask) | (mode_mask & (value ^ value_mask));
	this->set(Register8::Flags, value);
}
