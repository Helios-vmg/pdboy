#include "HostSystemServiceProviders.h"
#include <fstream>

std::atomic<bool> slow_mode(false);

StorageProvider::~StorageProvider(){
}

std::unique_ptr<std::vector<byte_t>> StorageProvider::load_file(const char *path, size_t maximum_size){
	std::unique_ptr<std::vector<byte_t>> ret;
	std::ifstream file(path, std::ios::binary);
	if (!file)
		return ret;
	file.seekg(0, std::ios::end);
	if ((size_t)file.tellg() > maximum_size)
		return ret;
	ret.reset(new std::vector<byte_t>(file.tellg()));
	file.seekg(0);
	file.read((char *)&(*ret)[0], ret->size());
	return ret;
}

std::unique_ptr<std::vector<byte_t>> StorageProvider::load_file(const wchar_t *path, size_t maximum_size){
	return nullptr;
}

bool StorageProvider::save_file(const char *path, const std::vector<byte_t> &buffer){
	std::unique_ptr<std::vector<byte_t>> ret;
	std::ofstream file(path, std::ios::binary);
	if (!file)
		return false;
	file.write((const char *)&buffer[0], buffer.size());
	return true;
}

bool StorageProvider::save_file(const wchar_t *path, const std::vector<byte_t> &){
	return false;
}

std::string StorageProvider::get_save_location(){
	return ".";
}

std::wstring StorageProvider::get_save_location_wide(){
	return L".";
}

