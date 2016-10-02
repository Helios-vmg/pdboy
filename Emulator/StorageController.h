#pragma once
#include "CommonTypes.h"
#include "exceptions.h"
#include <memory>
#include <vector>

class Gameboy;
class HostSystem;

enum class CartridgeMemoryType{
	ROM,
	MBC1,
	MBC2,
	MBC3,
	MBC4,
	MBC5,
	MBC6,
	MBC7,
	MMM01,
	Other,
};

struct CartridgeCapabilities{
	byte_t cartridge_type;
	CartridgeMemoryType memory_type;
	bool
		has_ram,
		has_battery,
		has_timer,
		has_rumble,
		has_sensor,
		has_special_features;
	unsigned ram_size;
};

class Cartridge{
	static bool determine_capabilities(CartridgeCapabilities &, const std::vector<byte_t> &);
	static std::unique_ptr<Cartridge> construct_from_buffer_inner(std::unique_ptr<std::vector<byte_t>> &&);
public:
	virtual ~Cartridge() = 0;
	static std::unique_ptr<Cartridge> construct_from_buffer(std::unique_ptr<std::vector<byte_t>> &&);
	virtual void write8(main_integer_t address, byte_t value) = 0;
	virtual byte_t read8(main_integer_t address) = 0;
	virtual void post_initialization(){}
};

#define DECLARE_UNSUPPORTED_CARTRIDGE_CLASS(x, base) \
	class x : public base{ \
	public: \
		x(std::unique_ptr<std::vector<byte_t>> &&){ \
			throw NotImplementedException(); \
		} \
		void write8(main_integer_t, byte_t) override{} \
		byte_t read8(main_integer_t address) override{ \
			return 0; \
		} \
	}

#define DECLARE_UNSUPPORTED_STANDARD_CARTRIDGE_CLASS(x) \
	class x : public StandardCartridge{ \
	public: \
		x(std::unique_ptr<std::vector<byte_t>> &&v, const CartridgeCapabilities &cc): StandardCartridge(std::move(v), cc){ \
			throw NotImplementedException(); \
		} \
		void write8(main_integer_t, byte_t) override{} \
		byte_t read8(main_integer_t) override{ \
			return 0; \
		} \
	}

DECLARE_UNSUPPORTED_CARTRIDGE_CLASS(PocketCameraCartridge, Cartridge);
DECLARE_UNSUPPORTED_CARTRIDGE_CLASS(BandaiTama5Cartridge, Cartridge);
DECLARE_UNSUPPORTED_CARTRIDGE_CLASS(Hudson3Cartridge, Cartridge);
DECLARE_UNSUPPORTED_CARTRIDGE_CLASS(Hudson1Cartridge, Cartridge);

class StandardCartridge : public Cartridge{
protected:
	typedef void(*write8_f)(StandardCartridge *, main_integer_t, byte_t);
	typedef byte_t(*read8_f)(StandardCartridge *, main_integer_t);
private:
	std::unique_ptr<write8_f[]> write_callbacks_unique;
	std::unique_ptr<read8_f[]> read_callbacks_unique;

	void init_functions();
	
protected:
	CartridgeCapabilities capabilities;
	std::vector<byte_t> buffer;
	size_t size;
	byte_t *data;
	unsigned current_rom_bank = 0;
	unsigned current_ram_bank = 0;
	unsigned ram_bank_bits_copy = 0;
	write8_f *write_callbacks;
	read8_f *read_callbacks;
	std::vector<byte_t> ram;
	bool ram_banking_mode = false;
	bool ram_written = false;
	bool ram_enabled = false;

	static void write8_do_nothing(StandardCartridge *, main_integer_t, byte_t){}
	static byte_t read8_do_nothing(StandardCartridge *, main_integer_t){ return 0; }
	virtual void init_functions_derived(){}
public:
	StandardCartridge(std::unique_ptr<std::vector<byte_t>> &&, const CartridgeCapabilities &);
	virtual ~StandardCartridge() = 0;
	void post_initialization() override;
	virtual void write8(main_integer_t, byte_t) override;
	virtual byte_t read8(main_integer_t) override;
	void commit_ram();
};

//Overrides usage of function arrays.
class RomOnlyCartridge : public StandardCartridge{
protected:
public:
	RomOnlyCartridge(std::unique_ptr<std::vector<byte_t>> &&, const CartridgeCapabilities &);
	void write8(main_integer_t, byte_t) override{}
	byte_t read8(main_integer_t) override;
};

class Mbc1Cartridge : public StandardCartridge{
protected:

	void init_functions_derived() override;
	void set_ram_functions();
	static byte_t read8_simple(StandardCartridge *, main_integer_t);
	static byte_t read8_switchable_rom_bank(StandardCartridge *, main_integer_t);
	static byte_t read8_switchable_ram_bank(StandardCartridge *, main_integer_t);
	static byte_t read8_small_ram(StandardCartridge *, main_integer_t);
	static byte_t read8_invalid_ram(StandardCartridge *, main_integer_t);
	static void write8_ram_enable(StandardCartridge *, main_integer_t, byte_t);
	static void write8_switch_rom_bank_low(StandardCartridge *, main_integer_t, byte_t);
	static void write8_switch_rom_bank_high_or_ram(StandardCartridge *, main_integer_t, byte_t);
	static void write8_switch_rom_ram_banking_mode(StandardCartridge *, main_integer_t, byte_t);
	static void write8_switchable_ram_bank(StandardCartridge *, main_integer_t, byte_t);
	static void write8_small_ram(StandardCartridge *, main_integer_t, byte_t);
	static void write8_invalid_ram(StandardCartridge *, main_integer_t, byte_t);

	void toggle_ram(bool);
	void toggle_ram_banking(bool);
	main_integer_t compute_rom_offset(main_integer_t address);
	main_integer_t compute_ram_offset(main_integer_t address);
public:
	Mbc1Cartridge(std::unique_ptr<std::vector<byte_t>> &&, const CartridgeCapabilities &);
};

class Mbc2Cartridge : public StandardCartridge{
protected:
public:
	Mbc2Cartridge(std::unique_ptr<std::vector<byte_t>> &&, const CartridgeCapabilities &);
};

class Mbc3Cartridge : public StandardCartridge{
protected:
public:
	Mbc3Cartridge(std::unique_ptr<std::vector<byte_t>> &&, const CartridgeCapabilities &);
};

DECLARE_UNSUPPORTED_STANDARD_CARTRIDGE_CLASS(Mbc4Cartridge);

class Mbc5Cartridge : public StandardCartridge{
protected:
public:
	Mbc5Cartridge(std::unique_ptr<std::vector<byte_t>> &&, const CartridgeCapabilities &);
};

DECLARE_UNSUPPORTED_STANDARD_CARTRIDGE_CLASS(Mbc6Cartridge);
DECLARE_UNSUPPORTED_STANDARD_CARTRIDGE_CLASS(Mbc7Cartridge);
DECLARE_UNSUPPORTED_STANDARD_CARTRIDGE_CLASS(Mmm01Cartridge);

class StorageController{
	Gameboy *system;
	HostSystem *host;
	std::unique_ptr<Cartridge> cartridge;
public:
	StorageController(Gameboy &system, HostSystem &host): system(&system), host(&host){}
	bool load_cartridge(const char *path);
	void write8(main_integer_t address, byte_t value){
		this->cartridge->write8(address, value);
	}
	byte_t read8(main_integer_t address){
		return this->cartridge->read8(address);
	}
};
