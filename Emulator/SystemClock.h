#pragma once

#include "CommonTypes.h"

class Gameboy;

class SystemClock{
	Gameboy *system;
	std::uint64_t clock = 0;
	std::uint32_t DIV_register = 0;
	std::uint32_t TIMA_register = 0;
	std::uint8_t TMA_register = 0;
	std::uint8_t TAC_register = 0;
	std::uint32_t timer_enable_mask = 0;
	std::uint32_t tac_mask = 0;
	std::uint32_t tac_mask_bit = 0;
	std::uint32_t tima_overflow = 0;
	bool last_preincrement_value = false;
	bool trigger_interrupt = false;
	static const std::uint32_t tac_selector[][2];

	void cascade_timer_behavior(std::uint32_t old_tac = 0, std::uint32_t new_tac = 0);
	void cascade_timer_behavior_no_check();
	void handle_tima_overflow_part1();
	void handle_tima_overflow_part2();
public:
	SystemClock(Gameboy &system): system(&system){}

	std::uint64_t get_clock_value() const{
		return this->clock;
	}
	void advance_clock(std::uint32_t clocks);
	byte_t get_DIV_register() const{
		auto ret = this->DIV_register;
		ret >>= 8;
		ret &= 0xFF;
		return (byte_t)ret;
	}
	void reset_DIV_register(){
		this->DIV_register = 0;
		this->cascade_timer_behavior();
	}
	bool get_trigger_interrupt(){
		auto ret = this->trigger_interrupt;
		this->trigger_interrupt = false;
		return ret;
	}
	void set_TIMA_register(byte_t val){
		this->TIMA_register = val;
		this->tima_overflow = 0;
	}
	byte_t get_TIMA_register() const{
		auto ret = this->TIMA_register;
		ret &= 0xFF;
		return (byte_t)ret;
	}
	void set_TMA_register(byte_t val){
		this->TMA_register = val;
		if (this->tima_overflow){
			this->TIMA_register = this->TMA_register;
			this->tima_overflow = 0;
		}
	}
	byte_t get_TMA_register() const{
		auto ret = this->TMA_register;
		ret &= 0xFF;
		return (byte_t)ret;
	}
	void set_TAC_register(byte_t val);
	byte_t get_TAC_register() const{
		auto ret = this->TAC_register;
		ret &= 0xFF;
		return (byte_t)ret;
	}

};
