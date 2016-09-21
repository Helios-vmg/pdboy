#pragma once
#include <cstdint>

class FlagSetting{
public:
	enum class Operation{
		Reset,
		Set,
		Keep,
		Flip,
		IfZero,
		IfNonZero,
		DontCare
	} op;
	uintptr_t src_value;
	FlagSetting(Operation op, uintptr_t src = 0): op(op), src_value(src){}

	static FlagSetting Reset;
	static FlagSetting Set;
	static FlagSetting Keep;
	static FlagSetting Flip;
	static FlagSetting IfZero(uintptr_t val){
		return FlagSetting(Operation::IfZero, val);
	}
	static FlagSetting IfNonZero(uintptr_t val){
		return FlagSetting(Operation::IfNonZero, val);
	}
	static FlagSetting DontCare;
};

struct FlagSettings{
	FlagSetting zero,
		subtract,
		half_carry,
		carry;
};
