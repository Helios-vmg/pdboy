#include "CartMbc3.h"
#include "HostSystem.h"

Mbc3Cartridge::Mbc3Cartridge(HostSystem &host, std::unique_ptr<std::vector<byte_t>> &&buffer, const CartridgeCapabilities &cc):
	Mbc1Cartridge(host, std::move(buffer), cc){
}

void Mbc3Cartridge::init_functions_derived(){
	Mbc1Cartridge::init_functions_derived();
	for (unsigned i = 0x20; i < 0x40; i++)
		this->write_callbacks[i] = write8_switch_rom_bank;
	for (unsigned i = 0x40; i < 0x60; i++)
		this->write_callbacks[i] = write8_switch_ram_bank;
	for (unsigned i = 0x60; i < 0x80; i++)
		this->write_callbacks[i] = write8_latch_rtc_registers;
}

void Mbc3Cartridge::set_ram_functions(){
	Mbc1Cartridge::set_ram_functions();
	if (!this->capabilities.has_ram)
		return;

	if (!this->ram_enabled)
		return;

	if (this->capabilities.ram_size == 1 << 11)
		return;

	if (this->current_rtc_register < 0)
		return;

	for (unsigned i = 0xA0; i < 0xC0; i++){
		this->write_callbacks[i] = write8_switchable_ram_bank;
		this->read_callbacks[i] = read8_switchable_ram_bank;
	}
}

posix_delta_t Mbc3Cartridge::get_rtc_counter_value(){
	posix_time_t start_time;
	auto &gb = this->host->get_guest();
	if (this->overriding_start_time >= 0)
		start_time = this->overriding_start_time;
	else
		start_time = gb.get_start_time();
	auto running_time = gb.get_system_clock().get_realtime_clock_value_seconds();
	auto current_time = start_time + (posix_time_t)running_time;

	if (this->rtc_start_time >= 0)
		return current_time - this->rtc_start_time;

	return current_time - start_time;
}

void Mbc3Cartridge::set_rtc_registers(){
	auto rtc = this->get_rtc_counter_value();
	auto days = rtc / 86400;
	rtc %= 86400;
	auto hours = rtc / 3600;
	rtc /= 3600;
	auto minutes = rtc / 60;
	rtc %= 60;
	auto seconds = rtc;

	this->rtc_registers.seconds = (byte_t)seconds;
	this->rtc_registers.minutes = (byte_t)minutes;
	this->rtc_registers.hours = (byte_t)hours;
	this->rtc_registers.day_counter_low = days % 256;
	this->rtc_registers.day_counter_high =
		(check_flag(days, 256) * rtc_8th_day_bit_mask) |
		((rtc_pause_time >= 0) * rtc_8th_day_bit_mask) |
		((days >= 512) * rtc_8th_day_bit_mask);
}

void Mbc3Cartridge::write8_switch_rom_bank(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc3Cartridge *>(sc);
	This->current_rom_bank = value & 0x7F;
	This->current_rom_bank %= This->rom_bank_count;
	if (!This->current_rom_bank)
		This->current_rom_bank = 1;
}

void Mbc3Cartridge::write8_switch_ram_bank(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc3Cartridge *>(sc);
	if (value < 4){
		This->current_ram_bank = value;
		This->current_rtc_register = -1;
	} else if (value >= 8 && value < 12)
		This->current_rtc_register = value - 8;
}

void Mbc3Cartridge::write8_latch_rtc_registers(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc3Cartridge *>(sc);
	if (!This->rtc_latch && value)
		This->set_rtc_registers();
	This->rtc_latch = value;
}

byte_t Mbc3Cartridge::read8_rtc_register(StandardCartridge *sc, main_integer_t address){
	auto This = static_cast<Mbc3Cartridge *>(sc);
	switch (This->current_rtc_register){
		case 0x08:
			return This->rtc_registers.seconds;
		case 0x09:
			return This->rtc_registers.minutes;
		case 0x0A:
			return This->rtc_registers.hours;
		case 0x0B:
			return This->rtc_registers.day_counter_low;
		case 0x0C:
			return This->rtc_registers.day_counter_high;
	}
}

void Mbc3Cartridge::write8_rtc_register(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc3Cartridge *>(sc);
	switch (This->current_rtc_register){
		case 0x08:
			This->rtc_registers.seconds = value;
		case 0x09:
			This->rtc_registers.minutes = value;
		case 0x0A:
			This->rtc_registers.hours = value;
		case 0x0B:
			This->rtc_registers.day_counter_low = value;
		case 0x0C:
			break;
		default:
			return;
	}
	auto delta = This->rtc_registers.day_counter_high ^ value;
	This->rtc_registers.day_counter_high = value;
	if (delta & rtc_stop_mask){
		if (value & rtc_stop_mask)
			This->stop_rtc();
		else
			This->resume_rtc();
	}
}
