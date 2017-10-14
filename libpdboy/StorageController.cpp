#include "StorageController.h"
#include "libpdboy.hpp"
#include <cassert>
#include <ctime>

bool StorageController::load_cartridge(const path_t &path){
	std::vector<byte_t> buffer;
	if (!this->host->load_file(path, buffer, 16 << 20) || !buffer.size())
		return false;
	auto new_cart = Cartridge::construct_from_buffer(*this->host, path, std::move(buffer));
	if (!new_cart)
		return false;
	this->cartridge = std::move(new_cart);
	return true;
}

int StorageController::get_current_rom_bank(){
	if (!this->cartridge)
		return -1;
	return this->cartridge->get_current_rom_bank();
}
