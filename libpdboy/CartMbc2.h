#pragma once
#include "StorageController.h"

class Mbc2Cartridge : public StandardCartridge{
protected:
public:
	Mbc2Cartridge(libpdboy &host, std::vector<byte_t> &&, const CartridgeCapabilities &);
};
