#include "HostSystemServiceProviders.h"
#include "HostSystem.h"
#include <fstream>
#include <iostream>

StorageProvider::~StorageProvider(){
}

std::unique_ptr<std::vector<byte_t>> StorageProvider::load_file(const path_t &path, size_t maximum_size){
	std::unique_ptr<std::vector<byte_t>> ret;
	auto casted_path = std::dynamic_pointer_cast<StdBasicString<char>>(path);
	if (!casted_path)
		return ret;

	auto string = casted_path->get_std_basic_string();
	std::cout << "Requested file load: \"" << string << "\", maximum size: " << maximum_size << " bytes.\n";

	std::ifstream file(string.c_str(), std::ios::binary);
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

bool StorageProvider::save_file(const path_t &path, const std::vector<byte_t> &buffer){
	auto casted_path = std::dynamic_pointer_cast<StdBasicString<char>>(path);
	if (!casted_path)
		return false;

	auto string = casted_path->get_std_basic_string();
	std::cout << "Requested file save: \"" << string << "\", size: " << buffer.size() << " bytes.\n";

	std::ofstream file(string.c_str(), std::ios::binary);
	if (!file)
		return false;
	file.write((const char *)&buffer[0], buffer.size());
	return true;
}

path_t StorageProvider::get_save_location(){
	return path_t(new StdBasicString<char>("."));
}

void EventProvider::toggle_fastforward(bool fastforward){
	this->host->toggle_fastforward(fastforward);
}

void EventProvider::toggle_pause(int pause){
	this->host->toggle_pause(pause);
}
