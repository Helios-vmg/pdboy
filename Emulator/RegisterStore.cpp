#include "RegisterStore.h"
#include <cstring>

RegisterStore::RegisterStore(){
	memset(this->registers, 0, sizeof(this->registers));
}

unsigned RegisterStore::get(Register8 reg) const{
	return this->registers[(int)reg];
}

void RegisterStore::set(Register8 reg, unsigned value){
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
