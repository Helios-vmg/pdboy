#include "StorageController.h"
#include "HostSystem.h"
#include <cassert>

bool StorageController::load_cartridge(const char *path){
	auto buffer = this->host->get_storage_provider()->load_file(path, 16 << 20);
	if (!buffer || !buffer->size())
		return false;
	auto new_cart = Cartridge::construct_from_buffer(std::move(buffer));
	if (!new_cart)
		return false;
	this->cartridge = std::move(new_cart);
	return true;
}

Cartridge::~Cartridge(){
}

std::unique_ptr<Cartridge> Cartridge::construct_from_buffer_inner(std::unique_ptr<std::vector<byte_t>> &&buffer){
	CartridgeCapabilities capabilities;
	if (!Cartridge::determine_capabilities(capabilities, *buffer))
		return nullptr;

	if (capabilities.has_special_features){
		switch (capabilities.cartridge_type){
			case 0xFC:
				return std::make_unique<PocketCameraCartridge>(std::move(buffer));
			case 0xFD:
				return std::make_unique<BandaiTama5Cartridge>(std::move(buffer));
			case 0xFE:
				return std::make_unique<Hudson3Cartridge>(std::move(buffer));
			case 0xFF:
				return std::make_unique<Hudson1Cartridge>(std::move(buffer));
		}
		throw GenericException("Internal error: control flow inside the emulator has reached a point that should have never been reached.");
	}

	switch (capabilities.memory_type){
		case CartridgeMemoryType::ROM:
			return std::make_unique<RomOnlyCartridge>(std::move(buffer), capabilities);
		case CartridgeMemoryType::MBC1:
			return std::make_unique<Mbc1Cartridge>(std::move(buffer), capabilities);
		case CartridgeMemoryType::MBC2:
			return std::make_unique<Mbc2Cartridge>(std::move(buffer), capabilities);
		case CartridgeMemoryType::MBC3:
			return std::make_unique<Mbc3Cartridge>(std::move(buffer), capabilities);
		case CartridgeMemoryType::MBC4:
			return std::make_unique<Mbc4Cartridge>(std::move(buffer), capabilities);
		case CartridgeMemoryType::MBC5:
			return std::make_unique<Mbc5Cartridge>(std::move(buffer), capabilities);
		case CartridgeMemoryType::MBC6:
			return std::make_unique<Mbc6Cartridge>(std::move(buffer), capabilities);
		case CartridgeMemoryType::MBC7:
			return std::make_unique<Mbc7Cartridge>(std::move(buffer), capabilities);
		case CartridgeMemoryType::MMM01:
			return std::make_unique<Mmm01Cartridge>(std::move(buffer), capabilities);
	}
	throw GenericException("Internal error: control flow inside the emulator has reached a point that should have never been reached.");
}

std::unique_ptr<Cartridge> Cartridge::construct_from_buffer(std::unique_ptr<std::vector<byte_t>> &&buffer){
	auto ret = Cartridge::construct_from_buffer_inner(std::move(buffer));
	if (ret)
		ret->post_initialization();
	return ret;
}

template <size_t N>
static bool any_match(byte_t n, const byte_t (&array)[N]){
	for (auto c : array)
		if (n == c)
			return true;
	return false;
}

bool Cartridge::determine_capabilities(CartridgeCapabilities &capabilities, const std::vector<byte_t> &buffer){
	memset(&capabilities, 0, sizeof(capabilities));
	if (buffer.size() < 0x150)
		return false;
	auto type = buffer[0x147];

	auto rom_banks_value = buffer[0x148];
	if (rom_banks_value < 8)
		capabilities.rom_bank_count = 1 << (rom_banks_value + 1);
	else if (rom_banks_value == 0x52)
		capabilities.rom_bank_count = 72;
	else if (rom_banks_value == 0x53)
		capabilities.rom_bank_count = 80;
	else if (rom_banks_value == 0x54)
		capabilities.rom_bank_count = 96;
	else
		return false;

	capabilities.cartridge_type = type;

	if (type >= 0xFC){
		capabilities.has_special_features = true;
		return true;
	}

	static const byte_t rom_only_types[] = { 0x00, 0x08, 0x09 };
	static const byte_t mbc1_types[] = { 0x01, 0x02, 0x03 };
	static const byte_t mbc2_types[] = { 0x05, 0x06 };
	static const byte_t mbc3_types[] = { 0x0F, 0x10, 0x11, 0x12, 0x13 };
	static const byte_t mbc4_types[] = { 0x15, 0x16, 0x17 };
	static const byte_t mbc5_types[] = { 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E };
	static const byte_t mmm01_types[] = { 0x0B, 0x0C, 0x0D };
	static const byte_t ram_havers[] = { 0x02, 0x03, 0x08, 0x09, 0x0C, 0x0D, 0x10, 0x12, 0x13, 0x16, 0x17, 0x19, 0x1A, 0x1B, 0x1D, 0x1E, 0x22 };
	static const byte_t battery_havers[] = { 0x03, 0x06, 0x09, 0x0D, 0x0F, 0x10, 0x13, 0x17, 0x1B, 0x1E, 0x22 };
	static const byte_t timer_havers[] = { 0x0F, 0x10 };
	static const byte_t rumble_havers[] = { 0x0C, 0x0D, 0x0E, 0x22 };
	static const byte_t sensor_havers[] = { 0x022 };

	//Set memory type.

	if (any_match(type, rom_only_types))
		capabilities.memory_type = CartridgeMemoryType::ROM;
	else if (any_match(type, mbc1_types))
		capabilities.memory_type = CartridgeMemoryType::MBC1;
	else if (any_match(type, mbc2_types))
		capabilities.memory_type = CartridgeMemoryType::MBC2;
	else if (any_match(type, mbc3_types))
		capabilities.memory_type = CartridgeMemoryType::MBC3;
	else if (any_match(type, mbc4_types))
		capabilities.memory_type = CartridgeMemoryType::MBC4;
	else if (any_match(type, mbc5_types))
		capabilities.memory_type = CartridgeMemoryType::MBC5;
	else if (type == 0x20){
		capabilities = { type, CartridgeMemoryType::MBC6, false, false, false, false, false, false };
		return true;
	}else if (type == 0x22){
		capabilities.memory_type = CartridgeMemoryType::MBC7;
		capabilities.has_sensor = true;
		capabilities.has_ram = true;
		capabilities.has_rumble = true;
		capabilities.has_battery = true;
		capabilities.has_timer = false;
		capabilities.has_special_features = false;
		return true;
	}else if (any_match(type, mmm01_types))
		capabilities.memory_type = CartridgeMemoryType::MMM01;
	else
		return false;

	//Set haves:
	capabilities.has_ram = any_match(type, ram_havers);
	capabilities.has_battery = any_match(type, battery_havers);
	capabilities.has_timer = any_match(type, timer_havers);
	capabilities.has_rumble = any_match(type, rumble_havers);
	assert(!any_match(type, sensor_havers));
	//capabilities.has_sensor = any_match(type, sensor_havers);
	capabilities.has_sensor = false;
	capabilities.has_special_features = false;

	if (capabilities.has_ram){
		auto size = buffer[0x149];
		static const unsigned ram_sizes[] = {
			0,
			1 << 11,
			1 << 13,
			1 << 15,
			1 << 17,
			1 << 16,
		};
		capabilities.ram_size = ram_sizes[size % 6];
	}
	return true;
}

StandardCartridge::StandardCartridge(std::unique_ptr<std::vector<byte_t>> &&buffer, const CartridgeCapabilities &capabilities){
	this->capabilities = capabilities;
	this->buffer = std::move(*buffer);
	buffer.reset();
	this->size = this->buffer.size();
	assert(this->size);
	this->data = &this->buffer[0];
	if (this->capabilities.has_ram)
		this->ram.resize(this->capabilities.ram_size);
	this->write_callbacks_unique.reset(new write8_f[0x100]);
	this->read_callbacks_unique.reset(new read8_f[0x100]);
	this->write_callbacks = this->write_callbacks_unique.get();
	this->read_callbacks = this->read_callbacks_unique.get();
	this->rom_bank_count = capabilities.rom_bank_count;
}

StandardCartridge::~StandardCartridge(){
}

void StandardCartridge::post_initialization(){
	this->init_functions();
}

void StandardCartridge::init_functions(){
	std::fill(this->write_callbacks, this->write_callbacks + 0x100, write8_do_nothing);
	std::fill(this->read_callbacks, this->read_callbacks + 0x100, read8_do_nothing);
	this->init_functions_derived();
}

void StandardCartridge::write8(main_integer_t address, byte_t value){
	this->write_callbacks[address >> 8](this, address, value);
}

byte_t StandardCartridge::read8(main_integer_t address){
	return this->read_callbacks[address >> 8](this, address);
}

void StandardCartridge::commit_ram(){
	throw NotImplementedException();
}

RomOnlyCartridge::RomOnlyCartridge(std::unique_ptr<std::vector<byte_t>> &&buffer, const CartridgeCapabilities &cc): StandardCartridge(std::move(buffer), cc){
}

byte_t RomOnlyCartridge::read8(main_integer_t address){
	return this->data[address & 0x7FFF];
}

Mbc1Cartridge::Mbc1Cartridge(std::unique_ptr<std::vector<byte_t>> &&buffer, const CartridgeCapabilities &cc): StandardCartridge(std::move(buffer), cc){
}

void Mbc1Cartridge::init_functions_derived(){
	for (unsigned i = 0; i < 0x40; i++)
		this->read_callbacks[i] = read8_simple;
	for (unsigned i = 0x40; i < 0x80; i++)
		this->read_callbacks[i] = read8_switchable_rom_bank;

	if (this->capabilities.has_ram){
		for (unsigned i = 0; i < 0x20; i++)
			this->write_callbacks[i] = write8_ram_enable;
	}

	for (unsigned i = 0x20; i < 0x40; i++)
		this->write_callbacks[i] = write8_switch_rom_bank_low;
	for (unsigned i = 0x40; i < 0x60; i++)
		this->write_callbacks[i] = write8_switch_rom_bank_high_or_ram;
	for (unsigned i = 0x60; i < 0x80; i++)
		this->write_callbacks[i] = write8_switch_rom_ram_banking_mode;

	this->set_ram_functions();
}

void Mbc1Cartridge::set_ram_functions(){
	if (!this->capabilities.has_ram)
		return;

	if (!this->ram_enabled){
		for (unsigned i = 0xA0; i < 0xC0; i++){
			this->write_callbacks[i] = write8_invalid_ram;
			this->read_callbacks[i] = read8_invalid_ram;
		}
		return;
	}

	if (this->capabilities.ram_size == 1 << 11){
		for (unsigned i = 0xA0; i < 0xA8; i++){
			this->write_callbacks[i] = write8_small_ram;
			this->read_callbacks[i] = read8_small_ram;
		}
		for (unsigned i = 0xA8; i < 0xC0; i++){
			this->write_callbacks[i] = write8_invalid_ram;
			this->read_callbacks[i] = read8_invalid_ram;
		}
	}else{
		for (unsigned i = 0xA0; i < 0xC0; i++){
			this->write_callbacks[i] = write8_switchable_ram_bank;
			this->read_callbacks[i] = read8_switchable_ram_bank;
		}
	}
}

byte_t Mbc1Cartridge::read8_simple(StandardCartridge *sc, main_integer_t address){
	auto This = static_cast<Mbc1Cartridge *>(sc);
	return This->data[address];
}

byte_t Mbc1Cartridge::read8_switchable_rom_bank(StandardCartridge *sc, main_integer_t address){
	auto This = static_cast<Mbc1Cartridge *>(sc);
	auto offset = This->compute_rom_offset(address);
	return This->data[offset];
}

byte_t Mbc1Cartridge::read8_small_ram(StandardCartridge *sc, main_integer_t address){
	auto This = static_cast<Mbc1Cartridge *>(sc);
	return This->ram[address & 0x7FFF];
}

byte_t Mbc1Cartridge::read8_switchable_ram_bank(StandardCartridge *sc, main_integer_t address){
	auto This = static_cast<Mbc1Cartridge *>(sc);
	auto offset = This->compute_ram_offset(address);
	return This->ram[offset];
}

void Mbc1Cartridge::write8_ram_enable(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc1Cartridge *>(sc);
	This->toggle_ram((value & 0x0F) == 0x0A);
}

void Mbc1Cartridge::write8_switch_rom_bank_low(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc1Cartridge *>(sc);
	const decltype(This->current_rom_bank) mask = 0x1F;
	value &= mask;
	This->current_rom_bank &= ~mask;
	This->current_rom_bank |= value & mask;
	This->current_rom_bank %= This->rom_bank_count;
	if (!(This->current_rom_bank & mask))
		This->current_rom_bank++;
}

void Mbc1Cartridge::write8_switch_rom_bank_high_or_ram(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc1Cartridge *>(sc);
	const decltype(This->current_rom_bank) mask = 3;
	value &= mask;
	This->ram_bank_bits_copy = value;
	This->toggle_ram_banking(This->ram_banking_mode);
}

void Mbc1Cartridge::write8_switch_rom_ram_banking_mode(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc1Cartridge *>(sc);
	This->toggle_ram_banking(!!value);
}

void Mbc1Cartridge::write8_switchable_ram_bank(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc1Cartridge *>(sc);
	auto offset = This->compute_ram_offset(address);
	This->ram[offset] = value;
	This->ram_written = true;
}

byte_t Mbc1Cartridge::read8_invalid_ram(StandardCartridge *, main_integer_t){
	throw GenericException("Attempt to read from invalid cartridge RAM.");
}

void Mbc1Cartridge::write8_small_ram(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc1Cartridge *>(sc);
	//address -= 0xA000; (Unnecessary due to bitwise AND.)
	auto offset = address & 0x7FF;
	This->ram[offset] = value;
	This->ram_written = true;
}

void Mbc1Cartridge::write8_invalid_ram(StandardCartridge *, main_integer_t, byte_t){
	throw GenericException("Attempt to write to invalid cartridge RAM.");
}

main_integer_t Mbc1Cartridge::compute_rom_offset(main_integer_t address){
	return (address & 0x3FFF) | (this->current_rom_bank << 14);
}

main_integer_t Mbc1Cartridge::compute_ram_offset(main_integer_t address){
	address -= 0xA000;
	return (address & 0x3FFF) | (this->current_ram_bank << 14);
}

void Mbc1Cartridge::toggle_ram(bool enable){
	if (!(this->ram_enabled ^ enable))
		return;
	if (this->ram_enabled && this->ram_written)
		this->commit_ram();
	this->ram_enabled = enable;
	this->set_ram_functions();
}

void Mbc1Cartridge::toggle_ram_banking(bool enable){
	const decltype(this->current_rom_bank) rom_mask = 0xE0;
	this->ram_banking_mode = enable;
	if (this->ram_banking_mode)
		this->current_ram_bank = this->ram_bank_bits_copy;
	else{
		this->current_rom_bank &= ~rom_mask;
		this->current_rom_bank |= this->ram_bank_bits_copy << 5;
	}
}

Mbc2Cartridge::Mbc2Cartridge(std::unique_ptr<std::vector<byte_t>> &&buffer, const CartridgeCapabilities &cc):
		StandardCartridge(std::move(buffer), cc){
	throw NotImplementedException();
}

Mbc3Cartridge::Mbc3Cartridge(std::unique_ptr<std::vector<byte_t>> &&buffer, const CartridgeCapabilities &cc):
		Mbc1Cartridge(std::move(buffer), cc){
}

void Mbc3Cartridge::init_functions_derived(){
	Mbc1Cartridge::init_functions_derived();
	for (unsigned i = 0x20; i < 0x40; i++)
		this->write_callbacks[i] = write8_switch_rom_bank;
	for (unsigned i = 0x40; i < 0x60; i++)
		this->write_callbacks[i] = write8_switch_ram_bank;
	for (unsigned i = 0x60; i < 0x80; i++)
		this->write_callbacks[i] = write8_switch_ram_bank;
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

void Mbc3Cartridge::set_rtc_registers(){
	throw NotImplementedException();
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
	}else if (value >= 8 && value < 12)
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
	throw NotImplementedException();
}

void Mbc3Cartridge::write8_rtc_register(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc3Cartridge *>(sc);
	throw NotImplementedException();
}

Mbc5Cartridge::Mbc5Cartridge(std::unique_ptr<std::vector<byte_t>> &&buffer, const CartridgeCapabilities &cc) :
		StandardCartridge(std::move(buffer), cc){
	throw NotImplementedException();
}
