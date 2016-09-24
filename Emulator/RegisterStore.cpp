#include "RegisterStore.h"
#include "GameboyCpu.h"
#include <cstring>
#include <iostream>
#include <iomanip>

RegisterStore::RegisterStore(GameboyCpu &cpu): cpu(&cpu){
	memset(&this->data, 0, sizeof(this->data));
}

void RegisterStore::set_flags(bool zero, bool subtract, bool half_carry, bool carry){
	unsigned val =
		((integer_type)zero << (integer_type)Flags::Zero) |
		((integer_type)subtract << (integer_type)Flags::Subtract) |
		((integer_type)half_carry << (integer_type)Flags::HalfCarry) |
		((integer_type)carry << (integer_type)Flags::Carry);
	this->f() = val;
}

void RegisterStore::set_flags(integer_type mode_mask, integer_type value_mask){
	auto &value = this->f();
	value = (~mode_mask & value_mask) | (mode_mask & (value ^ value_mask));
}
