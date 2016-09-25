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
		((main_integer_t)zero << (main_integer_t)Flags::Zero) |
		((main_integer_t)subtract << (main_integer_t)Flags::Subtract) |
		((main_integer_t)half_carry << (main_integer_t)Flags::HalfCarry) |
		((main_integer_t)carry << (main_integer_t)Flags::Carry);
	this->f() = val;
}

void RegisterStore::set_flags(main_integer_t mode_mask, main_integer_t value_mask){
	auto &value = this->f();
	value = (byte_t)((~mode_mask & value_mask) | (mode_mask & (value ^ value_mask)));
}
