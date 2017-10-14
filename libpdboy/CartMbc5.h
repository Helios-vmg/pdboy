#pragma once
#include "StorageController.h"

class Mbc5Cartridge : public StandardCartridge{
protected:
public:
	Mbc5Cartridge(libpdboy &host, std::vector<byte_t> &&, const CartridgeCapabilities &);
};
